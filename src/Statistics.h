//
// Created by wangnan on 16-1-26.
//

#ifndef SPNP_DISTRIBUTION_H
#define SPNP_DISTRIBUTION_H

#include <cmath>
#include <limits>
#include <random>
#include <functional>

namespace Statistics
{
    double Ztable(double prob);

    extern std::default_random_engine generator;

    std::function<double()> Exp(double lambda);
}


#endif //SPNP_DISTRIBUTION_H
