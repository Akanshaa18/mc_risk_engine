import math
from scipy.stats import chi2


def kupiec_test(n, x, p, significance = 0.05):
    if x == 0:
        llr = -2 * ((n - x) * math.log(1 - p) - (n - x) * math.log(1 - x / n))
    else:
        llr = -2 * ((n - x) * math.log(1 - p) + x * math.log(p) - (n - x) * math.log(1 - x / n) - x * math.log(x / n))

    p_value = chi2.sf(llr, df=1)
    passed = p_value >= significance

    return {
        "n": n,
        "x": x,
        "p_expected": p,
        "breach_rate_observed": x/n,
        "LR_stat": llr,
        "p_value": p_value,
        "passed": passed
    }