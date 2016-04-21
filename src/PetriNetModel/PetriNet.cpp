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
        if (_current_state.StateDuration() == std::numeric_limits<double>::infinity())
        {
            return;
        }
        Fire();
        FindNextFiringTransition(generator);
    }

    bool PetriNet::IsTransitionEnabled(size_t t_index) const
    {
        const Transition &trans = _petri_net_static->GetTransition(t_index);
        for (int arc_index : trans._input_arcs)
        {
            const Arc &arc = _petri_net_static->GetArc(arc_index);
            Mark place_mark = _current_state.GetMark(arc._place_index);
            if (arc._multiplicity > place_mark)
            {
                return false;
            }
        }
        for (int arc_index : trans._inhibitor_arcs)
        {
            const Arc &arc = _petri_net_static->GetArc(arc_index);
            Mark place_mark = _current_state.GetMark(arc._place_index);
            if (arc._multiplicity < place_mark)
            {
                return false;
            }
        }
        return true;
    }

    void PetriNet::Fire()
    {
        size_t t_index = _current_state.GetFiringTransitionIndex();
        const Transition &trans = _petri_net_static->GetTransition(t_index);
        for (size_t arc_index : trans._input_arcs)
        {
            const Arc &arc = _petri_net_static->GetArc(arc_index);
            _current_state.ModifyMark(arc._place_index, -arc._multiplicity);
        }
        for (size_t arc_index : trans._output_arcs)
        {
            const Arc &arc = _petri_net_static->GetArc(arc_index);
            _current_state.ModifyMark(arc._place_index, arc._multiplicity);
        }
    }

    void PetriNet::FindNextFiringTransition(UniformRandomNumberGenerator &generator)
    {
        size_t firing_t_index = 0;
        double firing_time = std::numeric_limits<double>::infinity();
        size_t t_count = _petri_net_static->GetTransitionCount();
        for (size_t t_index = 0; t_index < t_count; t_index++)
        {
            if (IsTransitionEnabled(t_index))
            {
                double time = _petri_net_static->GetFiringTimeSample(t_index, generator.GetVariate());
                if (time < firing_time)
                {
                    firing_time = time;
                    firing_t_index = t_index;
                }
            }
        }
        _current_state.SetNextFiring(firing_time, firing_t_index);
    }

}
