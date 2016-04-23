//
// Created by wangnan on 16-1-28.
//

#include "Statistics.h"
#include <cmath>

namespace Statistics
{
    double StdNormDistCDF(double x)
    {
        double mu = 0.0;
        double sigma = 1.0;
        return 0.5 * (1.0 +
                      std::erf(
                              (x - mu) / (std::sqrt(2.0) * sigma)
                      )
        );
    }

    double StdNormQuantile(double p)
    {
        double x = -1.0;
        double step = 0.5;
        double prob = StdNormDistCDF(x);
        while (prob > p)
        {
            step *= 2.0;
            x -= step;
            prob = StdNormDistCDF(x);
        }
        while (std::abs(prob - p) > 1e-5)
        {
            step = step / 2.0;
            if (prob < p)
            {
                x += step;
            } else
            {
                x -= step;
            }
            prob = StdNormDistCDF(x);
        }

        return x;
    }



    std::function<double(double uniform_rand_num)> Exp(double lambda)
    {
        return [lambda](double p)
        { return -std::log(1 - p) / lambda; };
    }

    std::function<double(double uniform_rand_num)> ParetoTrunc(double alpha, double m, double n)
    {
        return [alpha, m, n](double p)
        {
            double c = 1.0 / (1.0 - std::pow(m / n, alpha));
            return m / std::pow(1.0 - p / c, 1.0 / alpha);
        };
    }

    std::function<double(double uniform_rand_num)> Weibull(double k, double theta)
    {
        return [k, theta](double p)
        {
            return theta * std::pow(std::log(1.0 / (1.0 - p)), 1.0 / k);
        };
    }

    std::function<double(double uniform_rand_num)> Deterministic(double t)
    {
        return [t](double p)
        {
            return t;
        };
    }

}
