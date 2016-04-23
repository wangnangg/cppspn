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
    SamplingResult summary;
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

    SamplingResult summary;
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

    SamplingResult summary1;
    SamplingResult summary2;
    SamplingResult summary3;
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
    SamplingResult summary = summary1 + summary2 + summary3;
    ASSERT_EQ(summary.TotalWeight(), (double) numbers.size());
    ASSERT_NEAR(summary.Average(), average, 1e-6);
    ASSERT_NEAR(summary.VarianceSum(), variance_sum, 1e-6);

}

TEST(SamplingSummary_test, WeightCombineTest)
{
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_real_distribution<double> distribution;

    SamplingResult summary1;
    SamplingResult summary2;
    SamplingResult summary3;
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
    SamplingResult summary = summary1 + summary2 + summary3;
    ASSERT_EQ(summary.TotalWeight(), (double) numbers.size());
    ASSERT_NEAR(summary.Average(), average, 1e-6);
    ASSERT_NEAR(summary.VarianceSum(), variance_sum, 1e-6);

}


TEST(SamplingSummary_test, PropertyTest)
{
    SamplingResult result1;
    SamplingResult result2;
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_real_distribution<double> distribution;
    for (int i = 0; i < 30; i++)
    {
        double rand_num = distribution(generator);
        result1.AddNewSample(rand_num, 1.0);
        result2.AddNewSample(rand_num, 3.14);
    }
    ASSERT_DOUBLE_EQ(result1.Average(), result2.Average());
    ASSERT_DOUBLE_EQ(result1.Variance(), result2.Variance());
    ASSERT_DOUBLE_EQ(result1.AverageVariance(), result2.AverageVariance());
}


TEST(SamplingSummary_test, AverageStandardDeviationTest)
{
    SamplingResult result;
    result.AddNewSample(5, 1.23);
    result.AddNewSample(5, 2.12);
    result.AddNewSample(4, 1.23);
    result.AddNewSample(4, 0.32);
    result.AddNewSample(3, 1.53);
    result.AddNewSample(4, 0.59);
    result.AddNewSample(3, 0.94);
    result.AddNewSample(2, 0.94);
    result.AddNewSample(2, 0.84);
    result.AddNewSample(1, 0.73);
    std::cout << result.Variance() << std::endl;
    std::cout << result.EffectiveBase() << std::endl;
    std::cout << result.AverageVariance() << std::endl;
}


