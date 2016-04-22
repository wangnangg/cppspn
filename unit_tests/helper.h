//
// Created by wangnan on 16-4-19.
//

#include <PetriNetModel/PetriNetModel.h>

using namespace PetriNetModel;

void StartClock();

double StopClock();

bool IsOnP1(const PetriNet &petri_net, double &value);

PetriNetCreator SimplePetriNet();

bool UserUnavail(const PetriNet &petri_net, double &value);

PetriNetCreator ComplexPetriNet();

string GetStateName(const PetriNet &pn);

