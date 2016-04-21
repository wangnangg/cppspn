//
// Created by wangnan on 16-1-19.
//

#ifndef SPNP_PETRI_NET_MODEL_H
#define SPNP_PETRI_NET_MODEL_H

#include<string>
#include<vector>
#include<memory>
#include<unordered_map>
#include<sstream>
#include<functional>
#include "Statistics.h"

namespace PetriNetModel
{
    using std::string;
    using std::vector;
    using std::unique_ptr;
    using std::unordered_map;
    using std::ostringstream;
    using std::function;
    using std::shared_ptr;
    using std::size_t;
    using namespace Statistics;

    typedef int Mark;
    enum ArcType
    {
        Input,
        Output,
        Inhibitor,
    };


    class Arc
    {
        friend class PetriNet;

    private:
        size_t _transition_index;
        size_t _place_index;
        ArcType _type;
        Mark _multiplicity;
    public:
        Arc(size_t t_index, size_t p_index, ArcType type, Mark multiplicity) :
                _transition_index(t_index), _place_index(p_index), _type(type), _multiplicity(multiplicity)
        { }
    };

    typedef std::function<double(double uniform_rand_num)> FiringTimeFuncType;

    class Transition
    {
        friend class PetriNet;

    private:
        string _name;
        vector<size_t> _input_arcs;
        vector<size_t> _output_arcs;
        vector<size_t> _inhibitor_arcs;
        FiringTimeFuncType _sample_func;
    public:
        Transition(const string &name, FiringTimeFuncType sample_func) :
                _name(name), _sample_func(sample_func)
        { }

        void AddArc(size_t arc_index, ArcType type)
        {
            switch (type)
            {
                case ArcType::Input:
                    _input_arcs.push_back(arc_index);
                    break;
                case ArcType::Output:
                    _output_arcs.push_back(arc_index);
                    break;
                case ArcType::Inhibitor:
                    _inhibitor_arcs.push_back(arc_index);
                    break;
            }
        }

        double GetFireTimeSample(double uniform_rand_num) const
        { return _sample_func(uniform_rand_num); }
    };

    class Place
    {
        friend class PetriNet;

    private:
        string _name;
        size_t _index;
    public:
        Place(const string &name, size_t index) : _name(name), _index(index)
        { }
    };


    class DuplicateName : public std::exception
    {
    };

    class NameNotFound : public std::exception
    {
    };


    class PetriNetState
    {
    private:
        vector<Mark> _mark_list;
        double _time = 0.0;
        double _next_firing_time = 0.0;
        size_t _firing_transition_index = 0;

    public:
        double GetTime() const
        { return _time; }

        void SetTime(double t)
        { _time = t; }

        void SetMark(size_t p_index, Mark m)
        { _mark_list[p_index] = m; }

        Mark GetMark(size_t p_index) const
        { return _mark_list[p_index]; }

        void ModifyMark(size_t p_index, Mark value)
        { _mark_list[p_index] += value; }

        void AddMark()
        { _mark_list.push_back(Mark()); }

        double StateDuration() const
        { return _next_firing_time - _time; };

        double NextFiringTime() const
        { return _next_firing_time; }

        void SetNextFiring(double inc_time, size_t t_index)
        {
            _time = _next_firing_time;
            _next_firing_time += inc_time;
            _firing_transition_index = t_index;
        }

        size_t GetFiringTransitionIndex() const
        { return _firing_transition_index; }

    };

    class PetriNetStatic
    {
    private:
        vector<Transition> _transition_list;
        unordered_map<string, size_t> _transition_name_map;
        vector<Place> _place_list;
        unordered_map<string, size_t> _place_name_map;
        vector<Arc> _arc_list;
        PetriNetState _init_state;
    public:
        PetriNetStatic()
        { }

        PetriNetStatic(const PetriNetStatic &petri_net_static) = delete;

        size_t AddPlace(const string &name);

        void SetInitMark(const string &p_name, Mark mark);

        void AddTransition(const string &name, FiringTimeFuncType firing_time_func);

        void AddArc(const string &t_name, const string &p_name, ArcType type, Mark multiplicity);

        size_t GetPlaceIndex(const string &name) const
        { return FindIndex(name, _place_name_map); }

        PetriNetState GetInitState() const
        { return _init_state; }

        const Transition &GetTransition(size_t index) const
        { return _transition_list[index]; }

        double GetFiringTimeSample(size_t index, double uniform_rand_num) const
        { return _transition_list[index].GetFireTimeSample(uniform_rand_num); }

        size_t GetTransitionCount() const
        { return _transition_list.size(); }

        const Arc &GetArc(size_t index) const
        { return _arc_list[index]; }

        const Place &GetPlace(size_t index) const
        { return _place_list[index]; }

    private:
        size_t FindIndex(const string &name, const unordered_map<string, size_t> &map) const;

        bool HasName(const string &name, const unordered_map<string, size_t> &map) const;

    };


    class PetriNet
    {
    private:
        shared_ptr<PetriNetStatic> _petri_net_static;
        PetriNetState _current_state;
    public:
        PetriNet()
        {
            _petri_net_static = std::make_shared<PetriNetStatic>();
        }

        //construction phase
        size_t AddPlace(const string &name)
        { return _petri_net_static->AddPlace(name); }

        void SetInitMark(const string &p_name, Mark mark)
        { _petri_net_static->SetInitMark(p_name, mark); }

        void AddTransition(const string &name, FiringTimeFuncType firing_time_func)
        { _petri_net_static->AddTransition(name, firing_time_func); }

        void AddArc(const string &t_name, const string &p_name, ArcType type, int multiplicity)
        { _petri_net_static->AddArc(t_name, p_name, type, multiplicity); }

        //simulation phase
        void Reset(UniformRandomNumberGenerator &generator)
        {
            _current_state = _petri_net_static->GetInitState();
            FindNextFiringTransition(generator);
        }

        double GetTime() const
        { return _current_state.GetTime(); }

        Mark GetPlaceMark(size_t index) const
        { return _current_state.GetMark(index); }

        Mark GetPlaceMark(const string &name) const
        {
            size_t p_index = GetPlaceIndex(name);
            return GetPlaceMark(p_index);
        }

        size_t GetPlaceIndex(const string &name) const
        { return _petri_net_static->GetPlaceIndex(name); }

        void NextState(UniformRandomNumberGenerator &generator);

        double StateDuration() const
        { return _current_state.StateDuration(); }

        double NextFiringTime() const
        { return _current_state.NextFiringTime(); }

    private:
        bool IsTransitionEnabled(size_t t_index) const;

        void Fire();

        void FindNextFiringTransition(UniformRandomNumberGenerator &generator);
    };

}
#endif //SPNP_PETRI_NET_MODEL_H
