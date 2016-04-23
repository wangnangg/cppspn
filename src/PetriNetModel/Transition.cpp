//
// Created by wangnan on 16-4-22.
//

#include "PetriNetModel.h"

namespace PetriNetModel
{
    void Transition::InputArcChanged(double current_time, double uniform_rand_num)
    {
        double enabled = IsEnabled();
        switch (_state)
        {
            case State::JustFired:
                if (enabled)
                {
                    _last_sample_value = _sample_func(uniform_rand_num);
                    _firing_time = current_time + _last_sample_value;
                    _state = State::Enable;
                } else
                {
                    _firing_time = -1;
                    _state = State::Disable_NeverEnabledSinceFire;
                }
                break;
            case State::Enable:
                if (enabled)
                {
                    return;
                } else
                {
                    _left_time = _firing_time - current_time;
                    _firing_time = -1;
                    _state = State::Disable_EnabledSinceFire;
                }
                break;
            case State::Disable_EnabledSinceFire:
                if (enabled)
                {
                    switch (_policy)
                    {
                        case ResamplingPolicy::Identical:
                            _firing_time = current_time + _last_sample_value;
                            break;
                        case ResamplingPolicy::Resume:
                            _firing_time = current_time + _left_time;
                            break;
                        case ResamplingPolicy::Different:
                        default:
                            _firing_time = current_time + _sample_func(uniform_rand_num);
                            break;
                    }
                    _state = State::Enable;
                } else
                {
                    return;
                }
            case State::Disable_NeverEnabledSinceFire:
                if (enabled)
                {
                    _last_sample_value = _sample_func(uniform_rand_num);
                    _firing_time = current_time + _last_sample_value;
                    _state = State::Enable;
                } else
                {
                    return;
                }
        }
    }

    void Transition::AddArc(Arc *arc_ptr, Arc::Type type)
    {
        switch (type)
        {
            case Arc::Type::Input:
                _input_arcs.push_back(arc_ptr);
                break;
            case Arc::Type::Output:
                _output_arcs.push_back(arc_ptr);
                break;
            case Arc::Type::Inhibitor:
                _inhibitor_arcs.push_back(arc_ptr);
                break;
        }
    }

    void Transition::Fire()
    {
        for (Arc *arc_ptr:_input_arcs)
        {
            Place *place = arc_ptr->GetPlace();
            place->ModifyMark(-arc_ptr->GetMultiplicity());
        }
        for (Arc *arc_ptr:_output_arcs)
        {
            Place *place = arc_ptr->GetPlace();
            place->ModifyMark(arc_ptr->GetMultiplicity());
        }
        _state = State::JustFired;
    }


    bool Transition::IsEnabled() const
    {
        for (Arc *arc_ptr:_input_arcs)
        {
            Mark place_mark = arc_ptr->GetPlace()->GetMark();
            if (arc_ptr->GetMultiplicity() > place_mark)
            {
                return false;
            }
        }
        for (Arc *arc_ptr:_inhibitor_arcs)
        {
            Mark place_mark = arc_ptr->GetPlace()->GetMark();
            if (arc_ptr->GetMultiplicity() <= place_mark)
            {
                return false;
            }
        }
        return true;
    }
}