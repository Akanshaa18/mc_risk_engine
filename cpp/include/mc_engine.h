#pragma once
#include <string>
#include<vector>
#include "instrument.h"

struct MC_Engine {
    double s1_curr; //stock 1 - curr price
    double s2_curr; //stock 2 - curr price
    double drift;
    double r; //risk-free rate
    double vol1; //stock 1 volatility
    double vol2; //stock 1 volatility
    double del_t; //delta t
    int N; //number of simulations
    std::vector<std::vector<double>> cholesky_matrix;
    std::vector<Instrument>portfolio;
};

struct MC_returns {
    std::vector<double> pnl;
    double var95;
    double cvar95;
    double var99;
    double cvar99;
};

MC_returns riskEngine(MC_Engine mc_params);