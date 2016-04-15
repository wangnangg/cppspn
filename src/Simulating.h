//
// Created by wangnan on 16-1-19.
//

#ifndef SPNP_SIMULATING_H
#define SPNP_SIMULATING_H

#include "PetriNetModel.h"
#include "Estimating.h"
#include <memory>

namespace Simulating
{
    using namespace PetriNetModel;
    using std::unique_ptr;
    using namespace Estimating;
    using std::shared_ptr;


    typedef void (*SimEventCallback)(const PetriNetDynamic &dynamic);

    class PetriNetSimulator
    {
    private:
        shared_ptr<PetriNet> _petri_net;
        PetriNetDynamic _current_dynamic;
        double _end_time;
        const int _iteration_num;
        int _iteration_count = 0;

    public:
        PetriNetSimulator(shared_ptr<PetriNet> petri_net, double end_time, int iteration_num)
                : _petri_net(petri_net), _current_dynamic(petri_net->ForkInitDynamic()), _end_time(end_time),
                  _iteration_num(iteration_num)
        { }

        void Rewind()
        {
            _iteration_count++;
            if (_iteration_num > 0 && _iteration_count >= _iteration_num)
            {
                throw EndOfSample();
            }
            _current_dynamic = _petri_net->ForkInitDynamic();
        }

        void RunToEnd();

        void RunToNextEvent();

        const PetriNetDynamic &CurrentDynamic()
        {
            return _current_dynamic;
        }
    };

    class PetriNetSteadyStateSampler
    {
    private:
        PetriNetSimulator _simulator;
    public:
        PetriNetSteadyStateSampler(const PetriNetSimulator &simulator) : _simulator(simulator)
        { }

        const PetriNetDynamic &NextSample();
    };

};


#endif //SPNP_SIMULATING_H
