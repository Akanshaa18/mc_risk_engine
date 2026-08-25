"""
Determine real sigma and correlation values which are going to be used for GBM
"""
import yfinance as yf
import numpy as np
import pandas as pd

tickers = ["AAPL", "GOOG"]
data = yf.download(tickers, start="2025-08-01", end ="2026-08-01", interval="1d")["Close"]
log_returns = np.log(data/data.shift(1)).dropna()
print(log_returns.shape)
print(log_returns.isna().sum())
sigma_annual = log_returns.std() * np.sqrt(252)
print("\nAnnualized volatility:", sigma_annual)
corr_matrix = log_returns.corr()
print("\nCorrelation matrix:", corr_matrix)
L = np.linalg.cholesky(corr_matrix.values)
print("\nCholesky lower-triangular matrix:\n", L)

mu_daily = log_returns.mean()
mu_annual = mu_daily * 252
#Only for reference, not to use as drift
print("\nAnnualized mean return:", mu_annual)

irx = yf.download("^IRX", period="5d")["Close"]
print(irx)