#pragma once

//#include <quickfix/fix44/MarketDataRequest.h>
//#include <quickfix/fix50sp2/MarketDataRequest.h>

//#include <memory>
//#include <string>
//#include <vector>

class Router;

class Model {
	Router* router_;

public:
	Model(Router* router) : router_(router) {
	}
};
