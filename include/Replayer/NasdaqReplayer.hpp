#pragma once
#include <../Engine/OrderBook.hpp>

#include <string>
#include <array>
#include <fstream>
#include <memory>
#include <unordered_map>

#include "Snapshot.hpp"

namespace Replayer {
	class NasdaqReplayer {

	private:
		const std::string file_name;

		std::array<std::unique_ptr<Engine::OrderBook>, 65536> order_books {nullptr};
		std::array<std::unique_ptr<TickerMicrostate>, 65536> stock_states {nullptr};

		std::ofstream csv_file;

		static int64_t calculateTickOFI(const Engine::OrderBook& order_book, TickerMicrostate& state);

		inline void writeSnapshot(const Snapshot &snapshot);

		// convenient access to write csv stock dictionary, used 2 times per Run()
		std::unordered_map<uint16_t, std::string> stock_dict;
		inline void writeStockDictionary();

	public:

		explicit NasdaqReplayer(std::string file_name);
		
		void Run();

	};
}
