## Milestone Breakdown: MC Risk Engine + VaR/CVaR + Kupiec Backtest

### Phase 0: Design decisions (do this before writing code — ~2-3 hrs)

Lock these in now so you don't re-architect mid-build:

- **Portfolio composition**: pick 2-3 underlyings, 4-6 options total (mix of calls/puts, a couple strikes/maturities) + maybe the underlying stock itself. Keep it small — this is a risk engine demo, not a real book.
- **Price process**: GBM, correlated via Cholesky if >1 underlying. Don't reach for jump-diffusion or stochastic vol — out of scope for narrow version.
- **Pricing method for repricing under scenarios**: full revaluation using Black-Scholes closed-form (fast enough since it's just BS, no need for MC-within-MC)
- **VaR horizon**: 1-day VaR is simplest and matches Kupiec's original setup — use this unless you have a reason not to.
- **Confidence levels**: 95% and 99% (standard, lets you show the Kupiec test behaves differently at each)

**Deliverable**: a short `DESIGN.md` with these decisions written down. This alone is a good interview artifact — shows you make deliberate scoping choices.

---

### Phase 1: C++ core — instruments & BS pricer (Day 1-2)

- Define `Instrument` types: `Stock`, `EuropeanOption` (call/put, strike, maturity)
- Implement Black-Scholes closed-form pricing + delta (delta isn't strictly required for VaR but useful for sanity-checking full revaluation)
- Unit tests: known BS values against a reference calculator, put-call parity check

**Deliverable**: `pricer.cpp/.h`, passes unit tests, put-call parity holds to floating point tolerance.

---

### Phase 2: C++ core — Monte Carlo scenario engine (Day 2-4)

- GBM path generator for correlated underlyings (Cholesky decomposition of correlation matrix)
- Scenario loop: for N scenarios, simulate underlying price(s) forward by the VaR horizon, reprice every instrument in the portfolio under that scenario, sum to portfolio value, compute P&L vs. current value
- Multithread the scenario loop (`std::thread` or a thread pool — split N scenarios across cores)
- Variance reduction: antithetic variates (straightforward to add on top of existing RNG calls)

**Deliverable**: engine that outputs a vector of N simulated portfolio P&L values. Sanity check: single-stock-only portfolio's P&L distribution should look normal (validates your GBM + repricing logic before options complexity is layered in).

---

### Phase 3: C++ core — VaR/CVaR calculation (Day 4-5)

- Sort simulated P&L, extract the empirical quantile for VaR at 95%/99%
- CVaR: average of all losses beyond the VaR threshold
- Expose a clean function signature: `RiskResult computeVaR(Portfolio, MarketParams, int numScenarios, double confidence)`

**Deliverable**: given a portfolio and market params, engine returns VaR and CVaR at both confidence levels. This is your core C++ library, essentially done.

---

### Phase 4: pybind11 bindings (Day 5-6, ~1 day)

- Wrap `Portfolio`, `MarketParams`, and `computeVaR` for Python
- Build a minimal Python script that calls the C++ engine and prints results — confirms the binding works end-to-end before you build anything more complex on top

**Deliverable**: `import risk_engine` works in Python, returns correct VaR/CVaR matching your C++ unit test values.

---

### Phase 5: Python — historical data pipeline (Day 6-7)

- Pull historical prices for your chosen underlyings (yfinance)
- Compute rolling historical volatility and correlation (your GBM inputs) — decide on a lookback window (e.g., 60-day rolling)
- Structure a rolling-window backtest loop: for each day in your historical test period, use trailing data to parameterize GBM, call your C++ engine to get that day's VaR/CVaR forecast, store it

**Deliverable**: a DataFrame with one row per day: date, VaR_95, VaR_99, CVaR_95, CVaR_99, and the _actual_ next-day portfolio P&L (computed from real historical price moves).

---

### Phase 6: Kupiec backtest + breach analysis (Day 7-8)

- For each day, flag a "breach" if actual loss exceeded VaR
- Implement Kupiec's POF likelihood-ratio test statistic, compare breach rate to expected rate at each confidence level
- Report: number of breaches, expected number, LR statistic, pass/fail at standard significance (e.g., 95%)

**Deliverable**: a results table + interpretation (does your engine's 99% VaR get breached roughly 1% of the time, as it should?).

---

### Phase 7: Visualization + writeup (Day 8-9/10)

- Plot 1: P&L distribution histogram with VaR/CVaR markers (from a single scenario run)
- Plot 2: time series of daily VaR bands vs. actual P&L with breaches marked
- Plot 3: breach clustering over time (sets up a natural "Christoffersen would test this" mention in your README even if unimplemented)
- README: motivation → design decisions → architecture (C++ engine / Python backtest split) → results → Kupiec interpretation → references (Kupiec 1995, He 2012)

**Deliverable**: polished repo, ready to link on your resume/LinkedIn.

---

### Buffer

Day 10 as slack — Cholesky/multithreading debugging or data pipeline quirks (yfinance gaps, dividend adjustments) are the most likely places you lose time.

---

**One thing to flag now, not later**: decide your repo structure before Phase 1 — a `cpp/` directory with its own CMakeLists, and a `python/` directory for the backtest, with pybind11 bridging them. Doing this from day one avoids a painful restructure in Phase 4.

Want help setting up the C++ dev environment / CMake + pybind11 build config next, or do you want to start with the BS pricer implementation itself?
