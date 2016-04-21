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

bool IsOnP1(const PetriNet &petri_net, double &value)
{
    value = petri_net.GetPlaceMark(p1_uid);
    return true;
}


TEST(SimulatingTest, SmokeTest)
{
    Statistics::DefaultUniformRandomNumberGenerator generator(123456);
    auto pn = SimplePetriNet();

    RandomVariableEstimator steady_state_estimator(1);
    RandomVariable rand1("P(on p1)", IsOnP1);
    steady_state_estimator.AddRandomVariable(rand1);

    RandomVariableEstimator transient_state_estimator(1);

    PetriNetSimulator simulator(pn, steady_state_estimator, transient_state_estimator, 1000.0, 0);
    simulator.Run(100, generator);
    simulator.SubmitResult();

    ConfidenceInterval interval = steady_state_estimator.GetRandomVariable(0).GetSamplingResult();
    std::cout << interval.ToString() << std::endl;
    ASSERT_NEAR(interval.Median(), 2.0 / 3.0, 1.0e-2);
}

TEST(SimulatingTest, MultithreadSimulatorTest)
{
    Statistics::DefaultUniformRandomNumberGenerator generator;
    auto pn = SimplePetriNet();

    PetriNetMultiSimulator simulator(pn, 4, 1000.0);
    RandomVariable rand1("P(on p1)", IsOnP1);
    simulator.GetSteadyStateEstimator().AddRandomVariable(rand1);

    RandomVariableEstimator steady_state_estimator(1);
    steady_state_estimator.AddRandomVariable(rand1);
    RandomVariableEstimator transient_state_estimator(1);

    PetriNetSimulator single_simulator(pn, steady_state_estimator, transient_state_estimator, 1000.0, 0);

    simulator.Run(10); //for loading pages.
    single_simulator.Run(10, generator);
    uint32_t iteration_count = 10000;
    StartClock();
    single_simulator.Run(iteration_count, generator);
    single_simulator.SubmitResult();
    double single_thread_time = StopClock();

    StartClock();
    simulator.Run(iteration_count);
    double multi_thread_time = StopClock();


    ConfidenceInterval multi_interval = simulator.GetSteadyStateEstimator().GetRandomVariable(0).GetSamplingResult();
    ConfidenceInterval single_interval = steady_state_estimator.GetRandomVariable(0).GetSamplingResult();

    std::cout << "4 threads time: " << multi_thread_time << "s" << std::endl;
    std::cout << "4 threads result: " << multi_interval.ToString() << std::endl;
    std::cout << "single thread time:" << single_thread_time << "s" << std::endl;
    std::cout << "single thread value: " << single_interval.ToString() << std::endl;
    ASSERT_NEAR(multi_interval.Error(), single_interval.Error(), 1.0e-5);
}

TEST(SimulatingTest, MultipleSimulatorTest)
{
    Statistics::DefaultUniformRandomNumberGenerator generator1(123456);
    Statistics::DefaultUniformRandomNumberGenerator generator2(123456);
    auto pn = SimplePetriNet();

    RandomVariableEstimator steady_state_estimator(2);
    RandomVariable rand1("P(on p1)", IsOnP1);
    steady_state_estimator.AddRandomVariable(rand1);

    RandomVariableEstimator steady_state_estimator_standard(1);
    steady_state_estimator_standard.AddRandomVariable(rand1);

    RandomVariableEstimator transient_state_estimator(2);

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

    ConfidenceInterval interval = steady_state_estimator.GetRandomVariable(0).GetSamplingResult();
    ConfidenceInterval interval_standard = steady_state_estimator_standard.GetRandomVariable(0).GetSamplingResult();
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
    simulator.GetSteadyStateEstimator().AddRandomVariable(rand1);

    SimulatorController controller(simulator, 1.0);

    controller.Start(10000);

    while (!controller.Wait())
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
    simulator.GetSteadyStateEstimator().AddRandomVariable(rand1);

    SimulatorController controller(simulator, 1.0, TargetPrecision(TargetPrecision::Absolute, 5e-4));

    controller.Start(1000000);

    while (!controller.Wait())
    {
        string result = controller.ResultToString();
        std::cout << result << std::endl;
    }

    string result = controller.ResultToString();
    std::cout << result << std::endl;


}


TEST(SimulatingTest, Complex)
{
//    static char pF[] = "pF";
//    static char pA[] = "pA";
//    static char pT[] = "pT";
//    static char pU[] = "pU";
//    static char pD[] = "pD";
//
//    static char tR[] = "tR";
//    static char tT[] = "tT";
//    static char tF[] = "tF";
//    static char tG[] = "tG";
//    static char tMu[] = "tMu";
//    static char tLambda[] = "tLambda";
//    static double r = -1;
//    auto pn = std::make_shared<PetriNetGeneric>();
//
//    pn->AddPlace(pF);
//    pn->SetInitMark(pF, 1);
//    pn->AddPlace(pA);
//    pn->AddPlace(pT);
//    pn->AddPlace(pU);
//    pn->SetInitMark(pU, 1);
//    pn->AddPlace(pD);
//
//    pn->AddTransition(tR, Statistics::Exp(1));
//    pn->AddTransition("t2", Statistics::Exp(2));
//
//    pn->AddArc("t1", "p1", ArcType::Input, 1);
//    pn->AddArc("t1", "p2", ArcType::Output, 1);
//    pn->AddArc("t2", "p2", ArcType::Input, 1);
//    pn->AddArc("t2", "p1", ArcType::Output, 1);
//    vector<ExpectationEstimator<PetriNetDynamic, PetriNetSteadyStateSampler>> estimators;
//    PetriNetSimulatorGeneric simulator(pn, 1000.04, 1000);
//    PetriNetSteadyStateSampler sampler(simulator);
//    ExpectationEstimator<PetriNetDynamic, PetriNetSteadyStateSampler> estimator(sampler);
//    estimator.AddRandomVariable(IsOnP1);
//    for(int i=0; i<4; i++)
//    {
//        estimators.push_back(estimator);
//    }
//    for(auto& e: estimators)
//    {
//        e.EstimateAsync();
//    }
//
//    for(auto& e:estimators)
//    {
//        ASSERT_EQ(e.Wait(),true);
//        ConfidenceInterval interval = e.GetResult(0);
//        std::cout << interval.ToString() << std::endl;
//    }
//    SamplingResult summary;
//    for(auto& e:estimators)
//    {
//        summary += e.GetResult(0);
//    }
//    ConfidenceInterval interval_total(summary);
//    std::cout << "summary:" << interval_total.ToString() << std::endl;
};