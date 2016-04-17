//
// Created by wangnan on 16-1-19.
//

#ifndef SPNP_SIMULATING_H
#define SPNP_SIMULATING_H

#include "PetriNetModel/PetriNetModel.h"
#include "Estimating.h"
#include <memory>

namespace Simulating
{
    using namespace PetriNetModel;
    using std::unique_ptr;
    using namespace Estimating;
    using std::shared_ptr;

    class PetriNetSimulator
    {
    private:
        PetriNet _petri_net;
        double _end_time;
        RandomVariableEstimator &_steady_state_estimator;
        RandomVariableEstimator &_transient_estimator;
        size_t _source_index;

    public:
        PetriNetSimulator(const PetriNet &petri_net,
                          RandomVariableEstimator &steady_state_estimator,
                          RandomVariableEstimator &transient_estimator,
                          double end_time,
                          size_t source_index)
                : _petri_net(petri_net), _steady_state_estimator(steady_state_estimator),
                  _transient_estimator(transient_estimator), _end_time(end_time), _source_index(source_index)
        { }

        // we require that _end_time < infinity
        void Run(int iteration_num, UniformRandomNumberGenerator &generator)
        {
            for (int i = 0; i < iteration_num; i++)
            {
                _petri_net.Reset(generator);
                while (_petri_net.NextFiringTime() < _end_time)
                {
                    _steady_state_estimator.InputSample(_source_index, _petri_net, _petri_net.StateDuration());
                    _petri_net.NextState(generator);
                }
                _steady_state_estimator.InputSample(_source_index, _petri_net, _end_time - _petri_net.GetTime());
                _transient_estimator.InputSample(_source_index, _petri_net, 1.0);
            }
        }

    };


};


#endif //SPNP_SIMULATING_H
