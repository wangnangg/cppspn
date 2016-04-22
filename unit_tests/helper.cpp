//
// Created by wangnan on 16-4-19.
//

#include "helper.h"
#include <chrono>
#include <iostream>
#include "gtest/gtest.h"

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

bool IsOnP1(const PetriNet &petri_net, double &value)
{
    value = petri_net.GetPlaceMark(p1_uid);
    return true;
}

PetriNetCreator SimplePetriNet()
{
    auto creator = PetriNetCreator();
    p1_uid = creator.AddPlace("p1", 1);
    p2_uid = creator.AddPlace("p2", 0);
    double lambda_t1 = 1.0;
    double lambda_t2 = 2.0;
    creator.AddTransition("t1", Statistics::Exp(lambda_t1));
    creator.AddTransition("t2", Statistics::Exp(lambda_t2));
    creator.AddArc("t1", "p1", Arc::Type::Input, 1);
    creator.AddArc("t1", "p2", Arc::Type::Output, 1);
    creator.AddArc("t2", "p2", Arc::Type::Input, 1);
    creator.AddArc("t2", "p1", Arc::Type::Output, 1);
    creator.Commit();
    return creator;
}

static size_t pF_uid;
static size_t pA_uid;
static size_t pT_uid;
static size_t pU_uid;
static size_t pD_uid;

bool UserUnavail(const PetriNet &petri_net, double &value)
{
    if (petri_net.GetPlaceMark(pT_uid) == 1)
    {
        return false;
    }
    if (petri_net.GetPlaceMark(pA_uid) == 1 && petri_net.GetPlaceMark(pU_uid) == 1)
    {
        value = 0.0;
        return true;
    }
    value = 1.0;
    return true;
}

PetriNetCreator ComplexPetriNet()
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

    auto creator = PetriNetCreator();

    pF_uid = creator.AddPlace(pF, 0);
    pA_uid = creator.AddPlace(pA, 0);
    pT_uid = creator.AddPlace(pT, 1);
    pU_uid = creator.AddPlace(pU, 1);
    pD_uid = creator.AddPlace(pD, 0);

    creator.AddTransition(tF, Weibull(0.88, std::exp(4.5)));
    creator.AddTransition(tG, ParetoTrunc(0.5, 60, 6000));
    creator.AddTransition(tR, Exp(0.1));
    creator.AddTransition(tT, Deterministic(10.0));
    creator.AddTransition(tLambda, Exp(1.0 / 346982.0));
    creator.AddTransition(tMu, Exp(1.0 / 2581.0));

    creator.AddArc(tR, pF, Arc::Input);
    creator.AddArc(tR, pA, Arc::Output);
    creator.AddArc(tT, pA, Arc::Input);
    creator.AddArc(tT, pF, Arc::Output);
    creator.AddArc(tT, pU, Arc::Inhibitor);
    creator.AddArc(tF, pA, Arc::Input);
    creator.AddArc(tF, pT, Arc::Output);
    creator.AddArc(tF, pD, Arc::Inhibitor);
    creator.AddArc(tG, pT, Arc::Input);
    creator.AddArc(tG, pA, Arc::Output);
    creator.AddArc(tLambda, pU, Arc::Input);
    creator.AddArc(tLambda, pD, Arc::Output);
    creator.AddArc(tMu, pD, Arc::Input);
    creator.AddArc(tMu, pU, Arc::Output);

    creator.Commit();
    return creator;

}

string GetStateName(const PetriNet &pn)
{
    auto mF = pn.GetPlaceMark("pF");
    auto mA = pn.GetPlaceMark("pA");
    auto mT = pn.GetPlaceMark("pT");
    auto mU = pn.GetPlaceMark("pU");
    auto mD = pn.GetPlaceMark("pD");

    string user;
    string server;
    auto user_value = mF * 1 + mA * 2 + mT * 4;
    auto server_value = mU * 1 + mD * 2;
    switch (user_value)
    {
        case 1:
            user = "F";
            break;
        case 2:
            user = "A";
            break;
        case 4:
            user = "T";
            break;
        default:
            throw std::exception();
            break;
    }
    switch (server_value)
    {
        case 1:
            server = "U";
            break;
        case 2:
            server = "D";
        default:
            throw std::exception();
            break;
    }
    return (server + user);
}