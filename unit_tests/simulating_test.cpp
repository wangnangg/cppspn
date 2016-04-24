//
// Created by wangnan on 16-1-25.
//
#include <gtest/gtest.h>
#include <Simulating.h>
#include <PetriNetModel/PetriNetModel.h>
#include <iostream>
#include "helper.h"

using namespace PetriNetModel;
using namespace Simulating;
using namespace Estimating;



TEST(SimulatingTest, SmokeTest)
{
    Statistics::DefaultUniformRandomNumberGenerator generator(123456);
    auto pn = SimplePetriNet();

    MeanEstimator steady_state_estimator(1);
    RandomVariable rand1("P(on p1)", IsOnP1);
    steady_state_estimator.AddRandomVariable(rand1);

    MeanEstimator transient_state_estimator(1);

    PetriNetSimulator simulator(pn, steady_state_estimator, transient_state_estimator, 1000.0, 0);
    simulator.Run(100, generator);
    simulator.SubmitResult();

    ConfidenceInterval interval = steady_state_estimator.GetRandomVariableList()[0].GetSamplingResult();
    std::cout << interval.ToString() << std::endl;
    ASSERT_NEAR(interval.Median(), 2.0 / 3.0, 1.0e-2);
}

TEST(SimulatingTest, MultithreadSimulatorTest)
{
    Statistics::DefaultUniformRandomNumberGenerator generator;
    auto pn = SimplePetriNet();

    PetriNetMultiSimulator simulator(pn, 4, 1000.0);
    RandomVariable rand1("P(on p1)", IsOnP1);
    simulator.GetCumulativeEstimator().AddRandomVariable(rand1);

    MeanEstimator steady_state_estimator(1);
    steady_state_estimator.AddRandomVariable(rand1);
    MeanEstimator transient_state_estimator(1);

    PetriNetSimulator single_simulator(pn, steady_state_estimator, transient_state_estimator, 1000.0, 0);

    simulator.Run(10); //for loading pages.
    single_simulator.Run(10, generator);
    uint32_t iteration_count = 1000;
    StartClock();
    single_simulator.Run(iteration_count, generator);
    single_simulator.SubmitResult();
    double single_thread_time = StopClock();

    StartClock();
    simulator.Run(iteration_count);
    double multi_thread_time = StopClock();


    ConfidenceInterval multi_interval = simulator.GetCumulativeEstimator().GetRandomVariableList()[0].GetSamplingResult();
    ConfidenceInterval single_interval = steady_state_estimator.GetRandomVariableList()[0].GetSamplingResult();

    std::cout << "4 threads time: " << multi_thread_time << "s" << std::endl;
    std::cout << "4 threads result: " << multi_interval.ToString() << std::endl;
    std::cout << "single thread time:" << single_thread_time << "s" << std::endl;
    std::cout << "single thread value: " << single_interval.ToString() << std::endl;
    ASSERT_NEAR(multi_interval.Error(), single_interval.Error(), 1.0e-4);
}

TEST(SimulatingTest, MultipleSimulatorTest)
{
    Statistics::DefaultUniformRandomNumberGenerator generator1(123456);
    Statistics::DefaultUniformRandomNumberGenerator generator2(123456);
    auto pn = SimplePetriNet();

    MeanEstimator steady_state_estimator(2);
    RandomVariable rand1("P(on p1)", IsOnP1);
    steady_state_estimator.AddRandomVariable(rand1);

    MeanEstimator steady_state_estimator_standard(1);
    steady_state_estimator_standard.AddRandomVariable(rand1);

    MeanEstimator transient_state_estimator(2);

    PetriNetSimulator simulator_standard(pn, steady_state_estimator_standard, transient_state_estimator, 1000.0, 0);
    PetriNetSimulator simulator1(pn, steady_state_estimator, transient_state_estimator, 1000.0, 0);
    PetriNetSimulator simulator2(pn, steady_state_estimator, transient_state_estimator, 1000.0, 1);
    steady_state_estimator.ClearResult();
    steady_state_estimator_standard.ClearResult();
    simulator1.Run(500, generator1);
    simulator1.SubmitResult();
    simulator2.Run(500, generator1);
    simulator2.SubmitResult();
    simulator_standard.Run(1000, generator2);
    simulator_standard.SubmitResult();

    ConfidenceInterval interval = steady_state_estimator.GetRandomVariableList()[0].GetSamplingResult();
    ConfidenceInterval interval_standard = steady_state_estimator_standard.GetRandomVariableList()[0].GetSamplingResult();
    std::cout << interval.ToString() << std::endl;
    std::cout << interval_standard.ToString() << std::endl;
    ASSERT_NEAR(interval.LowerBound(), interval_standard.LowerBound(), 1e-10);
    ASSERT_NEAR(interval.UpperBound(), interval_standard.UpperBound(), 1e-10);
}


TEST(SimulatingTest, SimulatorControllerTest)
{
    auto pn = SimplePetriNet();

    PetriNetMultiSimulator simulator(pn, 4, 1000.0);
    RandomVariable rand1("P(on p1)", IsOnP1);
    simulator.GetCumulativeEstimator().AddRandomVariable(rand1);

    SimulatorController controller(simulator);

    controller.Start(10000);

    while (!controller.WaitFor(1.0))
    {
        string result = controller.ResultToString();
        std::cout << result << std::endl;
    }
    string result = controller.ResultToString();
    std::cout << result << std::endl;

}

TEST(SimulatingTest, PrecisionControllTest)
{
    auto pn = SimplePetriNet();

    PetriNetMultiSimulator simulator(pn, 4, 1000.0);
    RandomVariable rand1("P(on p1)", IsOnP1);
    simulator.GetCumulativeEstimator().AddRandomVariable(rand1);

    SimulatorController controller(simulator, TargetPrecision(TargetPrecision::Relative, 5e-4, 0.99));

    controller.Start(1000000);

    while (!controller.WaitFor(1.0))
    {
        string result = controller.ResultToString();
        std::cout << result << std::endl;
    }

    string result = controller.ResultToString();
    std::cout << result << std::endl;


}


TEST(SimulatingTest, Complex)
{
    auto pn = ComplexPetriNet();

    PetriNetMultiSimulator simulator(pn, 4, 1e7);
    RandomVariable rand_user_unavail("UserUnavail", UserUnavail);
    RandomVariable rand_is_user_active("P(User Active)", IsUserActive);
    simulator.GetCumulativeEstimator().AddRandomVariable(rand_user_unavail);
    simulator.GetCumulativeEstimator().AddRandomVariable(rand_is_user_active);
    simulator.GetTransientEstimator().AddRandomVariable(rand_user_unavail);
    simulator.GetTransientEstimator().AddRandomVariable(rand_is_user_active);

    SimulatorController controller(simulator);

    controller.Start(1000);

    while (!controller.WaitFor(1.0))
    {
        string result = controller.ResultToString();
        std::cout << result << std::endl;
    }
    string result = controller.ResultToString();
    std::cout << result << std::endl;

};