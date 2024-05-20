#include "Model.h"

#include <quickfix/fix44/MarketDataRequest.h>
#include <quickfix/fix44/MarketDataSnapshotFullRefresh.h>

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

