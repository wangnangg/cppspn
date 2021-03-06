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
        MeanEstimator &_cumulative_estimator;
        MeanEstimator &_transient_estimator;
        double _end_time;
        size_t _source_index;
        thread _worker_thread;
        bool _stop = false;
        bool _running = false;
    public:
        PetriNetSimulator(const PetriNetCreator &creator,
                          MeanEstimator &cumulative_estimator,
                          MeanEstimator &transient_estimator,
                          double end_time,
                          size_t source_index)
                : _petri_net(creator.CreatePetriNet()), _cumulative_estimator(cumulative_estimator),
                  _transient_estimator(transient_estimator), _end_time(end_time), _source_index(source_index)
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
        MeanEstimator _cumulative_estimator;
        MeanEstimator _transient_estimator;
        vector<PetriNetSimulator> _simulator_list;
        vector<DefaultUniformRandomNumberGenerator> _generator_list;
        const PetriNetCreator &_creator;
        double _end_time;
    public:
        PetriNetMultiSimulator(const PetriNetCreator &creator,
                               size_t simulator_count,
                               double end_time) :
                _simulator_count(simulator_count),
                _cumulative_estimator(simulator_count), _transient_estimator(simulator_count),
                _creator(creator), _end_time(end_time)
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

        MeanEstimator &GetCumulativeEstimator()
        { return _cumulative_estimator; }

        MeanEstimator &GetTransientEstimator()
        { return _transient_estimator; }

        void UpdateResult()
        {
            _cumulative_estimator.ClearResult();
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
        double _confidence_coefficient;
    public:

        TargetPrecision(Type type, double precision, double confidence_coefficient = 0.95) :
                _type(type), _precision(precision), _confidence_coefficient(confidence_coefficient)
        { }

        bool IsSatisfied(const SamplingResult &result) const
        {
            bool satisfied = false;
            ConfidenceInterval interval(result, _confidence_coefficient);
            switch (_type)
            {
                case Relative:
                    satisfied = interval.RelativeError() < _precision;
                    break;
                case Absolute:
                    satisfied = interval.Error() < _precision;
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
        TargetPrecision _precision;
    public:
        SimulatorController(PetriNetMultiSimulator &simulator,
                            TargetPrecision precision = TargetPrecision(TargetPrecision::Inf, 0.0))
                : _simulator(simulator), _precision(precision)
        { }

        void Start(uint32_t max_interation_count)
        {
            _simulator.RunAsync(max_interation_count);
        }

        bool WaitFor(double seconds);

        string ResultToString() const;


    private:
        bool IsPrecisionSatisfied() const;


    };


};


#endif //SPNP_SIMULATING_H
