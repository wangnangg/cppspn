//
// Created by wangnan on 16-1-19.
//

#include "Estimating.h"

using std::function;
namespace Estimating
{
    template<typename SampleType>
    void RandomVariableEstimatorGeneric<SampleType>::InputSample(size_t source_index, const SampleType &sample,
                                                                 double weight)
    {
        SummaryList &summary_list = _source_list[source_index];
        std::shared_lock<std::shared_timed_mutex> lock(_mutex);
        for (size_t rand_index = 0; rand_index < _random_variable_list.size(); rand_index++)
        {
            RandomVariableGeneric<SampleType> &rand_variable = _random_variable_list[rand_index];
            SamplingSummary &summary = summary_list[rand_index];
            double value;
            if (rand_variable(sample, value))
            {
                summary.AddNewSample(value, weight);
            }
        }
    }

    template<typename SampleType>
    void RandomVariableEstimatorGeneric<SampleType>::UpdateResult()
    {
        std::unique_lock<std::shared_timed_mutex> lock(_mutex);
        for (SamplingSummary &summary : _result_summary_list)
        {
            summary = SamplingSummary();
        }
        for (size_t rand_index = 0; rand_index < _random_variable_list.size(); rand_index++)
        {
            for (size_t source_index = 0; source_index < _source_list.size(); source_index++)
            {
                _result_summary_list[rand_index] += _source_list[source_index][rand_index];
            }
        }
    }
}
