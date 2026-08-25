#pragma once
#include<string>

enum class OptionType {Call, Put};

struct Instrument {
    std::string underlying; //"AAPL", "GOOG"
    bool isOption; //false when it is a plain stock
    OptionType type;
    double strike;
    double maturity;  //in years
    double quantity; //signed; so positive: long and negative: short
};
