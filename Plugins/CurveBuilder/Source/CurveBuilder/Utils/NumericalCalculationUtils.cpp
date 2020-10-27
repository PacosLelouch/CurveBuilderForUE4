// Copyright 2020 PacosLelouch, Inc. All Rights Reserved.
// https://github.com/PacosLelouch/

#include "NumericalCalculationUtils.h"


double TGaussLegendre<GaussLegendreN>::Weights[GaussLegendreN] =
{ 0.5688888888888889, 0.4786286704993665, 0.4786286704993665, 0.2369268850561891, 0.2369268850561891 };
double TGaussLegendre<GaussLegendreN>::Abscissa[GaussLegendreN] =
{ 0.0000000000000000,-0.5384693101056831, 0.5384693101056831,-0.9061798459386640, 0.9061798459386640 };
