#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <math.h>

void getFormattedValAndUncertainty(const double val, const double err, char *str, const long unsigned int strLength, const int showErr, const int roundErr);
void getFormattedYAxisVal(const double val, const double axisMinVal, const double axisMaxVal, char *str, const long unsigned int strLength);
int getNSigf(const double val, const double oomVal);

#endif