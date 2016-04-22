//

#include "Simulating.h"
#include <iostream>

namespace Simulating
{
    void PetriNetSimulator::Run(int iteration_num, UniformRandomNumberGenerator &generator)
    {
        for (int i = 0; i < iteration_num; i++)
        {
            if (_stop)
            {
                break;
            }
            _petri_net.Reset(generator);

            std::unique_lock<std::mutex> steady_unique_lock(_steady_state_estimator.GetSourceMutex(_source_index));
            while (_petri_net.GetNextFiringTime() < _end_time)
            {
                _steady_state_estimator.InputSample(_source_index, _petri_net, _petri_net.GetDuration());
                _petri_net.NextState(generator);
            }
            _steady_state_estimator.InputSample(_source_index, _petri_net, _end_time - _petri_net.GetTime());
            steady_unique_lock.unlock();
            std::unique_lock<std::mutex> transient_unique_lock(_transient_estimator.GetSourceMutex(_source_index));
            _transient_estimator.InputSample(_source_index, _petri_net, 1.0);
            transient_unique_lock.unlock();
        }
        _running = false;
    }

    void PetriNetSimulator::SubmitResult()
    {
        std::unique_lock<std::mutex> steady_unique_lock(_steady_state_estimator.GetSourceMutex(_source_index));
        _steady_state_estimator.SubmitResult(_source_index);
        steady_unique_lock.unlock();
        std::unique_lock<std::mutex> transient_unique_lock(_transient_estimator.GetSourceMutex(_source_index));
        _transient_estimator.SubmitResult(_source_index);
        transient_unique_lock.unlock();
    }

    void PetriNetMultiSimulator::RunAsync(uint32_t interation_count)
    {
        _simulator_list.clear();
        _generator_list.clear();
        uint32_t iteration_per_worker = interation_count / (uint32_t) _simulator_count;
        for (uint32_t i = 0; i < _simulator_count; i++)
        {
            _simulator_list.push_back(
                    PetriNetSimulator(_creator, _steady_state_estimator, _transient_estimator, _end_time, i));
            _generator_list.push_back(DefaultUniformRandomNumberGenerator());
        }
        for (uint32_t i = 0; i < _simulator_count; i++)
        {
            _simulator_list[i].RunAsync(iteration_per_worker, _generator_list[i]);
        }
    }

    bool SimulatorController::IsPrecisionSatisfied() const
    {
        for (const auto &rand_var:_simulator.GetSteadyStateEstimator())
        {
            if (!_precision.IsSatisfied(rand_var.GetSamplingResult()))
            {
                return false;
            }
        }
        for (const auto &rand_var:_simulator.GetTransientEstimator())
        {
            if (!_precision.IsSatisfied(rand_var.GetSamplingResult()))
            {
                return false;
            }
        }
        return true;
    }

    string SimulatorController::ResultToString() const
    {
        ostringstream ss;
        ss << "steady states variable:" << std::endl;
        for (const auto &rand_var: _simulator.GetSteadyStateEstimator())
        {
            ConfidenceInterval interval = rand_var.GetSamplingResult();
            ss << "\t" << rand_var.GetName() << ": " << interval.ToString() << std::endl;
        }
        ss << "transient states variable:" << std::endl;
        for (const auto &rand_var: _simulator.GetTransientEstimator())
        {
            ConfidenceInterval interval = rand_var.GetSamplingResult();
            ss << "\t" << rand_var.GetName() << ": " << interval.ToString() << std::endl;
        }
        return ss.str();
    }

    bool SimulatorController::WaitFor(double seconds)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds((uint64_t) (seconds * 1000)));
        _simulator.UpdateResult();
        if (IsPrecisionSatisfied())
        {
            _simulator.Stop();
            _simulator.Wait();
            return true;
        }
        if (_simulator.IsRunning())
        {
            return false;
        } else
        {
            _simulator.Wait();
            return true;
        }
    }
}



