import os
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.dates as mdates

FIG_DIR = "../figures"

def saveimage(fig, filename):
    os.makedirs(FIG_DIR, exist_ok=True)
    path = os.path.join(FIG_DIR,filename)
    fig.savefig(path, dpi=200, bbox_inches="tight")
    plt.close(fig)
    print(f"Saved {path}")

def add_breach_columns(df):
    df = df.copy()
    df["breach_95"] = df["actual_pnl"] < -df["var95"]
    df["breach_99"] = df["actual_pnl"] < -df["var99"]
    df["exceedance_99"] = -df["actual_pnl"] / df["var99"]   # >1 on breach days
    return df

#Plot: simulated P&L distribution with VaR/CVaR markers
def plot_pnl_distr(pnl, result, filename ="pnl_distr.png"):
    fig, ax = plt.subplots(figsize=(10,6))
    ax.hist(pnl, bins=200, color="steelblue", alpha=0.7, density=True)

    lines = [
        (-result.var95,  "VaR 95%",  "orange", "-"),
        (-result.cvar95, "CVaR 95%", "orange", "--"),
        (-result.var99,  "VaR 99%",  "red",    "-"),
        (-result.cvar99, "CVaR 99%", "red",    "--"),
    ]
    for x, label, color, style in lines:
        ax.axvline(x, color=color, linestyle=style, linewidth=1.8,
                   label=f"{label}: ${-x:,.0f}")

    ax.set_title(f"Simulated 1-day portfolio P&L distribution, N={len(pnl):,}")
    ax.set_xlabel("P&L in $")
    ax.set_ylabel("Density")
    ax.legend()
    saveimage(fig, filename)

#Plot: VaR bands vs actual P&L, breaches are marked

def plot_var_vs_actual(df, filename="var_vs_actual.png"):
    df = add_breach_columns(df)
    fig, ax = plt.subplots(figsize=(14, 6))

    ax.plot(df["date"], df["actual_pnl"], color="grey", linewidth=0.8, label="Actual P&L")
    ax.plot(df["date"], -df["var95"], color="orange", linewidth=1.2, label="-VaR 95%")
    ax.plot(df["date"], -df["var99"], color="red", linewidth=1.2, label="-VaR 99%")

    only_95 = df[df["breach_95"] & ~df["breach_99"]]
    both = df[df["breach_99"]]
    ax.scatter(only_95["date"], only_95["actual_pnl"], color="orange", s=30, zorder=3,
               label=f"95% breach only ({len(only_95)})")
    ax.scatter(both["date"], both["actual_pnl"], color="red", s=45, zorder=4,
               marker="X", label=f"99% breach ({len(both)})")

    ax.axhline(0, color="black", linewidth=0.5)
    ax.set_title("Rolling 1-day VaR forecasts vs realized hypothetical P&L")
    ax.set_xlabel("Date")
    ax.set_ylabel("P&L in $")
    ax.legend(loc="lower left")
    saveimage(fig, filename)

#Plot: 99% breach timeline, sized/colored by severity
def plot_breach_timeline(df, filename="breach_timeline.png"):
    df = add_breach_columns(df)
    b = df[df["breach_99"]]

    fig, ax = plt.subplots(figsize=(14, 5))
    sc = ax.scatter(b["date"], b["exceedance_99"],
                    c=b["exceedance_99"], cmap="Reds", edgecolor="black",
                    s=60 + 250 * (b["exceedance_99"] - 1.0), vmin=1.0, zorder=3)
    ax.vlines(b["date"], 1.0, b["exceedance_99"], color="grey", linewidth=0.8, zorder=2)
    ax.axhline(1.0, color="black", linestyle="--", linewidth=1, label="Breach threshold (loss = VaR 99%)")

    ax.set_xlim(df["date"].min(), df["date"].max())
    ax.xaxis.set_major_locator(mdates.MonthLocator(interval=2))
    ax.xaxis.set_major_formatter(mdates.DateFormatter("%b %Y"))
    plt.setp(ax.get_xticklabels(), rotation=45, ha="right")

    ax.set_title("99% VaR breaches: timing and severity")
    ax.set_xlabel("Date")
    ax.set_ylabel("Loss / VaR 99%  (exceedance ratio)")
    fig.colorbar(sc, ax=ax, label="Exceedance ratio")
    ax.legend()
    saveimage(fig, filename)

    print(f"Mean exceedance ratio on 99% breaches: {b['exceedance_99'].mean():.2f}x, "
          f"max: {b['exceedance_99'].max():.2f}x")


if __name__ == "__main__":
    from backtest import run_single_day

    df = pd.read_csv("output.csv", parse_dates=["date"])
    pnl, result = run_single_day()
    plot_pnl_distr(pnl,result)
    plot_var_vs_actual(df)
    plot_breach_timeline(df)