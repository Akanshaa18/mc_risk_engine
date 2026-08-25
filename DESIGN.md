## Portfolio Composition

|     | Underlying | Type  | Moneyness | Strike           | Maturity | Direction   |
| --- | ---------- | ----- | --------- | ---------------- | -------- | ----------- |
| 1   | AAPL       | Stock | -         | -                | -        | Long 100 sh |
| 2   | AAPL       | Call  | ATM       | $300(spot x 1)   | 30d      | Long 10     |
| 3   | AAPL       | Put   | OTM       | $270(spot x 0.9) | 30d      | Short 10    |
| 4   | AAPL       | Call  | ITM       | $270(spot x 0.9) | 90d      | Long 5      |
| 5   | GOOG       | Put   | ATM       | $370(spot x 1)   | 30d      | Long 10     |
| 6   | GOOG       | Call  | OTM       | $407(spot x 1.1) | 90d      | Short 5     |

## Price Process

Model: Geometric Brownian Motion (GBM), single-step simulation (Δt = 1/252)
Drift (μ): zero
Correlation: injected via Cholesky decomposition of historical correlation matrix

Calibration data:
Source: Yahoo Finance (yfinance), daily close
Window: 2025-08-01 to 2026-08-01 (252 trading days)

Volatility (σ, annualized):
AAPL: 25.9%
GOOG: 31.4%

Correlation (AAPL, GOOG): 0.193
Note: lower than the ~0.4–0.6 range typically cited for large-cap tech
pairs; attributed to AAPL-specific idiosyncratic events (earnings,
litigation) during the sample window. Row counts verified matched
(no missing-data misalignment).

Cholesky lower-triangular matrix (L):
[1.000, 0.000]
[0.193, 0.981]

Drift rationale:
for a 1-day VaR horizon, drift is negligible relative to volatility (μΔt vs. σ√Δt — at Δt=1/252, the vol term dominates by roughly an order of magnitude). Historical mean returns (AAPL 43.0%, GOOG 63.8% annualized) reflect a recent bull-run and aren't a reliable forward-looking estimate. Risk-free rate is also inappropriate here: that belongs to the risk-neutral measure used for pricing (Black-Scholes), not the physical measure used for risk simulation — conflating the two would understate real-world tail risk if r diverges from actual expected returns. Using zero drift is standard practice for short-horizon VaR and keeps the simulation measure-consistent (physical P, not risk-neutral Q).

Risk-free rate (r): 3.71%
Source: ^IRX (13-week T-bill discount rate), Yahoo Finance
As of: 2026-08-07
Note: unlike σ and correlation, this is a market-observed snapshot,
not a historically-averaged estimate — using the latest available
quote as the simulation's risk-free rate.

## Pricing Method

Method: Full revaluation via closed-form Black-Scholes
Rationale: fast (no MC-within-MC needed), exact given BS assumptions,
appropriate since portfolio convexity (options) requires more than
delta-normal approximation.

Inputs:
S: simulated spot from GBM path (per scenario)
K: fixed, from position table
r: 3.71% (risk-free rate, point 2)
T: shrinks each horizon step — T_scenario = T_today − VaR_horizon
σ: historical realized vol from point 2 (25.9% AAPL, 31.4% GOOG),
reused for pricing — not implied vol (scoped out)

Simplifications (explicitly noted):

- European exercise assumed (ignores early-exercise premium of
  real American-style single-stock options)
- Dividend yield assumed zero (both <0.5% annual, negligible at
  1-day horizon)

## VaR Horizon

Horizon: 1-day
Rationale: matches GBM single-step simulation (Δt=1/252), aligns with
Kupiec's original test formulation, and maximizes independent backtest
observations for reasonable statistical power.

## Confidence Levels

Levels: 95% and 99%
Rationale: standard levels enabling comparison of Kupiec test behavior
across the distribution — 95% has enough expected breaches (~25 over
~500 day backtest) for reliable test power, while 99% (~5 expected
breaches) demonstrates how test power degrades in the deep tail with
limited data — itself a useful engine validation point to discuss.
