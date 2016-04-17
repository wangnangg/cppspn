//
// Created by wangnan on 16-1-27.
//
#include <gtest/gtest.h>
#include <Estimating.h>
#include <PetriNetModel/PetriNetModel.h>
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

