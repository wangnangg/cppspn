//
// Created by wangnan on 16-4-16.
//
#include "PetriNetModel.h"

namespace PetriNetModel
{

    void PetriNetStatic::AddPlace(const string &name)
    {
        if (HasName(name, _place_name_map))
        {
            throw DuplicateName();
        }
        size_t place_index = _place_list.size();
        _place_name_map[name] = place_index;
        _place_list.push_back(Place(name, place_index));
        _init_state.AddMark();
    }

    void PetriNetStatic::AddTransition(const string &name, FiringTimeFuncType firing_time_func)
    {
        if (HasName(name, _transition_name_map))
        {
            throw DuplicateName();
        }
        size_t transition_index = _transition_list.size();
        _transition_name_map[name] = transition_index;
        _transition_list.push_back(Transition(name, firing_time_func));
    }

    void PetriNetStatic::AddArc(const string &t_name, const string &p_name, PetriNetModel::ArcType type,
                                Mark multiplicity)
    {
        size_t trans_index = FindIndex(t_name, _transition_name_map);
        size_t place_index = FindIndex(p_name, _place_name_map);
        size_t arc_index = _arc_list.size();
        _arc_list.push_back(Arc(trans_index, place_index, type, multiplicity));
        Transition &trans = _transition_list[trans_index];
        trans.AddArc(arc_index, type);
    }

    size_t PetriNetStatic::FindIndex(const string &name, const std::unordered_map<std::string, size_t> &map) const
    {
        try
        {
            return map.at(name);
        } catch (std::exception e)
        {
            throw NameNotFound();
        }
    }


    void PetriNetStatic::SetInitMark(const string &p_name, Mark mark)
    {
        size_t p_index = FindIndex(p_name, _place_name_map);
        _init_state.SetMark(p_index, mark);
    }

    bool PetriNetStatic::HasName(const string &name, const unordered_map<string, size_t> &map) const
    {
        try
        {
            map.at(name);
            return true;
        } catch (std::exception e)
        {
            return false;
        }
    }

}
