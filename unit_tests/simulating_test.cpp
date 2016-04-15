//
// Created by wangnan on 16-1-25.
//
#include <gtest/gtest.h>
#include <Simulating.h>
#include <PetriNetModel.h>
#include <iostream>

using namespace PetriNetModel;
using namespace Simulating;

double SampleFunc()
{
    return 1.0;
}

TEST(simulating_test, running_test)
{
    auto pn = std::make_shared<PetriNet>();
    pn->AddPlace("p1");
    pn->AddPlace("p2");
    pn->AddTransition("t1", SampleFunc);
    pn->AddTransition("t2", SampleFunc);
    pn->AddArc("t1", "p1", ArcType::Input, 1);
    pn->AddArc("t1", "p2", ArcType::Output, 1);
    pn->AddArc("t2", "p2", ArcType::Input, 1);
    pn->AddArc("t2", "p1", ArcType::Output, 1);
    pn->SetInitMark("p1", 1);
    PetriNetSimulator simulator(pn, 10.50, 100);
    simulator.RunToEnd();
    ASSERT_EQ(simulator.CurrentDynamic().NextTime(), 10.50);
}
