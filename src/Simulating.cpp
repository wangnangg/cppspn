//
// Created by wangnan on 16-1-19.
//

#include "Simulating.h"

using namespace Simulating;

void PetriNetSimulator::RunToEnd()
{
    if (_current_dynamic.NextTime() >= _end_time)
    {
        Rewind();
    }
    while (true)
    {
        try
        {
            _petri_net->PrepareToFire(_current_dynamic);
        } catch (const LackOfEnabledTransition &)
        {
            break;
        }
        if (_current_dynamic.NextTime() >= _end_time)
        {
            break;
        } else
        {
            _petri_net->Fire(_current_dynamic);
        }
    }
    _current_dynamic.SetNextTime(_end_time);
}

const PetriNetDynamic &PetriNetSteadyStateSampler::NextSample()
{
    _simulator.RunToNextEvent();
    return _simulator.CurrentDynamic();
}

void PetriNetSimulator::RunToNextEvent()
{
    if (_current_dynamic.NextTime() >= _end_time)
    {
        Rewind();
    }
    _petri_net->Fire(_current_dynamic);
    try
    {
        _petri_net->PrepareToFire(_current_dynamic);
    } catch (const LackOfEnabledTransition &)
    {
        _current_dynamic.SetNextTime(_end_time);
    }
}
