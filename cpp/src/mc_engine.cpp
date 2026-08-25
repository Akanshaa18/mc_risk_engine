#include<cmath>
#include<vector>
#include<iostream>
#include<random>
#include "pricer.h"
#include "instrument.h"

double gbm(double s1_curr, double s2_curr, double vol1, double vol2, double del_t, double drift, double Z1_corr, double Z2_corr){
    double s1_new = s1_curr * std::exp((drift - 0.5*vol1*vol1)* del_t + vol1*std::sqrt(del_t) * Z1_corr);
    double s2_new = s2_curr * std::exp((drift - 0.5*vol2*vol2)* del_t + vol2*std::sqrt(del_t) * Z2_corr);
    return s1_new, s2_new;
}
int main (){
    //input from DESIGN.md
    double s1_curr = 300.0, s2_curr = 370.0;
    double drift = 0.0;
    double r = 0.0371;
    double vol1 = 0.259, vol2 = 0.314;
    double del_t = 1.0/252.0;
    int N = 100000;
    std::vector<std::vector<double>> cholesky_matrix = {
        {1.000, 0.000},
        {0.193, 0.981}
    };
    std::vector<Instrument>portfolio = {
        {"AAPL", false, OptionType::Call, 0.0, 0.0, 100.0},
        {"AAPL", true, OptionType::Call, 300.0, 30.0/365.0, 10.0},
        {"AAPL", true, OptionType::Put, 270.0, 30.0/365.0, -10.0},
        {"AAPL", true, OptionType::Call, 270.0, 90.0/365.0, 5.0},
        {"GOOG", true, OptionType::Put, 370.0, 30.0/365.0, 10.0}, 
        {"GOOG", true, OptionType::Call, 407.0, 90.0/365.0, -5.0}
    };
    std::vector<Instrument> scenarioPortfolio = portfolio;
    for(auto& inst: scenarioPortfolio){
        if(inst.isOption) inst.maturity -= del_t;
    }
    
    double T = 1.0;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<double>dist(0.0,1.0);
    double Z1 = dist(gen);
    double Z2 = dist(gen);

    double Z1_correlated = cholesky_matrix[0][0] * Z1;
    double Z2_correlated = cholesky_matrix[1][0]*Z1 + cholesky_matrix[1][1]*Z2;

    double s1_new , s2_new = gbm(s1_curr,s2_curr,vol1, vol2, del_t, drift, Z1_correlated,Z2_correlated);
    double comp1 = blackScholesPrice(s1_new, 300.0, 3.71,vol1, 30.0, OptionType::Call);
    double comp2 = blackScholesPrice(s1_new, 270.0, 3.71,vol1, 30.0, OptionType::Put);
    double comp3 = blackScholesPrice(s1_new, 270.0, 3.71,vol1, 90.0, OptionType::Put);
    double comp4 = blackScholesPrice(s2_new, 370.0, 3.71,vol2, 30.0, OptionType::Put);
    double comp5 = blackScholesPrice(s2_new, 407.0, 3.71,vol2, 90.0, OptionType::Call);
    
}


