#include "NasdaqReplayer.hpp"

#include "Snapshot.hpp"
#include "TimeParser.cpp"
#include "../Parser/MessageType.hpp"
#include "../Parser/NasdaqStructures.hpp"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <bit>
#include <chrono>
#include <iostream>
#include <cstring>
#include <string>

namespace Replayer {

	NasdaqReplayer::NasdaqReplayer(std::string file_name) : file_name(std::move(file_name)) {
		csv_file.open("ofi.csv");
		csv_file << "timestamp,stock_locate,mid_price,mid_price_delta,ofi\n";
	}


	void NasdaqReplayer::Run() {

		const int fd = open(file_name.c_str(), O_RDONLY);

		struct stat file_stat{};
		fstat(fd, &file_stat);

		const size_t file_size = file_stat.st_size;

		auto* byte_stream = static_cast<uint8_t*>(mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, fd, 0));
		size_t offset = 0;


		size_t processed_messages = 0;
		const auto start_time = std::chrono::high_resolution_clock::now();

		while (offset < file_size) {

			processed_messages++;

			uint16_t message_length;
			std::memcpy(&message_length, byte_stream + offset, sizeof(uint16_t));
			message_length = std::byteswap(message_length);
			offset += sizeof(uint16_t);

			uint8_t message_type_bin = (*(byte_stream+offset));

			bool book_modified = false;
			uint16_t stock_locate = 0;
			uint64_t parsed_timestamp = 0;

			switch (static_cast<Parser::MessageType>(message_type_bin)) {

				case Parser::MessageType::StockDirectory: {

					const auto* payload = reinterpret_cast<const Parser::Structure::StockDirectory*>(byte_stream + offset);

					const auto stock_name = std::string_view(
						payload->stock, 8);

					if (stock_name == "MSFT    ") {
						// not wasting time to byte swap: consistently using the Big Endian
						// destructing
						stock_locate = payload->stock_locate;
					}

					if (stock_name == "AAPL    ") {
						stock_locate = payload->stock_locate;
					}

					if (stock_locate) {
						order_books[stock_locate] = std::make_unique<Engine::OrderBook>(
							Parser::Structure::MAX_ORDERS, Parser::Structure::MAX_ORDER_ID, Parser::Structure::MAX_PRICE);

						stock_states [stock_locate] = std::make_unique<TickerMicrostate>();

						stock_dict[stock_locate] = stock_name;
					}

					break;
				}

				case Parser::MessageType::AddOrderA: {

					const auto* payload = reinterpret_cast<const Parser::Structure::AddOrderA*>(byte_stream + offset);

					stock_locate = payload->stock_locate_code;
					parsed_timestamp = Parser::parseTimestamp(payload->timestamp);

					if (const auto order_book = order_books[stock_locate].get()) {

						const auto order_id = std::byteswap(payload->order_reference_number);
						const auto price = std::byteswap(payload->price);
						const auto volume = std::byteswap(payload->volume);
						const auto side = static_cast<Engine::Side>(payload->buy_or_sell);

						order_book->addOrder(order_id, price, volume, side);

						book_modified = true;
					}

					break;

				}

				case Parser::MessageType::AddOrderF: {

					const auto* payload = reinterpret_cast<const Parser::Structure::AddOrderF*>(byte_stream + offset);

					stock_locate = payload->stock_locate_code;
					parsed_timestamp = Parser::parseTimestamp(payload->timestamp);

					if (const auto order_book = order_books[stock_locate].get()) {

						const auto order_id = std::byteswap(payload->order_reference_number);
						const auto price = std::byteswap(payload->price);
						const auto volume = std::byteswap(payload->volume);
						const auto side = static_cast<Engine::Side>(payload->buy_or_sell);

						order_book->addOrder(order_id, price, volume, side);

						book_modified = true;

					}

					break;

				}

				case Parser::MessageType::OrderExecuted: {

					const auto* payload = reinterpret_cast<const Parser::Structure::OrderExecuted*>(byte_stream + offset);

					stock_locate = payload->stock_locate_code;
					parsed_timestamp = Parser::parseTimestamp(payload->timestamp);

					if (const auto order_book = order_books[stock_locate].get()) {

						order_book->reduceOrderVolume(
							std::byteswap(payload->order_reference_number),
							std::byteswap(payload->executed_shares)
						);

						book_modified = true;

					}
					break;
				}

				case Parser::MessageType::OrderExecutedWithPrice: {

					const auto* payload = reinterpret_cast<const Parser::Structure::OrderExecutedWithPrice*>(byte_stream + offset);

					stock_locate = payload->stock_locate_code;
					parsed_timestamp = Parser::parseTimestamp(payload->timestamp);

					if (const auto order_book = order_books[stock_locate].get()) {

						order_book->reduceOrderVolume(
							std::byteswap(payload->order_reference_number),
							std::byteswap(payload->executed_shares)
						);

						book_modified = true;
					}
					break;
				}

				case Parser::MessageType::OrderCancel: {

					const auto* payload = reinterpret_cast<const Parser::Structure::OrderCancel*>(byte_stream + offset);

					stock_locate = payload->stock_locate_code;
					parsed_timestamp = Parser::parseTimestamp(payload->timestamp);

					if (const auto order_book = order_books[stock_locate].get()) {

						order_book->reduceOrderVolume(
							std::byteswap(payload->order_reference_number),
							std::byteswap(payload->cancelled_shares)
						);

						book_modified = true;

					}
					break;

				}

				case Parser::MessageType::OrderDelete: {

					const auto* payload = reinterpret_cast<const Parser::Structure::OrderDelete*>(byte_stream + offset);

					stock_locate = payload->stock_locate_code;
					parsed_timestamp = Parser::parseTimestamp(payload->timestamp);

					if (const auto order_book = order_books[stock_locate].get()) {

						order_book->removeOrder(std::byteswap(payload->order_reference_number));

						book_modified = true;

					}
					break;
				}

				case Parser::MessageType::OrderReplace: {

					const auto* payload = reinterpret_cast<const Parser::Structure::OrderReplace*>(byte_stream + offset);

					stock_locate = payload->stock_locate_code;
					parsed_timestamp = Parser::parseTimestamp(payload->timestamp);

					if (const auto order_book = order_books[stock_locate].get()) {

						const auto old_id = std::byteswap(payload->original_order_reference_number);
						const auto new_id = std::byteswap(payload->new_order_reference_number);
						const auto new_price = std::byteswap(payload->price);
						const auto new_volume = std::byteswap(payload->shares);

						if (const auto old_order = order_book->getOrder(old_id)) {
							const Engine::Side side = old_order->side;
							order_book->replaceOrder(old_id, new_id, new_price, new_volume, side);
						}

						book_modified = true;

					}
					break;
				}

				default: break;
			}

			if (book_modified) {

				const auto order_book = order_books[stock_locate].get();

				const auto stock_state = stock_states[stock_locate].get();
				stock_state->current_ofi_accumulator += calculateTickOFI(*order_book, *stock_state);

				stock_state->event_count++;

				if (stock_state->event_count >= 1000) {

					const double current_mid = (order_book->getBestAskPrice() + order_book->getBestBidPrice()) / 2.0;
					Snapshot snap{};
					snap.parsed_timestamp = parsed_timestamp;
					snap.mid_price = current_mid;
					snap.mid_price_delta = current_mid - stock_state->prev_snapshot_mid_price;
					snap.cumulated_ofi = stock_state->current_ofi_accumulator;
					snap.stock_locate = stock_locate;


					stock_state->current_ofi_accumulator = 0;
					stock_state->event_count = 0;
					stock_state->prev_snapshot_mid_price = current_mid;
					stock_state->stock_locate = stock_locate;

					writeSnapshot(snap);

				}

			}

			offset+=message_length;

		}

		const auto end_time = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> diff = end_time - start_time;

		close(fd);
		munmap(byte_stream, file_size);

		writeStockDictionary();

		std::cout << "Messages processed: " << processed_messages << ".\n";
		std::cout << "Time: " << diff.count() << " seconds.\n";
		std::cout << "Throughput: " << (processed_messages / diff.count()) / 1e6 << " mln msg/s.\n";

	}

	int64_t NasdaqReplayer::calculateTickOFI(const Engine::OrderBook& order_book, TickerMicrostate& state) {

		const uint32_t cur_bid = order_book.getBestBidPrice();
		const uint32_t cur_ask = order_book.getBestAskPrice();

		const uint32_t cur_bid_vol = (cur_bid == 0) ? 0 : order_book.getPriceLevel(cur_bid).total_volume;
		const uint32_t cur_ask_vol = (cur_ask == UINT32_MAX) ? 0 : order_book.getPriceLevel(cur_ask).total_volume;

		int64_t buy_pressure = 0;
		if (cur_bid > state.prev_bid) buy_pressure = cur_bid_vol;
		else if (cur_bid == state.prev_bid) buy_pressure = static_cast<int64_t>(cur_bid_vol) - state.prev_bid_vol;
		else buy_pressure = -static_cast<int64_t>(state.prev_bid_vol);

		int64_t sell_pressure = 0;
		if (cur_ask < state.prev_ask) sell_pressure = cur_ask_vol;
		else if (cur_ask == state.prev_ask) sell_pressure = static_cast<int64_t>(cur_ask_vol) - state.prev_ask_vol;
		else sell_pressure = -static_cast<int64_t>(state.prev_ask_vol);

		state.prev_bid = cur_bid;
		state.prev_ask = cur_ask;
		state.prev_bid_vol = cur_bid_vol;
		state.prev_ask_vol = cur_ask_vol;

		return buy_pressure - sell_pressure;
	}

	inline void NasdaqReplayer::writeSnapshot(const Snapshot &snapshot) {
		if (csv_file.is_open()) {
			csv_file << snapshot.parsed_timestamp << ","
					 << snapshot.stock_locate << ","
					 << snapshot.mid_price << ","
					 << snapshot.mid_price_delta << ","
					 << snapshot.cumulated_ofi << "\n";
		}
	}

	void NasdaqReplayer::writeStockDictionary() {

		std::ofstream stock_dict_csv;
		stock_dict_csv.open("stock_dict.csv");
		stock_dict_csv << "stock_locate,stock_symbol\n";

		for (const auto&[stock_locate, stock_symbol] : stock_dict) {
			stock_dict_csv << stock_locate << "," << stock_symbol << "\n";
		}

		stock_dict_csv.close();
	}
}
