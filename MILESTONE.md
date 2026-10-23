## Milestone Breakdown: MC Risk Engine + VaR/CVaR + Kupiec Backtest

### Phase 0: Design decisions

- **Portfolio composition**: pick 2-3 underlyings, 4-6 options total (mix of calls/puts, a couple strikes/maturities) + maybe the underlying stock itself.
- **Price process**: GBM, correlated via Cholesky if >1 underlying.
- **Pricing method for repricing under scenarios**: full revaluation using Black-Scholes closed-form.
- **VaR horizon**: 1-day VaR is simplest and matches Kupiec's original setup
- **Confidence levels**: 95% and 99%

**Deliverable**:`DESIGN.md` with these decisions written down.

---

### Phase 1: C++ core — instruments & BS pricer

- Define `Instrument` types
- Implement Black-Scholes closed-form pricing + delta
- Unit tests: known BS values against a reference calculator, put-call parity check

**Deliverable**: `pricer.cpp/.h`, passes unit tests, put-call parity holds to floating point tolerance.

---

### Phase 2: C++ core — Monte Carlo scenario engine

- GBM path generator for correlated underlyings (Cholesky decomposition of correlation matrix)
- Scenario loop: for N scenarios, simulate underlying price(s) forward by the VaR horizon, reprice every instrument in the portfolio under that scenario, sum to portfolio value, compute P&L vs. current value
- Multithread the scenario loop
- Variance reduction: antithetic variates

## **Deliverable**: engine that outputs a vector of N simulated portfolio P&L values.

### Phase 3: C++ core — VaR/CVaR calculation

- Sort simulated P&L, extract the empirical quantile for VaR at 95%/99%
- CVaR: average of all losses beyond the VaR threshold

**Deliverable**: given a portfolio and market params, engine returns VaR and CVaR at both confidence levels

---

### Phase 4: pybind11 bindings

- Wrap `Portfolio`, `MarketParams`, and `computeVaR` for Python
- Build a minimal Python script that calls the C++ engine and prints results

**Deliverable**: `import risk_engine` works in Python, returns correct VaR/CVaR matching C++ unit test values.

---

### Phase 5: Python — historical data pipeline

- Pull historical prices for chosen underlyings (yfinance)
- Compute rolling historical volatility and correlation (GBM inputs), lookback window
- Structure a rolling-window backtest loop

**Deliverable**: a DataFrame with one row per day: date, VaR*95, VaR_99, CVaR_95, CVaR_99, and the \_actual* next-day portfolio P&L (computed from real historical price moves).

---

### Phase 6: Kupiec backtest + breach analysis

- For each day, flag a "breach" if actual loss exceeded VaR
- Implement Kupiec's POF likelihood-ratio test statistic, compare breach rate to expected rate at each confidence level
- Report: number of breaches, expected number, LR statistic, pass/fail at standard significance

**Deliverable**: a results table + interpretation

---

### Phase 7: Visualization

- Plot 1: P&L distribution histogram with VaR/CVaR markers (from a single scenario run)
- Plot 2: time series of daily VaR bands vs. actual P&L with breaches marked
- Plot 3: breach clustering over time

**Deliverable**: plots
