#include "Model.h"

#include <quickfix/fix44/MarketDataRequest.h>

void Model::Subscribe(const std::string& mdReqID, const std::string& symbol, const std::string& sessionID) {
	subscriber_.Subscribe(mdReqID, symbol, sessionID);
}

void Model::Publish(std::string symbol, Subscribers subscribers, Orderbook orderbook) {
	for (const auto& [mdReqID, sessionID] : subscribers) {
		if (auto mdSnapshotMsg = create<FIX44::MarketDataSnapshotFullRefresh>(mdReqID, symbol)) {
			FIX::Session::sendToTarget(*mdSnapshotMsg, sessionID);
		}
	}
}

std::vector<std::string> Model::getSymbols(const FIX44::MarketDataRequest& msg) {
	std::vector<std::string> symbols;

	FIX::NoRelatedSym noRelatedSym;
	msg.getField(noRelatedSym);

	for (int i = 1; i <= noRelatedSym; ++i) {
		FIX44::MarketDataRequest::NoRelatedSym noRelatedSymGroup;
		msg.getGroup(1, noRelatedSymGroup);

		FIX::Symbol symbol;
		noRelatedSymGroup.get(symbol);
		symbols.push_back(symbol.getValue());
	}

	return symbols;
}

