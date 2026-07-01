#pragma once
#pragma pack(push, 1)

#include <cstdint>
// https://www.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/NQTVITCHSpecification.pdf
namespace Parser::Structure {

	constexpr size_t MAX_ORDERS = 1'000'000;
	constexpr size_t MAX_ORDER_ID = 2'000'000'000;
	constexpr size_t MAX_PRICE = 10'000'000;

	struct SystemEvent {
	char message_type; // 'S'
	uint16_t stock_locate;	//always 0
	uint16_t tracking_number;
	uint8_t timestamp[6];
	char event_code; // some code it has its explanation somewhere
	};

	struct StockDirectory { // assign unique ids for symbols for the day
		char message_type; //'R'
		uint16_t stock_locate;
		uint16_t tracking_number;
		uint8_t timestamp[6];
		char stock[8];
		char market_category;	//Q Nasdaq Global Select MarketSM							//G Nasdaq Global MarketSM			//S Nasdaq Capital Market®
		char financial_status_indicator;
		uint32_t round_lot_size;
		char round_lots_only;
		char issue_classification;
		char issue_sub_type[2];
		char authenticity;
		char short_sale_threshold_indicator;
		char ipo_flag;
		char luldreference_price_tier;
		char etp_flag;
		uint32_t etp_leverage_factor;
		char inverse_indicator;
	};

	struct StockTradingAction {
		char message_type; //h lowercase 'h'
		uint16_t stock_locate;
		uint16_t tracking_number;
		uint8_t timestamp[6];
		char stock[8];
		char trading_state;
		char reserved;
		char reason[4];
	};

	struct RegSHOShortSalePriceTestRestrictedIndicator {
		char message_typ; //y
		uint16_t locate_code;
		uint16_t tracking_number;
		uint8_t timestamp[6];
		char stock[8];
		char reg_sho_action;
	};

	struct MarketParticipantPosition {
		char message_type; // L
		uint16_t stock_locate;
		uint16_t tracking_number;
		uint8_t timestamp[6];
		char mpid[4];
		char stock[8];
		char primary_market_maker;
		char marker_maker_mode;
		char market_participant_state;
	};

	struct MarketWideCircuitBreaker {
		char message_type; // V
		uint16_t stock_locate;
		uint16_t tracking_number;
		uint8_t timestamp[6];
		uint64_t level1;
		uint64_t level2;
		uint64_t level3;
	};

	struct MarketWideCircuitBreakerStatus {
		char message_type; // W
		uint16_t stock_locate;
		uint16_t tracking_number;
		uint8_t timestamp[6];
		char breached_level;
	};

	struct QuotingPeriodUpdate {
		char message_type; // K
		uint16_t stock_locate;
		uint16_t tracking_number;
		uint8_t timestamp[6];
		char stock[8];
		uint32_t ipo_quotation_release_time;
		char ipo_quotation_release_qualifier;
		uint32_t ipo_price;
	};

	struct LULDAuctionCollar {
		char message_type; //J
		uint16_t stock_locate;
		uint16_t tracking_number;
		uint8_t timestamp[6];
		char stock[8];
		uint32_t auction_collar_reference_price;
		uint32_t upper_auction_collar_price;
		uint32_t lower_auction_collar_price;
		uint32_t auction_collar_extension;
	};

	struct OperationalHalt {
		char message_type; //H
		uint16_t stock_locate;
		uint16_t tracking_number;
		uint8_t timestamp[6];
		char stock[8];
		char market_code;
		char operational_halt_action;
	};

	struct AddOrderA {
		char message_type; // 1 'A'
		uint16_t stock_locate_code; // 2 bajty
		uint16_t tracking_number; // 2 bajty
		uint8_t timestamp[6]; //6 bajtów
		uint64_t order_reference_number;
		char buy_or_sell; // 'B' or 'S'
		uint32_t volume;
		char stock_symbol[8];
		uint32_t price;
	};

	struct AddOrderF {
		char message_type; // 1 'F'
		uint16_t stock_locate_code; // 2 bajty
		uint16_t tracking_number; // 2 bajty
		uint8_t timestamp[6]; //6 bajtów
		uint64_t order_reference_number;
		char buy_or_sell; // 'B' or 'S'
		uint32_t volume;
		char stock_symbol[8];
		uint32_t price;
		char attribution[4];
	};

	struct OrderExecuted {
		char message_type; //E
		uint16_t stock_locate_code;
		uint16_t tracking_number;
		uint8_t timestamp[6];
		uint64_t order_reference_number;
		uint32_t executed_shares;
		uint64_t match_number;
	};

	struct OrderExecutedWithPrice {
		char message_type; //C
		uint16_t stock_locate_code;
		uint16_t tracking_number;
		uint8_t timestamp[6];
		uint64_t order_reference_number;
		uint32_t executed_shares;
		uint64_t match_number;
		char printable;
		uint32_t execution_price;
	};

	struct OrderCancel {
		char message_type; // X
		uint16_t stock_locate_code;
		uint16_t tracking_number;
		uint8_t timestamp[6];
		uint64_t order_reference_number;
		uint32_t cancelled_shares;
	};

	struct OrderDelete {
		char message_type; // D
		uint16_t stock_locate_code;
		uint16_t tracking_number;
		uint8_t timestamp[6];
		uint64_t order_reference_number;
	};

	struct OrderReplace {
		char message_type; // U
		uint16_t stock_locate_code;
		uint16_t tracking_number;
		uint8_t timestamp[6];
		uint64_t original_order_reference_number;
		uint64_t new_order_reference_number;
		uint32_t shares;
		uint32_t price;
	};

	struct Trade {
		char message_type; // P
		uint16_t stock_locate_code;
		uint16_t tracking_number;
		uint8_t timestamp[6];
		uint64_t order_reference_number;
		char buy_or_sell;
		uint32_t shares;
		char stock[8];
		uint32_t price;
		uint64_t match_number;
	};

	struct CrossTrade {
		char message_type; // Q
		uint16_t stock_locate_code;
		uint16_t tracking_number;
		uint8_t timestamp[6];
		uint64_t shares;
		char stock[8];
		uint32_t cross_price;
		uint64_t match_number;
		char cross_type;
	};

	struct BrokenTrade {
		char message_type; // B
		uint16_t stock_locate_code;
		uint16_t tracking_number;
		uint8_t timestamp[6];
		uint64_t match_number;
	};

	struct NetOrderImbalanceIndicator {
		char message_type; // I
		uint16_t stock_locate_code;
		uint16_t tracking_number;
		uint8_t timestamp[6];
		uint64_t paired_shares;
		uint64_t imbalance_shares;
		char imbalance_direction;
		char stock[8];
		uint32_t far_price;
		uint32_t near_price;
		uint32_t current_reference_price;
		char cross_type;
		char price_variation_indicator;
	};

	struct RetailPriceImprovementIndicator {
		char message_type; // N
		uint16_t stock_locate_code;
		uint16_t tracking_number;
		uint8_t timestamp[6];
		char stock[8];
		char interest_flag;
	};

	struct DirectListingWithCapitalRaisePriceDiscovery {
		char message_type; // O
		uint16_t stock_locate_code;
		uint16_t tracking_number;
		uint8_t timestamp[6];
		char stock[8];
		char open_eligibility_status;
		uint16_t minimum_allowable_price;
		uint16_t maximum_allowable_price;
		uint16_t near_execution_price;
		uint64_t near_execution_time;
		uint32_t lower_price_range_collar;
		uint32_t upper_price_range_collar;
	};

}

#pragma pack(pop)