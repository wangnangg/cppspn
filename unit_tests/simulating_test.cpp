//
// Created by wangnan on 16-1-25.
//
#include <gtest/gtest.h>
#include <Simulating.h>
#include <PetriNetModel/PetriNetModel.h>
#include <iostream>

using namespace PetriNetModel;
using namespace Simulating;
using namespace Estimating;

bool IsOnP1(const PetriNet &petri_net, double &value)
{
    value = petri_net.GetPlaceMark("p1");
    return true;
}

TEST(SimulatingTest, SmokeTest)
{
    Statistics::DefaultUniformRandomNumberGenerator generator;
    auto pn = PetriNet();
    pn.AddPlace("p1");
    pn.AddPlace("p2");
    double lambda_t1 = 1.0;
    double lambda_t2 = 2.0;
    pn.AddTransition("t1", Statistics::Exp(lambda_t1));
    pn.AddTransition("t2", Statistics::Exp(lambda_t2));
    pn.AddArc("t1", "p1", ArcType::Input, 1);
    pn.AddArc("t1", "p2", ArcType::Output, 1);
    pn.AddArc("t2", "p2", ArcType::Input, 1);
    pn.AddArc("t2", "p1", ArcType::Output, 1);
    pn.SetInitMark("p1", 1);

    RandomVariableEstimator steady_state_estimator(1);
    RandomVariable rand1("P(on p1)", IsOnP1);
    steady_state_estimator.AddRandomVariable(rand1);


    RandomVariableEstimator transient_state_estimator(1);

    PetriNetSimulator simulator(pn, steady_state_estimator, transient_state_estimator, 1000.0, 0);
    simulator.Run(1000, generator);

    steady_state_estimator.UpdateResult();
    ConfidenceInterval interval = steady_state_estimator.GetResult(0);
    std::cout << interval.ToString() << std::endl;
    ASSERT_NEAR(interval.Median(), lambda_t2 / (lambda_t1 + lambda_t2), 1.0e-3);
}

TEST(SimulatingTest, MultipleSimulatorTest)
{
    Statistics::DefaultUniformRandomNumberGenerator generator;
    auto pn = PetriNet();
    pn.AddPlace("p1");
    pn.AddPlace("p2");
    double lambda_t1 = 1.0;
    double lambda_t2 = 2.0;
    pn.AddTransition("t1", Statistics::Exp(lambda_t1));
    pn.AddTransition("t2", Statistics::Exp(lambda_t2));
    pn.AddArc("t1", "p1", ArcType::Input, 1);
    pn.AddArc("t1", "p2", ArcType::Output, 1);
    pn.AddArc("t2", "p2", ArcType::Input, 1);
    pn.AddArc("t2", "p1", ArcType::Output, 1);
    pn.SetInitMark("p1", 1);

    RandomVariableEstimator steady_state_estimator(1);
    RandomVariable rand1("P(on p1)", IsOnP1);
    steady_state_estimator.AddRandomVariable(rand1);

    RandomVariableEstimator steady_state_estimator0(1);
    steady_state_estimator0.AddRandomVariable(rand1);

    RandomVariableEstimator transient_state_estimator(1);

    PetriNetSimulator simulator0(pn, steady_state_estimator0, transient_state_estimator, 1000.0, 0);
    PetriNetSimulator simulator1(pn, steady_state_estimator, transient_state_estimator, 1000.0, 0);
    PetriNetSimulator simulator2(pn, steady_state_estimator, transient_state_estimator, 1000.0, 0);
    simulator1.Run(500, generator);
    simulator2.Run(500, generator);
    simulator0.Run(1000, generator);

    steady_state_estimator.UpdateResult();
    steady_state_estimator0.UpdateResult();
    ConfidenceInterval interval = steady_state_estimator.GetResult(0);
    ConfidenceInterval interval0 = steady_state_estimator0.GetResult(0);
    std::cout << interval.ToString() << std::endl;
    std::cout << interval0.ToString() << std::endl;
    ASSERT_NEAR(interval.Error(), interval0.Error(), 1.0e-5);
}


TEST(SimulatingTest, Complex)
{
    static char pF[] = "pF";
    static char pA[] = "pA";
    static char pT[] = "pT";
    static char pU[] = "pU";
    static char pD[] = "pD";

    static char tR[] = "tR";
    static char tT[] = "tT";
    static char tF[] = "tF";
    static char tG[] = "tG";
    static char tMu[] = "tMu";
    static char tLambda[] = "tLambda";
    static double r = -1;
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
//    SamplingSummary summary;
//    for(auto& e:estimators)
//    {
//        summary += e.GetResult(0);
//    }
//    ConfidenceInterval interval_total(summary);
//    std::cout << "summary:" << interval_total.ToString() << std::endl;
};