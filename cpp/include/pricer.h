#pragma once
#include "instrument.h"

double normCDF(double x);
double blackScholesPrice(double S, double K, double r, double sigma, double T, OptionType type);
double blackScholesDelta(double S, double K, double r, double sigma, double T, OptionType type);
