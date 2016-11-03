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

    class SamplingResult
    {
    private:
        double _variance_sum = 0;
        double _average = 0;
        double _total_weight = 0;
        double _squared_weight_sum = 0;
    public:
        void AddNewSample(double sample, double weight)
        {
            if (weight <= 0.0)
            {
                return;
            }
            _total_weight += weight;
            double old_average = _average;
            _average = old_average + (sample - old_average) * weight / _total_weight;
            _variance_sum = _variance_sum + weight * (sample - old_average) * (sample - _average);
            _squared_weight_sum += weight * weight;
        }

        double Variance() const
        { return _variance_sum / _total_weight; }

        double AverageVariance() const
        { return Variance() / EffectiveBase(); }

        double EffectiveBase() const
        {
            return (_total_weight * _total_weight) / _squared_weight_sum;
        }


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

        SamplingResult &operator+=(const SamplingResult &rhs)
        {
            if (rhs._total_weight <= 0.0)
            {
                return *this;
            }
            double _total_weight = this->TotalWeight() + rhs.TotalWeight();
            double _average = this->_average * this->_total_weight / _total_weight +
                              rhs._average * rhs._total_weight / _total_weight;
            double _variance_sum = this->_variance_sum + rhs._variance_sum +
                                   this->_total_weight * (this->_average - _average) * (this->_average - _average) +
                                   rhs._total_weight * (rhs._average - _average) * (rhs._average - _average);
            double _squared_weight_sum = this->_squared_weight_sum + rhs._squared_weight_sum;
            this->_average = _average;
            this->_total_weight = _total_weight;
            this->_variance_sum = _variance_sum;
            this->_squared_weight_sum = _squared_weight_sum;
            return *this;
        }
    };

    inline SamplingResult operator+(SamplingResult lhs, const SamplingResult &rhs)
    {
        lhs += rhs;
        return lhs;
    }

    class ConfidenceInterval
    {
    private:
        SamplingResult _summary;
        double _confidence_coefficient = 0.95;
        double _z_abs;
    public:
        ConfidenceInterval(const SamplingResult &summary, double confidence_coefficient = 0.95) :
                _summary(summary), _confidence_coefficient(confidence_coefficient)
        {
            double half_alpha = (1.0 - _confidence_coefficient) / 2.0;
            _z_abs = std::abs(Statistics::StdNormQuantile(half_alpha));
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
        SamplingResult _result;
    public:
        RandomVariableGeneric(const string &name, const RandomVariableFunc &func) : _name(name), _func(func), _result()
        { }

        bool operator()(const SampleType &sample, double &value) const
        { return _func(sample, value); }

        const string &GetName() const
        { return _name; }

        const SamplingResult &GetSamplingResult() const
        { return _result; }

        void ClearResult()
        { _result = SamplingResult(); }

        void CombineResult(const SamplingResult &other)
        { _result += other; }

    };


    template<typename SampleType>
    class MeanEstimatorGeneric
    {
    private:
        typedef vector<SamplingResult> ResultList;
        typedef vector<ResultList> SourceList;
        typedef vector<RandomVariableGeneric<SampleType>> RandomVariableList;
        RandomVariableList _random_variable_list;
        SourceList _source_result_list;
        vector<std::mutex> _source_result_mutex_list;
        SourceList _source_mean_list;
    private:
        void AddNewResultToSource()
        {
            for (ResultList &result_list: _source_result_list)
            {
                result_list.push_back(SamplingResult());
            }
            for (ResultList &mean_list: _source_mean_list)
            {
                mean_list.push_back(SamplingResult());
            }
        }

    public:
        MeanEstimatorGeneric(size_t source_count) : _source_result_list(source_count),
                                                    _source_result_mutex_list(source_count),
                                                    _source_mean_list(source_count)
        {
        }

        MeanEstimatorGeneric(const MeanEstimatorGeneric<SampleType> &) = delete;

        void AddRandomVariable(const RandomVariableGeneric<SampleType> &random_variable)
        {
            _random_variable_list.push_back(random_variable);
            AddNewResultToSource();
        }

        void InputSample(size_t source_index, const SampleType &sample, double weight);

        void SubmitMean(size_t source_index);

        void ClearResult();

        void SubmitResult(size_t source_index);

        const vector<RandomVariableGeneric<SampleType>> &GetRandomVariableList() const
        { return _random_variable_list; }
    };

    template
    class RandomVariableGeneric<PetriNetModel::PetriNet>;

    template
    class MeanEstimatorGeneric<PetriNetModel::PetriNet>;

    typedef RandomVariableGeneric<PetriNetModel::PetriNet> RandomVariable;
    typedef MeanEstimatorGeneric<PetriNetModel::PetriNet> MeanEstimator;

}

#endif //SPNP_ESTIMATOR_H
