#pragma once

namespace Parser {
	/// @brief Represents standard NASDAQ ITCH 5.0 message type codes.
	/// @details Strongly typed to prevent implicit conversions during parsing.
	enum MessageType : char {
		SystemEvent = 'S',
		StockDirectory = 'R',
		StockTradingAction = 'H',
		RegSHOShortSalePriceTestRestrictedIndicator = 'Y',
		MarketParticipantPosition = 'L',
		MarketWideCircuitBreaker = 'V',
		MarketWideCircuitBreakerStatus = 'W',
		QuotingPeriodUpdate = 'K',
		LimitUpLimitDownAuctionCollar = 'J',
		OperationalHalt = 'h',
		AddOrderA = 'A',
		AddOrderF = 'F',
		OrderExecuted = 'E',
		OrderExecutedWithPrice = 'C',
		OrderCancel = 'X',
		OrderDelete = 'D',
		OrderReplace =  'U',
		Trade = 'P',
		CrossTrade = 'Q',
		BrokenTrade = 'B',
		NetOrderImbalanceIndicator = 'I',
		RetailPriceImprovementIndicator = 'N',
		DirectListingWithCapitalRaisePriceDiscovery = 'O'
	};
}