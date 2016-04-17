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
#include <shared_mutex>
#include <mutex>
#include "PetriNetModel/PetriNetModel.h"

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
        { return _variance_sum / _total_weight; }

        double AverageVariance() const
        { return Variance() / _total_weight; }

        double AverageStandardDeviation() const
        { return std::sqrt(AverageVariance()); };

        double StandardDeviation() const
        { return std::sqrt(Variance()); }

        double VarianceSum() const
        { return _variance_sum; }

        double Average() const
        { return _average; }

        double TotalWeight() const
        { return _total_weight; }
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
        { return _summary.Average() - _summary.AverageStandardDeviation() * _z_abs; }

        double UpperBound() const
        { return _summary.Average() + _summary.AverageStandardDeviation() * _z_abs; }

        double Median() const
        { return _summary.Average(); }

        double Error() const
        { return _summary.AverageStandardDeviation() * _z_abs; }
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

    template<typename SampleType>
    class RandomVariableGeneric
    {
    public:
        typedef std::function<bool(const SampleType &, double &)> RandomVariableFunc;
    private:
        const string _name;
        const RandomVariableFunc _func;
    public:
        RandomVariableGeneric(const string &name, const RandomVariableFunc &func) : _name(name), _func(func)
        { }

        bool operator()(const SampleType &sample, double &value)
        { return _func(sample, value); }

        const string &GetName() const
        { return _name; }
    };


    template<typename SampleType>
    class RandomVariableEstimatorGeneric
    {
    private:
        typedef vector<SamplingSummary> SummaryList;
        typedef vector<SummaryList> SourceList;
        vector<RandomVariableGeneric<SampleType>> _random_variable_list;
        SourceList _source_list;
        std::shared_timed_mutex _mutex;
        SummaryList _result_summary_list;
    private:
        void AddNewSummary()
        {
            for (SummaryList &summary_list: _source_list)
            {
                summary_list.push_back(SamplingSummary());
            }
            _result_summary_list.push_back(SamplingSummary());
        }

    public:
        RandomVariableEstimatorGeneric(size_t source_count) : _source_list(source_count)
        {
        }

        void AddRandomVariable(const RandomVariableGeneric<SampleType> &random_variable)
        {
            _random_variable_list.push_back(random_variable);
            AddNewSummary();
        }

        void InputSample(size_t source_index, const SampleType &sample, double weight);

        void UpdateResult();

        SamplingSummary GetResult(size_t rand_var_index) const
        { return _result_summary_list[rand_var_index]; }

        const string &GetRandomVariableName(size_t rand_var_index) const
        {
            return _random_variable_list[rand_var_index].GetName();
        }

        size_t GetRandomVariableCount() const
        { return _random_variable_list.size(); }
    };

    template
    class RandomVariableGeneric<PetriNetModel::PetriNet>;

    template
    class RandomVariableEstimatorGeneric<PetriNetModel::PetriNet>;

    typedef RandomVariableGeneric<PetriNetModel::PetriNet> RandomVariable;
    typedef RandomVariableEstimatorGeneric<PetriNetModel::PetriNet> RandomVariableEstimator;

}

#endif //SPNP_ESTIMATOR_H
