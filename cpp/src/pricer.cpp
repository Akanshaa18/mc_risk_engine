#include "pricer.h"
#include<cmath>

double normCDF(double x) {
    return 0.5 * std::erfc(-x / std::sqrt(2.0));
}

double blackScholesPrice(double S, double K, double r, double sigma, double T, OptionType type){
    if(T<= 0.0){
        double payoff = (type == OptionType::Call)? std::max(S-K,0.0): std::max(K-S, 0.0);
        return payoff;
    }
    double d1 = (std::log(S/K) + (r+0.5*sigma*sigma)*T)/(sigma * std::sqrt(T));
    double d2 = d1-sigma*std::sqrt(T);
    if(type == OptionType::Call){
        return S*normCDF(d1) - K*std::exp(-r *T)*normCDF(d2);
    }
    else{
        return K*std::exp(-r*T)*normCDF(-d2) - S*normCDF(-d1);
    }
}

double blackScholesDelta(double S, double K, double r, double sigma, double T, OptionType type) {
    if (T <= 0.0) {
        double intrinsic_delta = (type == OptionType::Call) ? (S > K ? 1.0 : 0.0) : (S < K ? -1.0 : 0.0);
        return intrinsic_delta;
    }
    double d1 = (std::log(S / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
    return (type == OptionType::Call) ? normCDF(d1) : normCDF(d1) - 1.0;
}