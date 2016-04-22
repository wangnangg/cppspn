//
// Created by wangnan on 16-4-16.
//
#include <limits>
#include <random>
#include "PetriNetModel.h"

namespace PetriNetModel
{
    void PetriNet::NextState(UniformRandomNumberGenerator &generator)
    {
        if (_firing_transition == nullptr)
        {
            return;
        }
        _firing_transition->Fire();
        _time = _next_firing_time;
        FindNextFiringTransition(generator);
    }

    void PetriNet::Reset(UniformRandomNumberGenerator &generator)
    {
        for (auto &place:_place_list)
        {
            place.Reset();
        }
        for (auto &trans:_transition_list)
        {
            trans.Reset();
        }
        _time = 0.0;
        _next_firing_time = 0.0;
        _firing_transition = nullptr;

        FindNextFiringTransition(generator);
    }

    void PetriNet::FindNextFiringTransition(UniformRandomNumberGenerator &generator)
    {
        Transition *firing_transition = nullptr;
        double firing_time = std::numeric_limits<double>::infinity();
        for (auto &transition:_transition_list)
        {
            transition.InputArcChanged(_time, generator.GetVariate());
            double time = transition.GetFireTime();
            if (time < 0) //disabled
            {
                continue;
            }
            if (time < firing_time)
            {
                firing_time = time;
                firing_transition = &transition;
            }
        }
        _next_firing_time = firing_time;
        _firing_transition = firing_transition;
    }

}
