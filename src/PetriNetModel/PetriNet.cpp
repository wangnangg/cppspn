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
        for (Transition *trans_ptr:_firing_transition->GetAffectedTransition())
        {
            trans_ptr->InputArcChanged(_time, generator.GetVariate());
        }
        FindNextFiringTransition();
    }

    void PetriNet::Reset(UniformRandomNumberGenerator &generator)
    {
        _time = 0.0;
        _next_firing_time = 0.0;
        _firing_transition = nullptr;
        for (auto &place:_place_list)
        {
            place.Reset();
        }
        for (auto &trans:_transition_list)
        {
            trans.Reset();
            trans.InputArcChanged(_time, generator.GetVariate());
        }
        FindNextFiringTransition();
    }

    void PetriNet::FindNextFiringTransition()
    {
        Transition *firing_transition = nullptr;
        double firing_time = std::numeric_limits<double>::infinity();
        for (auto &transition:_transition_list)
        {
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
