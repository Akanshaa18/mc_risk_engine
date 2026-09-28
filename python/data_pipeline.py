import sys
import numpy as np
import pandas as pd
import yfinance as yf
from datetime import date, timedelta

sys.path.append("../build/cpp")   # wherever the .so actually is
import risk_engine

DEL_T = 1.0 / 252.0

def make_instrument(underlying, isOption, type, strike, maturity, quantity):
    inst = risk_engine.Instrument()
    inst.underlying = underlying
    inst.isOption = isOption
    inst.type = type
    inst.strike = strike
    inst.maturity = maturity
    inst.quantity = quantity
    return inst

def build_portfolio(time_elapsed=0.0):

    def mat(days):
        return days / 365.0 - time_elapsed
    
    return [
        make_instrument("AAPL", False, risk_engine.OptionType.Call, 0.0, 0.0, 100.0),
        make_instrument("AAPL", True, risk_engine.OptionType.Call, 300.0, mat(30), 10.0),
        make_instrument("AAPL", True, risk_engine.OptionType.Put, 270.0, mat(30), -10.0),
        make_instrument("AAPL", True, risk_engine.OptionType.Call, 270.0, mat(90), 5.0),
        make_instrument("GOOG", True, risk_engine.OptionType.Put, 370.0, mat(30), 10.0), 
        make_instrument("GOOG", True, risk_engine.OptionType.Call, 407.0, mat(90), -5.0)
    ]

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
    mc_params.del_t = DEL_T
    mc_params.N = N
    mc_params.cholesky_matrix = L.tolist()
    mc_params.portfolio = portfolio_template
    return mc_params

def fetch_price_history(tickers, lookback_days = 913):
    end = date.today()
    start = end - timedelta(lookback_days)
    data = yf.download(tickers, start = start, end = end, interval="1d")["Close"]
    return data

def fetch_risk_free_rate():
    raw = yf.download("^IRX", period="5d")["Close"].iloc[-1] / 100.0
    r = float(raw["^IRX"])
    return r