//
// Created by wangnan on 16-4-22.
//

#include "PetriNetModel.h"

namespace PetriNetModel
{
    size_t PetriNetCreator::AddPlace(const string &name, Mark mark)
    {
        if (_committed)
        {
            throw ModificationAfterCommit();
        }
        if (HasName(name, _place_name_map))
        {
            throw DuplicateName();
        }
        size_t p_index = _place_cmd.size();
        _place_name_map[name] = p_index;
        _place_cmd.push_back(CreatePlaceCmd{name, mark});
        return p_index;
    }

    void PetriNetCreator::AddTransition(const string &name, Transition::FiringTimeFuncType firing_time_func,
                                        Transition::ResamplingPolicy resampling_policy)
    {
        if (_committed)
        {
            throw ModificationAfterCommit();
        }
        if (HasName(name, _transition_name_map))
        {
            throw DuplicateName();
        }
        _transition_name_map[name] = _transition_cmd.size();
        _transition_cmd.push_back(CreateTransitionCmd{name, firing_time_func, resampling_policy});
    }

    void PetriNetCreator::Commit()
    {
        _committed = true;
    }

    void PetriNetCreator::AddArc(const string &transition_name, const string &place_name, Arc::Type type,
                                 Mark multiplicity)
    {
        if (_committed)
        {
            throw ModificationAfterCommit();
        }
        _arc_cmd.push_back(CreateArcCmd{
                FindIndex(transition_name, _transition_name_map),
                FindIndex(place_name, _place_name_map),
                type, multiplicity});
    }

    void PetriNetCreator::AddPlaceAffectedTransition(std::set<Transition *> &trans_set, Place *place_ptr) const
    {
        for (Arc *arc_ptr: place_ptr->_input_arcs)
        {
            trans_set.insert(arc_ptr->_transition);
        }
        for (Arc *arc_ptr: place_ptr->_inhibitor_arcs)
        {
            trans_set.insert(arc_ptr->_transition);
        }
    }


    PetriNet PetriNetCreator::CreatePetriNet() const
    {
        if (!_committed)
        {
            throw CreatePetriNetBeforeCommit();
        }
        PetriNet petri_net(*this, _place_cmd.size(), _transition_cmd.size(), _arc_cmd.size());
        for (size_t p_index = 0; p_index < _place_cmd.size(); p_index++)
        {
            auto &place = petri_net._place_list[p_index];
            const auto &cmd = _place_cmd[p_index];
            place.Init(&cmd.name, cmd.init_mark);
        }
        for (size_t arc_index = 0; arc_index < _arc_cmd.size(); arc_index++)
        {
            auto &arc = petri_net._arc_list[arc_index];
            auto &cmd = _arc_cmd[arc_index];
            Place *place_ptr = &petri_net._place_list[cmd.place_index];
            Transition *trans_ptr = &petri_net._transition_list[cmd.transition_index];
            arc.Init(trans_ptr, place_ptr, cmd.type, cmd.multiplicity);
            trans_ptr->AddArc(&arc, cmd.type);
            place_ptr->AddArc(&arc, cmd.type);
        }
        for (size_t t_index = 0; t_index < _transition_cmd.size(); t_index++)
        {
            auto &transition = petri_net._transition_list[t_index];
            const auto &cmd = _transition_cmd[t_index];
            std::set<Transition *> affected_trans_set;
            for (Arc *arc_ptr:transition._input_arcs)
            {
                AddPlaceAffectedTransition(affected_trans_set, arc_ptr->_place);
            }
            for (Arc *arc_ptr:transition._output_arcs)
            {
                AddPlaceAffectedTransition(affected_trans_set, arc_ptr->_place);
            }
            transition.Init(&cmd.name,
                            cmd.firing_time_func,
                            cmd.resampling_policy,
                            vector<Transition *>(affected_trans_set.begin(), affected_trans_set.end())
            );
        }
        return petri_net;
    }


    bool PetriNetCreator::HasName(const string &name, const unordered_map<string, size_t> &map) const
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

    size_t PetriNetCreator::FindIndex(const string &name, const std::unordered_map<std::string, size_t> &map) const
    {
        try
        {
            return map.at(name);
        } catch (std::exception e)
        {
            throw NameNotFound();
        }
    }

}
