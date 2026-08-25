#include "pricer.h"
#include<cassert>
#include<cmath>
#include<iostream>

void test_known_value() {
    // S=100, K=100, r=0.05, sigma = 0.2, T=1 : Call ~ 10.4506
    double call = blackScholesPrice(100,100,0.05,0.2,1.0,OptionType::Call);
    assert(std::abs(call-10.4506)<0.001);
}

void test_put_call_parity() {
    double S = 100, K = 105, r =0.03, sigma =0.25, T=0.5;
    double call = blackScholesPrice(S, K, r, sigma, T, OptionType::Call);
    double put = blackScholesPrice(S, K, r, sigma, T, OptionType::Put);

    // C-P = S - K*e^(-rT)
    double lhs = call - put;
    double rhs = S - K * std::exp(-r *T);
    assert(std::abs(lhs-rhs)<1e-9);
}

void test_expiry() {
    double call = blackScholesPrice(110, 100, 0.05, 0.2, 0.0, OptionType::Call);
    assert(std::abs(call-10.0)<1e-9);
}

int main(){
    test_known_value();
    test_put_call_parity();
    test_expiry();
    std::cout<<"All tests passed for pricer\n";
    return 0;
}
