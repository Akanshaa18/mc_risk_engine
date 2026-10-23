import pandas as pd
import numpy as np
from data_pipeline import fetch_price_history, fetch_risk_free_rate, build_portfolio, build_mc_engine, DEL_T
from kupiec import kupiec_test


import sys
sys.path.append("../build/cpp")
import risk_engine


def run_backtest(tickers = ["AAPL", "GOOG"], window = 60, N = 100000, lookback_days = 913):
    data = fetch_price_history(tickers, lookback_days)
    M = len(data)
    r = fetch_risk_free_rate()
    results = []

    for t in range(window, M-1):
        trailing_slice = data.iloc[t-window: t+1]

        s_aapl, s_goog = data["AAPL"].iloc[t], data["GOOG"].iloc[t]
        portfolio_today = build_portfolio(s_aapl, s_goog)                     # original maturities
        portfolio_tomorrow = build_portfolio(s_aapl, s_goog, time_elapsed=DEL_T)  # one day closer to expiry
        
        mc_params = build_mc_engine(trailing_slice, portfolio_today, r, N)
        result = risk_engine.riskEngine(mc_params)

        aapl_tom = data["AAPL"].iloc[t+1]
        goog_tom = data["GOOG"].iloc[t+1]

        value_today = risk_engine.portfolioValue(portfolio_today, s_aapl, s_goog, r, mc_params.vol1, mc_params.vol2)
        value_tomorrow = risk_engine.portfolioValue(portfolio_tomorrow, aapl_tom, goog_tom, r, mc_params.vol1, mc_params.vol2)
        actual_pnl = value_tomorrow - value_today

        results.append({
                "date": data.index[t],
                "var95": result.var95, "cvar95": result.cvar95,
                "var99": result.var99, "cvar99": result.cvar99,
                "actual_pnl": actual_pnl
        })
    return pd.DataFrame(results)

def summarize_backtest(results_df):
    n = len(results_df)
    breaches_95 = (results_df["actual_pnl"] < -results_df["var95"]).sum()
    breaches_99 = (results_df["actual_pnl"] < -results_df["var99"]).sum()

    kupiec_95 = kupiec_test(n=n, x=breaches_95, p=0.05)
    kupiec_99 = kupiec_test(n=n, x=breaches_99, p=0.01)


    print(f"n = {n}")
    print(f"95% VaR: {breaches_95} breaches (expected ~{0.05*n:.1f}) "
          f"-> LR={kupiec_95['LR_stat']:.3f}, p={kupiec_95['p_value']:.4f}, "
          f"{'PASS' if kupiec_95['passed'] else 'FAIL'}")
    print(f"99% VaR: {breaches_99} breaches (expected ~{0.01*n:.1f}) "
          f"-> LR={kupiec_99['LR_stat']:.3f}, p={kupiec_99['p_value']:.4f}, "
          f"{'PASS' if kupiec_99['passed'] else 'FAIL'}")

    return kupiec_95, kupiec_99

def run_single_day(tickers = ["AAPL", "GOOG"], window = 60, N = 100000, lookback_days = 913):
    """
    One risk engine call on the most recent trailing window, 
    preserving the full pnl array
    """
    data = fetch_price_history(tickers,lookback_days)
    r = fetch_risk_free_rate()

    trailing = data.iloc[-(window+1):]
    s_aapl, s_goog = trailing["AAPL"].iloc[-1], trailing["GOOG"].iloc[-1]
    portfolio = build_portfolio(s_aapl, s_goog)

    mc_params = build_mc_engine(trailing, portfolio, r, N)

    result = risk_engine.riskEngine(mc_params)
    pnl = np.array(result.pnl)
    assert np.isfinite(pnl).all(), f"{np.isnan(pnl).sum()} NaN values in pnl"
    return pnl, result


if __name__ == "__main__":
    results_df = run_backtest()
    results_df.to_csv('output.csv', index=False)
    print(results_df.shape)      
    summarize_backtest(results_df)
