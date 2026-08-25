# Big picture for pricer:

An option is a contract: the right (not obligation) to buy (call) or sell (put) a stock at a fixed price (the strike, K) by some future date (maturity, T). The question the pricer answers is: what is that right worth today?
If AAPL is at $300 today, what's a fair price for the right to buy it at $270 in 30 days? Too cheap and you're giving away free money; too expensive and no one buys it. Black-Scholes is the (famous, 1973) formula that answers this, given a few assumptions about how stock prices move randomly.

## Inputs:

Every pricing function takes: S, K, r, sigma, T

1. S — current stock price. Higher S → a call (right to buy cheap) is worth more; a put (right to sell high) is worth less.
2. K — strike price. The fixed price you're locked into.
3. r — risk-free interest rate. Money today is worth more than money later; r discounts the future strike payment back to today's dollars.
4. sigma (σ) — volatility, i.e., how much the stock tends to swing around annually. An option's value comes entirely from uncertainty. If a stock never moved, options would be worthless. More volatility = more chance of a big favorable swing = more valuable option, for both calls and puts.
5. T — time to maturity, in years. More time = more opportunity for the stock to swing favorably = more valuable option (usually).

## From Black Scholes Formula

1. log(S/K): if S>K then log(S/K) is positive which means the option is already "in the money"
2. Black Scholes formula uses normal CDF of d1 and d2
3. For a call: S _ normCDF(d1) - K _ exp(-rT) \* normCDF(d2)

Read this as two parts being netted against each other:

S _ normCDF(d1) — roughly, the expected value of receiving the stock, weighted by the probability-ish term normCDF(d1)
K _ exp(-r*T) * normCDF(d2) — the expected cost of paying the strike, discounted back to today's dollars

Note: exp(-r\*T) is the discount factor ie. money paid in the future is worth less today, weighted by the probability of actually exercising

The call price is "what you expect to get" minus "what you expect to pay". The discounting is what makes it a real price today rather than a future payoff.

4. For a put, it's the mirror image: K*exp(-rT)*normCDF(-d2) - S\*normCDF(-d1). Expected value of receiving the strike, minus expected value of giving up the stock, using -d1/-d2 because a put is profitable when the stock falls (the opposite direction).
5. Why the T<= 0.0 : When T<=0.0, option has expired. So the option is worth exactly its intrinsic value. A call is worth max(S - K, 0): if the stock is above the strike, you exercise and pocket the difference; if below, the option is worthless (you wouldn't exercise). This matters directly for the risk engine: when you shrink T_scenario = T_today - horizon, a 1-day option would hit exactly this case, and we want it to correctly return intrinsic value instead of crashing.

## Black Scholes Delta: why do we have this?

Delta measures how much the option's price moves for a $1 move in the stock. The role of this function is sanity check — we can verify if our full-revaluation P&L roughly matches delta × ΔS for small price moves, which is a good way to catch a sign error or a bug in the pricing logic before we trust the engine's output.
