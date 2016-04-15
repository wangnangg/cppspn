//
// Created by wangnan on 16-1-27.
//
#include <gtest/gtest.h>
#include <Estimating.h>
#include <PetriNetModel.h>
#include <Simulating.h>
#include <Statistics.h>
#include <random>
#include <chrono>
#include <utility>

using namespace Estimating;
using namespace PetriNetModel;
using namespace Simulating;

TEST(SamplingSummary_test, BasicTest)
{
    SamplingSummary summary;
    summary.AddNewSample(5.0, 1.0);
    ASSERT_EQ(summary.TotalWeight(), 1.0);
    ASSERT_EQ(summary.Average(), 5.0);
    ASSERT_EQ(summary.VarianceSum(), 0);

    summary.AddNewSample(5.0, 1.0);
    ASSERT_EQ(summary.TotalWeight(), 2.0);
    ASSERT_EQ(summary.Average(), 5.0);
    ASSERT_EQ(summary.VarianceSum(), 0);


    summary.AddNewSample(20.0, 1.0);
    ASSERT_EQ(summary.TotalWeight(), 3.0);
    ASSERT_EQ(summary.Average(), 10.0);
    ASSERT_EQ(summary.VarianceSum(), 150.0);
}

TEST(SamplingSummary_test, LargeTest)
{
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_real_distribution<double> distribution;

    SamplingSummary summary;
    vector<double> numbers;
    double sum = 0;
    for (int i = 0; i < 100; i++)
    {
        double number = distribution(generator);
        sum += number;
        numbers.push_back(number);
        summary.AddNewSample(number, 1.0);
    }

    double average = sum / numbers.size();
    double variance_sum = 0;
    for (double num:numbers)
    {
        variance_sum += (num - average) * (num - average);
    }
    ASSERT_EQ(summary.TotalWeight(), (double) numbers.size());
    ASSERT_NEAR(summary.Average(), average, 1e-7);
    ASSERT_NEAR(summary.VarianceSum(), variance_sum, 1e-7);

}

TEST(SamplingSummary_test, CombineTest)
{
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_real_distribution<double> distribution;

    SamplingSummary summary1;
    SamplingSummary summary2;
    SamplingSummary summary3;
    vector<double> numbers;
    double sum = 0;
    for (int i = 0; i < 30; i++)
    {
        double number = distribution(generator);
        sum += number;
        numbers.push_back(number);
        summary1.AddNewSample(number, 1.0);
    }
    for (int i = 0; i < 70; i++)
    {
        double number = distribution(generator);
        sum += number;
        numbers.push_back(number);
        summary2.AddNewSample(number, 1.0);
    }
    for (int i = 0; i < 50; i++)
    {
        double number = distribution(generator);
        sum += number;
        numbers.push_back(number);
        summary3.AddNewSample(number, 1.0);
    }
    double average = sum / numbers.size();
    double variance_sum = 0;
    for (double num:numbers)
    {
        variance_sum += (num - average) * (num - average);
    }
    SamplingSummary summary = summary1 + summary2 + summary3;
    ASSERT_EQ(summary.TotalWeight(), (double) numbers.size());
    ASSERT_NEAR(summary.Average(), average, 1e-6);
    ASSERT_NEAR(summary.VarianceSum(), variance_sum, 1e-6);

}

TEST(SamplingSummary_test, WeightCombineTest)
{
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_real_distribution<double> distribution;

    SamplingSummary summary1;
    SamplingSummary summary2;
    SamplingSummary summary3;
    vector<double> numbers;
    double sum = 0;
    for (int i = 0; i < 30; i++)
    {
        double number = distribution(generator);
        sum += number;
        numbers.push_back(number);
        summary1.AddNewSample(number, 1.0);
    }
    for (int i = 0; i < 70; i++)
    {
        double number = distribution(generator);
        sum += number;
        numbers.push_back(number);
        summary2.AddNewSample(number, 1.0);
    }
    for (int i = 0; i < 70; i++)
    {
        double number = distribution(generator);
        sum += number;
        sum += number;
        sum += number;
        numbers.push_back(number);
        numbers.push_back(number);
        numbers.push_back(number);
        summary2.AddNewSample(number, 3.0);
    }
    for (int i = 0; i < 50; i++)
    {
        double number = distribution(generator);
        sum += number;
        sum += number;
        numbers.push_back(number);
        numbers.push_back(number);
        summary3.AddNewSample(number, 2.0);
    }
    double average = sum / numbers.size();
    double variance_sum = 0;
    for (double num:numbers)
    {
        variance_sum += (num - average) * (num - average);
    }
    SamplingSummary summary = summary1 + summary2 + summary3;
    ASSERT_EQ(summary.TotalWeight(), (double) numbers.size());
    ASSERT_NEAR(summary.Average(), average, 1e-6);
    ASSERT_NEAR(summary.VarianceSum(), variance_sum, 1e-6);

}


bool IsOnP1(const PetriNetDynamic &dynamic, double &value, double &weight)
{
    value = (double) dynamic.GetMark("p1");
    weight = dynamic.Duration();
    return true;
}

TEST(EstimatorTest, SmokeTest)
{
    auto pn = std::make_shared<PetriNet>();
    pn->AddPlace("p1");
    pn->AddPlace("p2");
    pn->AddTransition("t1", Statistics::Exp(1));
    pn->AddTransition("t2", Statistics::Exp(2));
    pn->AddArc("t1", "p1", ArcType::Input, 1);
    pn->AddArc("t1", "p2", ArcType::Output, 1);
    pn->AddArc("t2", "p2", ArcType::Input, 1);
    pn->AddArc("t2", "p1", ArcType::Output, 1);
    pn->SetInitMark("p1", 1);
    PetriNetSimulator simulator(pn, 1000.04, 4000);
    PetriNetSteadyStateSampler sampler(simulator);
    ExpectationEstimator<PetriNetDynamic, PetriNetSteadyStateSampler> estimator(sampler);
    estimator.AddRandomVariable(IsOnP1);
    estimator.Estimate();
    ConfidenceInterval interval = estimator.GetResult(0);
    std::cout << interval.ToString() << std::endl;
}

TEST(EstimatorTest, AsyncTest)
{
    auto pn = std::make_shared<PetriNet>();
    pn->AddPlace("p1");
    pn->AddPlace("p2");
    pn->AddTransition("t1", Statistics::Exp(5));
    pn->AddTransition("t2", Statistics::Exp(10));
    pn->AddArc("t1", "p1", ArcType::Input, 1);
    pn->AddArc("t1", "p2", ArcType::Output, 1);
    pn->AddArc("t2", "p2", ArcType::Input, 1);
    pn->AddArc("t2", "p1", ArcType::Output, 1);
    pn->SetInitMark("p1", 1);
    vector<ExpectationEstimator<PetriNetDynamic, PetriNetSteadyStateSampler>> estimators;
    PetriNetSimulator simulator(pn, 1000.04, 1000);
    PetriNetSteadyStateSampler sampler(simulator);
    ExpectationEstimator<PetriNetDynamic, PetriNetSteadyStateSampler> estimator(sampler);
    estimator.AddRandomVariable(IsOnP1);
    for (int i = 0; i < 4; i++)
    {
        estimators.push_back(estimator);
    }
    for (auto &e: estimators)
    {
        e.EstimateAsync();
    }

    for (auto &e:estimators)
    {
        ASSERT_EQ(e.Wait(), true);
        ConfidenceInterval interval = e.GetResult(0);
        std::cout << interval.ToString() << std::endl;
    }
    SamplingSummary summary;
    for (auto &e:estimators)
    {
        summary += e.GetResult(0);
    }
    ConfidenceInterval interval_total(summary);
    std::cout << "summary:" << interval_total.ToString() << std::endl;
};

TEST(EstimatorTest, Complicated)
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
    auto pn = std::make_shared<PetriNet>();

    pn->AddPlace(pF);
    pn->SetInitMark(pF, 1);
    pn->AddPlace(pA);
    pn->AddPlace(pT);
    pn->AddPlace(pU);
    pn->SetInitMark(pU, 1);
    pn->AddPlace(pD);

    pn->AddTransition(tR, Statistics::Exp(1));
    pn->AddTransition("t2", Statistics::Exp(2));

    pn->AddArc("t1", "p1", ArcType::Input, 1);
    pn->AddArc("t1", "p2", ArcType::Output, 1);
    pn->AddArc("t2", "p2", ArcType::Input, 1);
    pn->AddArc("t2", "p1", ArcType::Output, 1);
//    vector<ExpectationEstimator<PetriNetDynamic, PetriNetSteadyStateSampler>> estimators;
//    PetriNetSimulator simulator(pn, 1000.04, 1000);
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