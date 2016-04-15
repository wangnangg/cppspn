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

namespace PetriNetModel
{
    using std::string;
    using std::vector;
    using std::unique_ptr;
    using std::unordered_map;
    using std::ostringstream;
    using std::function;

    class Arc;

    class Transition;

    class Place;

    typedef int Mark;
    enum ArcType
    {
        Input,
        Output,
        Inhibitor,
    };

    class PetriNet;

    class PetriNetDynamic
    {
        friend class PetriNet;

        friend class Transition;

    private:
        vector<Mark> mark_list;
        double time = 0.0;
        double next_time = 0.0;
        Transition *firing_transition_ptr = nullptr; //next transition to fire
        PetriNet *petri_net = nullptr;

        PetriNetDynamic(PetriNet *petri_net) : petri_net(petri_net)
        {
        }

    public:
        double Time() const
        {
            return time;
        }

        double NextTime() const
        {
            return next_time;
        }

        void SetNextTime(double end_time)
        {
            next_time = end_time;
        }

        double Duration() const
        {
            return next_time - time;
        };

        Mark GetMark(const string &name) const;

    };

    class Arc
    {
        friend class Transition;

        friend class Place;

        friend class PetriNet;

    private:
        Transition *_transition_ptr;
        Place *_place_ptr;
        ArcType _type;
        Mark _multiplicity;
    public:
        Arc(Transition *t, Place *p, ArcType type, unsigned int multiplicity) :
                _transition_ptr(t), _place_ptr(p), _type(type), _multiplicity(multiplicity)
        { }

        string ToString() const;

    };

    typedef std::function<double()> SampleFuncType;

    class Transition
    {
        friend class Arc;

        friend class Place;

        friend class PetriNet;

    private:
        string _name;
        vector<Arc *> _input_arcs;
        vector<Arc *> _output_arcs;
        vector<Arc *> _inhibitor_arcs;
        SampleFuncType _sample_func;
    public:
        Transition(const string &name, SampleFuncType sample_func) :
                _name(name), _sample_func(sample_func)
        { }

        bool IsEnabled(const PetriNetDynamic &dynamic) const;

        double SampleFireTime() const;

        void Fire(PetriNetDynamic &dynamic) const;

        void AddArc(Arc *arc_ptr, ArcType type);

        string ToString() const;
    };

    class Place
    {
        friend class Transition;

        friend class Arc;

        friend class PetriNet;

    private:
        string _name;
        int _index;
    public:
        Place(const string &name, int index) : _name(name), _index(index)
        { }

        string ToString() const;
    };

    class LackOfEnabledTransition : public std::exception
    {
    };

    class DuplicateName : public std::exception
    {
    };

    class NameNotFound : public std::exception
    {
    };


    class PetriNet
    {
    private:
        vector<unique_ptr<Transition>> _transition_list;
        unordered_map<string, int> _transition_name_map;
        vector<unique_ptr<Place>> _place_list;
        unordered_map<string, int> _place_name_map;
        vector<unique_ptr<Arc>> _arc_list;
        PetriNetDynamic _init_dynamic;
    public:
        PetriNet() : _init_dynamic(this)
        { }

        PetriNet(const PetriNet &net) = delete;

        void AddPlace(const string &name);

        void SetInitMark(const string &p_name, Mark mark);

        void AddTransition(const string &name, SampleFuncType sample_func);

        void AddArc(const string &t_name, const string &p_name, ArcType type, int multiplicity);

        void Fire(PetriNetDynamic &dynamic) const;

        void PrepareToFire(PetriNetDynamic &dynamic) const;

        PetriNetDynamic ForkInitDynamic() const;

        Mark GetPlaceMark(const PetriNetDynamic &dynamic, const string &p_name) const;

        string ToString() const;

    private:
        int FindIndex(const string &name, const unordered_map<string, int> &map) const;

        bool HasName(const string &name, const unordered_map<string, int> &map) const;

    };
}
#endif //SPNP_PETRI_NET_MODEL_H
