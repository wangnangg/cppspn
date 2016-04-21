//
// Created by wangnan on 16-4-19.
//

#include "helper.h"
#include <chrono>
#include <iostream>

static std::chrono::high_resolution_clock stopwatch;
static std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> start_time;

size_t p1_uid;
size_t p2_uid;

void StartClock()
{
    start_time = stopwatch.now();
}

double StopClock()
{
    auto duration = stopwatch.now() - start_time;
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    std::cout << "duration:" << milliseconds.count() << "ms" << std::endl;
    return milliseconds.count() / 1000.0;
}

PetriNet SimplePetriNet()
{
    auto pn = PetriNet();
    p1_uid = pn.AddPlace("p1");
    p2_uid = pn.AddPlace("p2");
    double lambda_t1 = 1.0;
    double lambda_t2 = 2.0;
    pn.AddTransition("t1", Statistics::Exp(lambda_t1));
    pn.AddTransition("t2", Statistics::Exp(lambda_t2));
    pn.AddArc("t1", "p1", ArcType::Input, 1);
    pn.AddArc("t1", "p2", ArcType::Output, 1);
    pn.AddArc("t2", "p2", ArcType::Input, 1);
    pn.AddArc("t2", "p1", ArcType::Output, 1);
    pn.SetInitMark("p1", 1);

    return pn;

}
