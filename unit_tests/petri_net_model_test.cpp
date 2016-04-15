//
// Created by wangnan on 16-1-19.
//

#include <gtest/gtest.h>
#include "PetriNetModel.h"
#include "Statistics.h"

using namespace PetriNetModel;


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

    std::cout << pn.ToString() << std::endl;
}


TEST(petri_net_model_test, firing)
{
    PetriNet pn;
    pn.AddPlace("p1");
    pn.AddPlace("p2");
    pn.AddTransition("t1", Statistics::Exp(0.5));
    pn.AddTransition("t2", Statistics::Exp(0.6));
    pn.AddArc("t1", "p1", ArcType::Input, 1);
    pn.AddArc("t1", "p2", ArcType::Output, 1);
    pn.AddArc("t2", "p2", ArcType::Input, 1);
    pn.AddArc("t2", "p1", ArcType::Output, 1);
    std::cout << pn.ToString() << std::endl;
    pn.SetInitMark("p1", 1);
    PetriNetDynamic dynamic = pn.ForkInitDynamic();
    pn.PrepareToFire(dynamic);
    pn.Fire(dynamic);
    GTEST_ASSERT_EQ(pn.GetPlaceMark(dynamic, "p1"), 0);
    GTEST_ASSERT_EQ(pn.GetPlaceMark(dynamic, "p2"), 1);

    pn.PrepareToFire(dynamic);
    pn.Fire(dynamic);
    GTEST_ASSERT_EQ(pn.GetPlaceMark(dynamic, "p1"), 1);
    GTEST_ASSERT_EQ(pn.GetPlaceMark(dynamic, "p2"), 0);
}


