#include<cmath>
#include<vector>
#include<iostream>
#include<random>
#include<thread>
#include "pricer.h"
#include "instrument.h"

std::pair<double,double> gbm(double s1_curr, double s2_curr, double vol1, double vol2, double del_t, double drift, double Z1_corr, double Z2_corr){
    double s1_new = s1_curr * std::exp((drift - 0.5*vol1*vol1)* del_t + vol1*std::sqrt(del_t) * Z1_corr);
    double s2_new = s2_curr * std::exp((drift - 0.5*vol2*vol2)* del_t + vol2*std::sqrt(del_t) * Z2_corr);
    return {s1_new, s2_new};
}
double portfolioValue(std::vector<Instrument>& portfolio, double s_appl, double s_goog, double r, double vol1, double vol2){
    double total = 0.0;
    for(const auto& inst: portfolio){
        double spot = (inst.underlying == "AAPL") ? s_appl : s_goog;
        double vol = (inst.underlying == "AAPL") ? vol1 : vol2;
        if(!inst.isOption){
            total += inst.quantity * spot;
        }
        else{
            double price = blackScholesPrice(spot, inst.strike, r, vol, inst.maturity, inst.type);
            total += inst.quantity * price;
        }
    }
    return total;
}
void monte_carlo(int start, int end, unsigned int seed,
    std::vector<double>&pnl, double s1_curr, double s2_curr, double vol1, double vol2,
    double del_t, double drift, double r, const std::vector<std::vector<double>>& cholesky_matrix,
    std::vector<Instrument>& scenarioPortfolio, double todayValue
){
    
    std::mt19937 gen(seed);
    std::normal_distribution<double>dist(0.0,1.0);

    int num = (end - start)/2;

    for(int k=0;k<num;++k){
        double Z1 = dist(gen);
        double Z2 = dist(gen);

        double Z1_correlated = cholesky_matrix[0][0] * Z1;
        double Z2_correlated = cholesky_matrix[1][0]*Z1 + cholesky_matrix[1][1]*Z2;

        auto[s1_new, s2_new] = gbm(s1_curr,s2_curr,vol1, vol2, del_t, drift, Z1_correlated,Z2_correlated);

        double simValue = portfolioValue(scenarioPortfolio, s1_new, s2_new, r, vol1, vol2);
        pnl[start + 2*k] = simValue - todayValue;

        double Z1_anti_correlated = cholesky_matrix[0][0] * (-Z1);
        double Z2_anti_correlated = cholesky_matrix[1][0]*(-Z1) + cholesky_matrix[1][1]*(-Z2);

        auto[s1_new_anti, s2_new_anti] = gbm(s1_curr,s2_curr,vol1, vol2, del_t, drift, Z1_anti_correlated,Z2_anti_correlated);

        double simValueAnti = portfolioValue(scenarioPortfolio, s1_new_anti, s2_new_anti, r, vol1, vol2);
        pnl[start + 2*k + 1] = simValueAnti - todayValue;
    }
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

    //shrink each option's maturity by the VaR horizon
    std::vector<Instrument> scenarioPortfolio = portfolio;
    for(auto& inst: scenarioPortfolio){
        if(inst.isOption) inst.maturity -= del_t;
    }

    //today's portfolio value
    double todayValue = portfolioValue(portfolio, s1_curr, s2_curr, r, vol1, vol2);

    std::random_device rd;
    std::vector<double> pnl(N);
    unsigned int num_threads = std::thread::hardware_concurrency();
    if ((num_threads == 0) || (num_threads%2!=0)) num_threads = 4;

    std::vector<std::thread> threads;
    int base = N/num_threads;
    int remainder = N%num_threads;

    int start =0;

    for(unsigned int i=0; i<num_threads; ++i){
        int chunk = base + (i< static_cast<unsigned int>(remainder) ? 1: 0);
        int end = start + chunk;
        std::cout << "Thread " << i << ": [" << start << ", " << end << ")\n";
        threads.emplace_back(monte_carlo, start, end, rd(), std::ref(pnl), s1_curr, s2_curr,vol1, vol2, del_t, drift, 
        r, std::cref(cholesky_matrix), std::ref(scenarioPortfolio), todayValue);
        start = end;
    }
    for(auto& th: threads) th.join();

    std::cout << "Ran " << N << " scenarios across " << num_threads << " threads.\n";
    std::cout<<"First 5 P&L values:";
    for(int i=0;i<5; ++i) std::cout<<pnl[i]<<" \n";

    double sum = 0.0;
    for (double x : pnl) sum += x;
    double mean = sum / pnl.size();

    double sq_sum = 0.0;
    for (double x : pnl) sq_sum += (x - mean) * (x - mean);
    double stdev = std::sqrt(sq_sum / pnl.size());

    std::cout << "Mean P&L: " << mean << ", Std Dev: " << stdev << "\n";
    return 0;
}


