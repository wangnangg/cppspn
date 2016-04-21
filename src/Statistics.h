//
// Created by wangnan on 16-1-26.
//

#ifndef SPNP_DISTRIBUTION_H
#define SPNP_DISTRIBUTION_H

#include <cmath>
#include <limits>
#include <random>
#include <functional>
#include <chrono>

namespace Statistics
{
    double Ztable(double prob);

    std::function<double(double uniform_rand_num)> Exp(double lambda);

    class UniformRandomNumberGenerator
    {
    public:
        virtual double GetVariate() = 0;

        ~UniformRandomNumberGenerator()
        { }
    };

    class DefaultUniformRandomNumberGenerator : public UniformRandomNumberGenerator
    {
    private:
        std::default_random_engine _generator;
        std::uniform_real_distribution<double> _uniform_dist;
    public:
        ~DefaultUniformRandomNumberGenerator()
        { }

        virtual double GetVariate() override
        {
            return _uniform_dist(_generator);
        }

        DefaultUniformRandomNumberGenerator()
        {
            unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
            _generator.seed(seed);
        }

        DefaultUniformRandomNumberGenerator(unsigned long seed)
        {
            _generator.seed(seed);
        }

    };

}


#endif //SPNP_DISTRIBUTION_H
