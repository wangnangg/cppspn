//
// Created by wangnan on 16-1-19.
//

#include <gtest/gtest.h>
#include "PetriNetModel/PetriNetModel.h"
#include "Statistics.h"
#include "helper.h"


TEST(petri_net_model_test, duplicate_place_name)
{
    PetriNet pn;
    pn.AddPlace("ppp");
    try
    {
        pn.AddPlace("ppp");
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
    PetriNet pn;
    pn.AddTransition("ppp", Statistics::Exp(1.0));
    try
    {
        pn.AddTransition("ppp", Statistics::Exp(1.0));
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
    PetriNet pn;
    pn.AddPlace("p1");
    pn.AddPlace("p2");
    pn.AddPlace("p3");

    pn.AddTransition("t1", Statistics::Exp(0.5));
    pn.AddTransition("t2", Statistics::Exp(0.5));
    pn.AddTransition("t3", Statistics::Exp(0.5));

    pn.AddArc("t1", "p1", ArcType::Input, 3);
    pn.AddArc("t1", "p2", ArcType::Output, 2);
    pn.AddArc("t2", "p2", ArcType::Input, 1);
    pn.AddArc("t2", "p3", ArcType::Output, 1);
    pn.AddArc("t3", "p3", ArcType::Input, 1);
    pn.AddArc("t3", "p1", ArcType::Output, 1);

}


TEST(petri_net_model_test, firing)
{
    Statistics::DefaultUniformRandomNumberGenerator generator;
    PetriNet pn = SimplePetriNet();
    pn.Reset(generator);
    pn.NextState(generator);
    GTEST_ASSERT_EQ(pn.GetPlaceMark("p1"), 0);
    GTEST_ASSERT_EQ(pn.GetPlaceMark("p2"), 1);

    pn.NextState(generator);
    GTEST_ASSERT_EQ(pn.GetPlaceMark("p1"), 1);
    GTEST_ASSERT_EQ(pn.GetPlaceMark("p2"), 0);
}


