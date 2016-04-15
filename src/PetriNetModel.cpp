//
// Created by wangnan on 16-1-19.
//
#include <limits>
#include "PetriNetModel.h"

using namespace PetriNetModel;

bool Transition::IsEnabled(const PetriNetDynamic &dynamic) const
{
    for (Arc *arc_ptr : _input_arcs)
    {
        Mark place_mark = dynamic.mark_list[arc_ptr->_place_ptr->_index];
        if (arc_ptr->_multiplicity > place_mark)
        {
            return false;
        }
    }
    for (Arc *arc_ptr : _inhibitor_arcs)
    {
        Mark place_mark = dynamic.mark_list[arc_ptr->_place_ptr->_index];
        if (arc_ptr->_multiplicity < place_mark)
        {
            return false;
        }
    }
    return true;
}

double Transition::SampleFireTime() const
{
    return _sample_func();
}

void Transition::Fire(PetriNetDynamic &dynamic) const
{
    for (Arc *arc_ptr : _input_arcs)
    {
        Mark &place_mark = dynamic.mark_list[arc_ptr->_place_ptr->_index];
        place_mark -= arc_ptr->_multiplicity;
    }
    for (Arc *arc_ptr : _output_arcs)
    {
        Mark &place_mark = dynamic.mark_list[arc_ptr->_place_ptr->_index];
        place_mark += arc_ptr->_multiplicity;
    }
}

void PetriNet::AddPlace(const string &name)
{
    if (HasName(name, _place_name_map))
    {
        throw DuplicateName();
    }
    int place_index = _place_list.size();
    _place_name_map[name] = place_index;
    _place_list.push_back(std::move(unique_ptr<Place>(new Place(name, place_index))));
    _init_dynamic.mark_list.push_back(0);
}

void PetriNet::AddTransition(const string &name, SampleFuncType sample_func)
{
    if (HasName(name, _transition_name_map))
    {
        throw DuplicateName();
    }
    int transition_index = _transition_list.size();
    _transition_name_map[name] = transition_index;
    _transition_list.push_back(std::move(unique_ptr<Transition>(new Transition(name, sample_func))));
}

void PetriNet::AddArc(const string &t_name, const string &p_name, PetriNetModel::ArcType type,
                      Mark multiplicity)
{
    int trans_index = FindIndex(t_name, _transition_name_map);
    int place_index = FindIndex(p_name, _place_name_map);
    Transition *t_ptr = _transition_list[trans_index].get();
    Place *p_ptr = _place_list[place_index].get();
    Arc *arc_ptr = new Arc(t_ptr, p_ptr, type, multiplicity);
    t_ptr->AddArc(arc_ptr, type);
    _arc_list.push_back(unique_ptr<Arc>(arc_ptr));
}

int PetriNet::FindIndex(const string &name, const std::unordered_map<std::string, int> &map) const
{
    try
    {
        return map.at(name);
    } catch (std::exception e)
    {
        throw NameNotFound();
    }

}


std::string PetriNetModel::Arc::ToString() const
{
    ostringstream ss;
    ss << "Arc{ transition:" << _transition_ptr->_name << " place:" << _place_ptr->_name
    << " type:" << _type << " multi:" << _multiplicity << "}";
    return ss.str();
}

std::string Transition::ToString() const
{
    ostringstream ss;
    ss << "Transition{ name:" << _name << "}";
    return ss.str();
}

std::string Place::ToString() const
{
    ostringstream ss;
    ss << "Place{ name:" << _name << "}";
    return ss.str();
}

std::string PetriNet::ToString() const
{
    ostringstream ss;
    ss << "Petri Net{" << std::endl;
    ss << ">>>>>Places<<<<<" << std::endl;
    for (auto &&ptr : _place_list)
    {
        ss << ptr->ToString() << std::endl;
    }
    ss << ">>>>>Transitions<<<<<" << std::endl;
    for (auto &&ptr : _transition_list)
    {
        ss << ptr->ToString() << std::endl;
    }
    ss << ">>>>>Arcs<<<<<" << std::endl;
    for (auto &&ptr : _arc_list)
    {
        ss << ptr->ToString() << std::endl;
    }
    ss << "}";
    return ss.str();
}

void Transition::AddArc(Arc *arc_ptr, ArcType type)
{
    switch (type)
    {
        case ArcType::Input:
            _input_arcs.push_back(arc_ptr);
            break;
        case ArcType::Output:
            _output_arcs.push_back(arc_ptr);
            break;
        case ArcType::Inhibitor:
            _inhibitor_arcs.push_back(arc_ptr);
            break;
    }

}

void PetriNet::PrepareToFire(PetriNetDynamic &dynamic) const
{
    double min_time = std::numeric_limits<double>::max();
    Transition *firing_trans = nullptr;
    for (auto &&trans : _transition_list)
    {
        if (trans->IsEnabled(dynamic))
        {
            double sample = trans->SampleFireTime();
            if (sample < min_time)
            {
                min_time = sample;
                firing_trans = trans.get();
            }

        }
    }
    if (firing_trans != nullptr)
    {
        dynamic.next_time = dynamic.time + min_time;
        dynamic.firing_transition_ptr = firing_trans;
    } else
    {
        throw LackOfEnabledTransition();
    }
}

void PetriNet::Fire(PetriNetDynamic &dynamic) const
{
    if (dynamic.firing_transition_ptr == nullptr)
    {
        return;
    }
    dynamic.firing_transition_ptr->Fire(dynamic);
    dynamic.time = dynamic.next_time;
}

void PetriNet::SetInitMark(const string &p_name, Mark mark)
{
    int index = FindIndex(p_name, _place_name_map);
    _init_dynamic.mark_list[index] = mark;
}

bool PetriNet::HasName(const string &name, const unordered_map<string, int> &map) const
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

PetriNetDynamic PetriNet::ForkInitDynamic() const
{
    return PetriNetDynamic(_init_dynamic);
}


Mark PetriNet::GetPlaceMark(const PetriNetDynamic &dynamic, const string &p_name) const
{
    int index = FindIndex(p_name, _place_name_map);
    return dynamic.mark_list[index];
}

Mark PetriNetDynamic::GetMark(const string &name) const
{
    return petri_net->GetPlaceMark(*this, name);
}
