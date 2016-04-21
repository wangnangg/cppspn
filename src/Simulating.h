//
// Created by wangnan on 16-1-19.
//

#ifndef SPNP_SIMULATING_H
#define SPNP_SIMULATING_H

#include "PetriNetModel/PetriNetModel.h"
#include "Estimating.h"
#include <thread>

namespace Simulating
{
    using namespace PetriNetModel;
    using namespace Estimating;
    using std::thread;

    class PetriNetSimulator
    {
    private:
        PetriNet _petri_net;
        RandomVariableEstimator &_steady_state_estimator;
        RandomVariableEstimator &_transient_estimator;
        double _end_time;
        size_t _source_index;
        thread _worker_thread;
        bool _stop = false;
        bool _running = false;
    public:
        PetriNetSimulator(const PetriNet &petri_net,
                          RandomVariableEstimator &steady_state_estimator,
                          RandomVariableEstimator &transient_estimator,
                          double end_time,
                          size_t source_index)
                : _petri_net(petri_net), _steady_state_estimator(steady_state_estimator),
                  _transient_estimator(transient_estimator), _end_time(end_time), _source_index(source_index)
        { }

        PetriNetSimulator(const PetriNetSimulator &simulator, size_t source_index) :
                _petri_net(simulator._petri_net),
                _steady_state_estimator(simulator._steady_state_estimator),
                _transient_estimator(simulator._transient_estimator),
                _end_time(simulator._end_time),
                _source_index(source_index)
        { }

        void Run(int iteration_num, UniformRandomNumberGenerator &generator);

        // we require that _end_time < infinity
        void RunAsync(int iteration_num, UniformRandomNumberGenerator &generator)
        {
            _stop = false;
            _running = true;
            _worker_thread = thread(worker, this, iteration_num, std::ref(generator));
        }

        static void worker(PetriNetSimulator *simulator, int iteration_num, UniformRandomNumberGenerator &generator)
        {
            simulator->Run(iteration_num, generator);
        }

        void SubmitResult();

        void Stop()
        { _stop = true; }

        void Wait()
        { _worker_thread.join(); }

        bool IsRunning()
        { return _running; }


    };

    class PetriNetMultiSimulator
    {
        size_t _simulator_count;
        RandomVariableEstimator _steady_state_estimator;
        RandomVariableEstimator _transient_estimator;
        PetriNetSimulator _template_simulator;
        vector<PetriNetSimulator> _simulator_list;
        vector<DefaultUniformRandomNumberGenerator> _generator_list;
    public:
        PetriNetMultiSimulator(const PetriNet &petri_net,
                               size_t simulator_count,
                               double end_time) :
                _simulator_count(simulator_count),
                _steady_state_estimator(simulator_count), _transient_estimator(simulator_count),
                _template_simulator(petri_net, _steady_state_estimator, _transient_estimator, end_time, 0)
        { }

        PetriNetMultiSimulator(const PetriNetMultiSimulator &) = delete;

        void Run(uint32_t interation_count)
        {
            RunAsync(interation_count);
            Wait();
        }

        void RunAsync(uint32_t interation_count);

        void Stop()
        {
            for (size_t i = 0; i < _simulator_count; i++)
            {
                _simulator_list[i].Stop();
            }
        }

        void Wait()
        {
            for (size_t i = 0; i < _simulator_count; i++)
            {
                _simulator_list[i].Wait();
            }
            UpdateResult();
        }

        bool IsRunning()
        {
            for (size_t i = 0; i < _simulator_count; i++)
            {
                if (_simulator_list[i].IsRunning())
                {
                    return true;
                }
            }
            return false;
        }

        RandomVariableEstimator &GetSteadyStateEstimator()
        { return _steady_state_estimator; }

        RandomVariableEstimator &GetTransientEstimator()
        { return _transient_estimator; }

        void UpdateResult()
        {
            _steady_state_estimator.ClearResult();
            _transient_estimator.ClearResult();
            for (auto &simulator: _simulator_list)
            {
                simulator.SubmitResult();
            }
        }


    };

    class TargetPrecision
    {
    public:
        enum Type
        {
            Relative,
            Absolute,
            Inf,
        };
    private:
        Type _type;
        double _precision;
    public:

        TargetPrecision(Type type, double precision) : _type(type), _precision(precision)
        { }

        bool IsSatisfied(const SamplingResult &result) const
        {
            bool satisfied = false;
            ConfidenceInterval interval = result;
            switch (_type)
            {
                case Relative:
                    if (interval.RelativeError() < _precision)
                    {
                        satisfied = true;
                    } else
                    {
                        satisfied = false;
                    }
                    break;
                case Absolute:
                    if (interval.Error() < _precision)
                    {
                        satisfied = true;
                    } else
                    {
                        satisfied = false;
                    }
                    break;
                case Inf:
                    satisfied = false;
                    break;
            }
            return satisfied;
        }
    };

    //TODO: Add signal catch
    class SimulatorController
    {
        PetriNetMultiSimulator &_simulator;
        double _poll_time;
        TargetPrecision _precision;
    public:
        SimulatorController(PetriNetMultiSimulator &simulator,
                            double poll_time,
                            TargetPrecision precision = TargetPrecision(TargetPrecision::Inf, 0.0))
                : _simulator(simulator), _poll_time(poll_time), _precision(precision)
        { }

        void Start(uint32_t max_interation_count)
        {
            _simulator.RunAsync(max_interation_count);
        }

        bool Wait();

        string ResultToString() const;


    private:
        bool IsPrecisionSatisfied() const;


    };


};


#endif //SPNP_SIMULATING_H
