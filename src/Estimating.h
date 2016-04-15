//
// Created by wangnan on 16-1-19.
//

#ifndef SPNP_ESTIMATOR_H
#define SPNP_ESTIMATOR_H


#include <exception>
#include <string>
#include <vector>
#include <functional>
#include <cmath>
#include "Statistics.h"
#include <utility>
#include <memory>
#include <thread>
#include <sstream>

namespace Estimating
{
    using std::string;
    using std::vector;
    using std::unique_ptr;
    using std::pair;
    using std::thread;
    using std::ostringstream;

    class SamplingSummary
    {
    private:
        double _variance_sum = 0;
        double _average = 0;
        double _total_weight = 0;
    public:
        void AddNewSample(double sample, double weight)
        {
            _total_weight += weight;
            double old_average = _average;
            _average = old_average + (sample - old_average) * weight / _total_weight;
            _variance_sum = _variance_sum + weight * (sample - old_average) * (sample - _average);
        }

        double Variance() const
        {
            return _variance_sum / _total_weight;
        }

        double AverageVariance() const
        {
            return Variance() / _total_weight;
        }

        double AverageStandardDeviation() const
        {
            return std::sqrt(AverageVariance());
        };

        double StandardDeviation() const
        {
            return std::sqrt(Variance());
        }

        double VarianceSum() const
        {
            return _variance_sum;
        }

        double Average() const
        {
            return _average;
        }

        double TotalWeight() const
        {
            return _total_weight;
        }

        SamplingSummary &operator+=(const SamplingSummary &rhs)
        {
            double _total_weight = this->TotalWeight() + rhs.TotalWeight();
            double _average = this->Average() * this->TotalWeight() / _total_weight +
                              rhs.Average() * rhs.TotalWeight() / _total_weight;
            double _variance_sum = this->VarianceSum() + rhs.VarianceSum() +
                                   this->TotalWeight() * (this->Average() - _average) * (this->Average() - _average) +
                                   rhs.TotalWeight() * (rhs.Average() - _average) * (rhs.Average() - _average);
            this->_average = _average;
            this->_total_weight = _total_weight;
            this->_variance_sum = _variance_sum;
            return *this;
        }


    };

    inline SamplingSummary operator+(SamplingSummary lhs, const SamplingSummary &rhs)
    {
        lhs += rhs;
        return lhs;
    }

    class ConfidenceInterval
    {
    private:
        SamplingSummary _summary;
        double _confidence_coefficient = 0.95;
        double _z_abs;
    public:
        ConfidenceInterval(const SamplingSummary &summary) : _summary(summary)
        {
            double half_alpha = (1.0 - _confidence_coefficient) / 2.0;
            _z_abs = std::abs(Statistics::Ztable(half_alpha));
        }

        double LowerBound() const
        {
            return _summary.Average() - _summary.AverageStandardDeviation() * _z_abs;
        }

        double UpperBound() const
        {
            return _summary.Average() + _summary.AverageStandardDeviation() * _z_abs;
        }

        double Error() const
        {
            return _summary.AverageStandardDeviation() * _z_abs;
        }

        double RelativeError() const
        {
            if (_summary.Average() == 0.0)
            {
                return 1.0;
            } else
            {
                return Error() / _summary.Average();
            }
        }

        string ToString() const
        {
            ostringstream ss;
            ss << _summary.Average() << "±" << this->Error();
            return ss.str();
        }


    };

    class TargetPrecision
    {
    private:
        double _confidence_coefficient = 0.95;
        double _error = 0.05;
        enum ErrorType
        {
            Absolute,
            Relative,
            Infinite,
        } _error_type = Relative;
    public:
        TargetPrecision()
        { }

        bool IsSatisfied(const ConfidenceInterval &interval) const
        {
            switch (_error_type)
            {
                case Infinite:
                    return false;
                case Absolute:
                    return interval.Error() < _error;
                case Relative:
                    return interval.RelativeError() < _error;

            }
        }
    };


    class EndOfSample : public std::exception
    {
    };

    template<typename SampleType, typename SamplerType>
    class ExpectationEstimator
    {
    public:
        typedef std::function<bool(const SampleType &, double &, double &)> RandomVariableFunc;
    private:
        SamplerType _sampler;
        vector<pair<RandomVariableFunc, SamplingSummary>> _random_variable_list;
        thread _current_worker;
        bool _end_of_sample = false;

        static void worker(ExpectationEstimator &estimator, int sample_count)
        {
            try
            {
                if (sample_count > 0)
                {
                    estimator.Estimate(sample_count);
                } else
                {
                    estimator.Estimate();
                }
            } catch (EndOfSample)
            {
                estimator._end_of_sample = true;
            }
            estimator._end_of_sample = false;
        }

        void OneSample()
        {
            const SampleType &sample = _sampler.NextSample();
            for (auto &&pair:_random_variable_list)
            {
                RandomVariableFunc &func = pair.first;
                SamplingSummary &summary = pair.second;
                double value, weight;
                if (func(sample, value, weight))
                {
                    summary.AddNewSample(value, weight);
                }
            }
        }

    public:
        ExpectationEstimator(const SamplerType &sampler) : _sampler(sampler)
        { }

        ExpectationEstimator(const ExpectationEstimator<SampleType, SamplerType> &other) :
                _sampler(other._sampler), _random_variable_list(other._random_variable_list)
        { }

        void AddRandomVariable(const RandomVariableFunc &random_variable)
        {
            _random_variable_list.push_back(std::make_pair(random_variable, SamplingSummary{}));
        }

        void Estimate(int sample_count)
        {
            for (int i = 0; i < sample_count; i++)
            {
                OneSample();
            }
        }

        void Estimate()
        {
            while (true)
            {
                try
                {
                    OneSample();
                } catch (EndOfSample)
                {
                    break;
                }
            }
        }

        void EstimateAsync(int sample_count)
        {
            _current_worker = thread(worker, std::ref(*this), sample_count);
        }

        void EstimateAsync()
        {
            _current_worker = thread(worker, std::ref(*this), -1);
        }

        bool Wait()
        {
            _current_worker.join();
            return !_end_of_sample;
        }

        SamplingSummary GetResult(int index)
        {
            return _random_variable_list[index].second;
        }

    };


}

#endif //SPNP_ESTIMATOR_H
