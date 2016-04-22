//
// Created by wangnan on 16-1-19.
//

#include <gtest/gtest.h>
#include "PetriNetModel/PetriNetModel.h"
#include "Statistics.h"
#include "helper.h"


TEST(petri_net_model_test, duplicate_place_name)
{
    PetriNetCreator creator;
    creator.AddPlace("ppp", 1);
    try
    {
        creator.AddPlace("ppp", 1);
        FAIL();
    } catch (DuplicateName)
    {
        return;
    } catch (...)
    {
        FAIL();
    }
    FAIL();
}

TEST(petri_net_model_test, duplicate_trans_name)
{
    PetriNetCreator creator;
    creator.AddTransition("ppp", Statistics::Exp(1.0));
    try
    {
        creator.AddTransition("ppp", Statistics::Exp(1.0));
        FAIL();
    } catch (DuplicateName)
    {
        return;
    } catch (...)
    {
        FAIL();
    }
    FAIL();
}

TEST(petri_net_model_test, creating)
{
    PetriNetCreator creator;
    creator.AddPlace("p1", 1);
    creator.AddPlace("p2", 0);
    creator.AddPlace("p3", 0);

    creator.AddTransition("t1", Statistics::Exp(0.5));
    creator.AddTransition("t2", Statistics::Exp(0.5));
    creator.AddTransition("t3", Statistics::Exp(0.5));

    creator.AddArc("t1", "p1", Arc::Type::Input, 3);
    creator.AddArc("t1", "p2", Arc::Type::Output, 2);
    creator.AddArc("t2", "p2", Arc::Type::Input, 1);
    creator.AddArc("t2", "p3", Arc::Type::Output, 1);
    creator.AddArc("t3", "p3", Arc::Type::Input, 1);
    creator.AddArc("t3", "p1", Arc::Type::Output, 1);

    creator.Commit();
}


TEST(petri_net_model_test, firing)
{
    Statistics::DefaultUniformRandomNumberGenerator generator;
    PetriNetCreator creator = SimplePetriNet();
    PetriNet pn = creator.CreatePetriNet();
    pn.Reset(generator);
    pn.NextState(generator);
    GTEST_ASSERT_EQ(pn.GetPlaceMark("p1"), 0);
    GTEST_ASSERT_EQ(pn.GetPlaceMark("p2"), 1);

    pn.NextState(generator);
    GTEST_ASSERT_EQ(pn.GetPlaceMark("p1"), 1);
    GTEST_ASSERT_EQ(pn.GetPlaceMark("p2"), 0);
}

TEST(petri_net_model_test, complex_firing)
{
    Statistics::DefaultUniformRandomNumberGenerator generator;
    PetriNetCreator creator = ComplexPetriNet();
    auto pn = creator.CreatePetriNet();
    pn.Reset(generator);

    for (int i = 0; i < 100; i++)
    {
        std::cout << GetStateName(pn) << std::endl;
        std::cout << pn.GetDuration() << std::endl << std::endl;
        pn.NextState(generator);
        auto mF = pn.GetPlaceMark("pF");
        auto mA = pn.GetPlaceMark("pA");
        auto mT = pn.GetPlaceMark("pT");
        auto mU = pn.GetPlaceMark("pU");
        auto mD = pn.GetPlaceMark("pD");
        GTEST_ASSERT_EQ(mF + mA + mT, 1);
        GTEST_ASSERT_EQ(mU + mD, 1);
    }
}


