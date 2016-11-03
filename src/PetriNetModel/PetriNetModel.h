//
// Created by wangnan on 16-1-19.
//

#ifndef SPNP_PETRI_NET_MODEL_H
#define SPNP_PETRI_NET_MODEL_H

#include<string>
#include<vector>
#include<memory>
#include <set>
#include<unordered_map>
#include<sstream>
#include<functional>
#include "Statistics.h"

//TODO: fluid support
//TODO: imm transition
//TODO: marking dependent properties of arc: multiplicity
//TODO: marking dependent properties of transition: guard, firing time, weight for imm
//TODO: sensitivity analysis

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


    class Arc;

    class Transition;

    class Place;

    typedef int Mark;


    class Arc
    {
        friend class PetriNetCreator;

    public:
        enum Type
        {
            Input,
            Output,
            Inhibitor,
        };
    private:
        Transition *_transition;
        Place *_place;
        Type _type;
        Mark _multiplicity;
    public:
        Arc()
        { };

        Place *GetPlace() const
        { return _place; }

        Mark GetMultiplicity() const
        { return _multiplicity; }

        void Init(Transition *trans, Place *place, Type type, Mark multiplicity)
        {
            _transition = trans;
            _place = place;
            _type = type;
            _multiplicity = multiplicity;
        }
    };


    class Place
    {
        friend class PetriNetCreator;

    private:
        const string *_name;
        Mark _mark;
        Mark _init_mark;
        vector<Arc *> _input_arcs;
        vector<Arc *> _output_arcs;
        vector<Arc *> _inhibitor_arcs;
    public:
        void Init(const string *name, Mark mark)
        {
            _name = name;
            _init_mark = mark;
        }

        void AddArc(Arc *arc_ptr, Arc::Type type)
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

        void Reset()
        {
            _mark = _init_mark;
        }

        void ModifyMark(Mark diff)
        { _mark += diff; }

        Mark GetMark() const
        { return _mark; }

    };

    class Transition
    {
        friend class PetriNetCreator;

    public:
        enum ResamplingPolicy
        {
            Different,
            Identical,
            Resume
        };
        typedef std::function<double(double uniform_rand_num)> FiringTimeFuncType;
    private:
        enum State
        {
            JustFired,
            Enable,
            Disable_EnabledSinceFire,
            Disable_NeverEnabledSinceFire,
        };
    private:
        const string *_name;
        vector<Arc *> _input_arcs;
        vector<Arc *> _output_arcs;
        vector<Arc *> _inhibitor_arcs;
        FiringTimeFuncType _sample_func;
        State _state = State::JustFired;
        double _firing_time = -1;
        double _last_sample_value = -1;
        double _left_time = -1;
        ResamplingPolicy _policy;
        vector<Transition *> _affected_transition;
    private:
        bool IsEnabled() const;

    public:
        Transition()
        { }

        void Reset()
        {
            _state = State::JustFired;
            _firing_time = -1;
        }


        void Init(const string *name, FiringTimeFuncType sample_func, ResamplingPolicy policy,
                  vector<Transition *> &&affected_trans)
        {
            _name = name;
            _sample_func = sample_func;
            _policy = policy;
            _affected_transition = affected_trans;
        }

        void AddArc(Arc *arc_ptr, Arc::Type type);

        double GetFireTime() const
        { return _firing_time; }

        void InputArcChanged(double current_time, double uniform_rand_num);

        vector<Transition *> &GetAffectedTransition()
        { return _affected_transition; }

        void Fire();
    };


    struct CreatePlaceCmd
    {
        string name;
        Mark init_mark;
    };
    struct CreateTransitionCmd
    {
        string name;
        Transition::FiringTimeFuncType firing_time_func;
        Transition::ResamplingPolicy resampling_policy;
    };
    struct CreateArcCmd
    {
        size_t transition_index;
        size_t place_index;
        Arc::Type type;
        Mark multiplicity;
    };

    class ModificationAfterCommit : public std::exception
    {
    };

    class CreatePetriNetBeforeCommit : public std::exception
    {
    };

    class DuplicateName : public std::exception
    {
    };

    class NameNotFound : public std::exception
    {
    };

    class PetriNet;

    class PetriNetCreator
    {
    private:
        vector<CreatePlaceCmd> _place_cmd;
        vector<CreateTransitionCmd> _transition_cmd;
        vector<CreateArcCmd> _arc_cmd;
        bool _committed = false;
        unordered_map<string, size_t> _transition_name_map;
        unordered_map<string, size_t> _place_name_map;
    private:
        bool HasName(const string &name, const unordered_map<string, size_t> &map) const;

        size_t FindIndex(const string &name, const std::unordered_map<std::string, size_t> &map) const;

        void AddPlaceAffectedTransition(std::set<Transition *> &trans_set, Place *place_ptr) const;


    public:
        PetriNetCreator() = default;

        PetriNetCreator(PetriNetCreator &&) = default;

        size_t AddPlace(const string &name, Mark mark);

        void AddTransition(const string &name, Transition::FiringTimeFuncType firing_time_func,
                           Transition::ResamplingPolicy resampling_policy = Transition::ResamplingPolicy::Different);

        void AddArc(const string &transition_name, const string &place_name, Arc::Type type, Mark multiplicity = 1);

        void Commit();

        size_t GetPlaceIndex(const string &name) const
        { return FindIndex(name, _place_name_map); }

        PetriNet CreatePetriNet() const;

    };

    class PetriNet
    {
        friend class PetriNetCreator;

    private:
        const PetriNetCreator &_creator;
        vector<Place> _place_list;
        vector<Transition> _transition_list;
        vector<Arc> _arc_list;

        double _time = 0.0;
        double _next_firing_time = 0.0;
        Transition *_firing_transition = nullptr;

        PetriNet(const PetriNetCreator &creator, size_t place_count, size_t transition_count, size_t arc_count) :
                _creator(creator), _place_list(place_count), _transition_list(transition_count), _arc_list(arc_count)
        { }

    public:

        PetriNet(const PetriNet &) = delete;

        PetriNet(PetriNet &&other) = default;

        void Reset(UniformRandomNumberGenerator &generator);

        void NextState(UniformRandomNumberGenerator &generator);

        double GetTime() const
        { return _time; }

        double GetDuration() const
        { return _next_firing_time - _time; }

        double GetNextFiringTime() const
        { return _next_firing_time; }

        Mark GetPlaceMark(size_t p_index) const
        { return _place_list[p_index].GetMark(); }

        Mark GetPlaceMark(const string &p_name) const
        {
            size_t p_index = _creator.GetPlaceIndex(p_name);
            return GetPlaceMark(p_index);
        }

    private:
        void FindNextFiringTransition();
    };
}
#endif //SPNP_PETRI_NET_MODEL_H
