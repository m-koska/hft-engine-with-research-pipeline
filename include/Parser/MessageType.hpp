#pragma once

namespace Parser {
	enum MessageType : char {
		SystemEvent = 'S',
		StockDirectory = 'R',
		StockTradingAction = 'H',
		RegSHOShortSalePriceTestRestrictedIndicator = 'Y',
		MarketParticipantPosition = 'L',
		MarketWideCircuitBreaker = 'V',
		MarketWideCircuitBreakerStatus = 'W',
		QuotingPriceUpdate = 'K',
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