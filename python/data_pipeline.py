import sys
import numpy as np
import pandas as pd
import yfinance as yf

sys.path.append("../build/cpp")   # wherever the .so actually is
import risk_engine


r = (yf.download("^IRX", period="5d")["Close"].iloc[-1])/100.0

def build_mc_engine(trailing_prices_df, portfolio_template, r, N) -> risk_engine.MC_Engine:
    log_returns = np.log(trailing_prices_df/trailing_prices_df.shift(1)).dropna()
    sigma_annual = log_returns.std() * np.sqrt(252)
    corr_matrix = log_returns.corr()
    L = np.linalg.cholesky(corr_matrix.values)

    mc_params = risk_engine.MC_Engine()
    mc_params.s1_curr = trailing_prices_df["AAPL"].iloc[-1]
    mc_params.s2_curr = trailing_prices_df["GOOG"].iloc[-1]
    mc_params.drift = 0.0
    mc_params.r = r
    mc_params.vol1 = sigma_annual["AAPL"]
    mc_params.vol2 = sigma_annual["GOOG"]
    mc_params.del_t = 1.0/252.0
    mc_params.N = N
    mc_params.cholesky_matrix = L.tolist()
    mc_params.portfolio = portfolio_template
    return mc_params


    


    