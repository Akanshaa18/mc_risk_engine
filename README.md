# Monte Carlo Risk Engine

A C++ Monte Carlo engine that estimates 1-day Value at Risk (VaR) and Conditional VaR (CVaR) for a small equity-options portfolio, exposed to Python through pybind11. A Python layer runs a rolling-window backtest on historical AAPL/GOOG data and validates the VaR forecasts with Kupiec's Proportion-of-Failures (POF) test.

A separate write-up of the results and the reasoning behind the design decisions lives in [`docs/results_report.pdf`](docs/results_report.pdf).

## What it does

1. Simulates correlated 1-day price moves for two underlyings using Geometric Brownian Motion, with correlation introduced through a Cholesky factor.
2. Reprices the whole portfolio in every scenario with closed-form Black-Scholes (not a delta approximation), with option maturities reduced by the 1-day horizon.
3. Computes VaR and CVaR at 95% and 99% from the sorted scenario P&L.
4. Backtests day by day: each day, volatility and correlation are re-estimated from a trailing window, the engine produces a forecast, and the forecast is compared with the portfolio's realized next-day (hypothetical) P&L.
5. Tests calibration with the Kupiec POF test at both confidence levels and plots the results.

### C++ core

- Black-Scholes pricer and delta, with an intrinsic-value branch at expiry
- Multithreaded scenario loop: each thread has its own seeded `std::mt19937` and writes to a disjoint slice of the output vector, so no locking is needed
- Antithetic variates: every draw also produces its mirrored scenario
- VaR/CVaR extraction (`var_cvar`) and a single entry point, `riskEngine(MC_Engine)`, returning `MC_returns`
- The core is a plain static library (`risk_core`) with no Python dependency; pybind11 is confined to `bindings.cpp`

### Python layer

- `data_pipeline.py`: price and risk-free-rate download, portfolio construction, and `build_mc_engine` (rolling vol, correlation, Cholesky)
- `backtest.py`: the rolling backtest loop and the summary printout
- `kupiec.py`: Kupiec POF likelihood-ratio test
- `visualize.py`: three plots (single-day P&L distribution, VaR vs actual P&L, 99% breach timeline)

## Repository layout

```
mc-risk-engine/
├── DESIGN.md
├── README.md
├── MILESTONE.md
├── calculate.py
├── CMakeLists.txt
├── cpp/
│   ├── CMakeLists.txt
│   ├── include/        instrument.h, pricer.h, mc_engine.h, ...
│   ├── src/            pricer.cpp, mc_engine.cpp, ...
│   ├── bindings/       bindings.cpp
│   └── tests/          test_pricer.cpp
├── python/
│   ├── data_pipeline.py
│   ├── backtest.py
│   ├── kupiec.py
│   └── visualize.py
|   ├── output.csv
├── docs/               results_report.pdf
├── figures/
└── requirements.txt
```

## Requirements

- A C++20 compiler
- CMake 3.15+
- Python 3.9+ with a virtual environment
- Python packages: `pybind11`, `numpy`, `pandas`, `scipy`, `yfinance`, `matplotlib`, `pytest`

## Setup

```bash
# 1. Python environment
python3 -m venv mc_env
source mc_env/bin/activate
pip install -r requirements.txt        # make sure pybind11 is listed

# 2. Build the C++ library and the Python module
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_PREFIX_PATH=$(python -m pybind11 --cmakedir) \
         -DPython3_EXECUTABLE=$(which python)
make -j4
cd ..
```

The built module lands in `build/cpp/`. The Python scripts add `../build/cpp` to `sys.path`, so run them from inside `python/`.

## Running

```bash
./build/cpp/test_pricer

cd python
python backtest.py

python visualize.py
```

## Configuration

| What                                                               | Where                                    | Default                  |
| ------------------------------------------------------------------ | ---------------------------------------- | ------------------------ |
| Portfolio (instruments, moneyness multipliers, tenors, quantities) | `build_portfolio` in `data_pipeline.py`  | 6 positions on AAPL/GOOG |
| Trailing window for vol/correlation                                | `window` in `run_backtest`               | 60 trading days          |
| History pulled                                                     | `lookback_days` in `fetch_price_history` | 913 calendar days        |
| Scenarios per forecast                                             | `N` in `run_backtest`                    | 100,000 (must be even)   |
| Horizon                                                            | `DEL_T` in `data_pipeline.py`            | 1/252                    |
| Drift                                                              | `build_mc_engine`                        | 0                        |

## Notes

- The history window ends on the day you run the code, so numbers differ between runs on different days. The write-up in `docs/` describes one specific run and records its dates.
- Option strikes are set each day as multiples of that day's spot, so the portfolio keeps the same moneyness profile across the whole backtest.

## References

- Kupiec, P. (1995). Techniques for Verifying the Accuracy of Risk Measurement Models. _The Journal of Derivatives_, 3(2), 73-84.
- He, P. (2012). Option Portfolio Value at Risk Using Monte Carlo Simulation Under a Risk Neutral Stochastic Implied Volatility Model. SSRN 2146103.
- Black, F. and Scholes, M. (1973). The Pricing of Options and Corporate Liabilities. _Journal of Political Economy_, 81(3), 637-654.
