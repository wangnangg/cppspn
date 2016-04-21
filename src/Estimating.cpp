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
        ResultList &result_list = _source_list[source_index];
        for (size_t rand_index = 0; rand_index < _random_variable_list.size(); rand_index++)
        {
            const RandomVariableGeneric<SampleType> &rand_variable = GetRandomVariable(rand_index);
            SamplingResult &result = result_list[rand_index];
            double value;
            if (rand_variable(sample, value))
            {
                result.AddNewSample(value, weight);
            }
        }
    }


    template<typename SampleType>
    void RandomVariableEstimatorGeneric<SampleType>::ClearResult()
    {
        for (auto &rand_var:_random_variable_list)
        {
            rand_var.ClearResult();
        }
    }

    template<typename SampleType>
    void RandomVariableEstimatorGeneric<SampleType>::SubmitResult(size_t source_index)
    {
        for (size_t rand_index = 0; rand_index < _random_variable_list.size(); rand_index++)
        {
            _random_variable_list[rand_index].CombineResult(_source_list[source_index][rand_index]);
        }
    }

}
