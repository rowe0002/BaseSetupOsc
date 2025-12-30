/*
 *  Score.cpp
 *
 *  Created by Robert Rowe on 9/29/06.
 *  Copyright 2006 Robert Rowe. All rights reserved.
 *
 */

#include "Score.hpp"

Score::Score(void) : ugIndex(0), currentState(0) {}

Score::~Score(void) {}

void Score::AddUG(Unit* ug)
{
    if ((ugIndex >= kMaxUgs) || (ugIndex < 0))
        return;
    
    ugs[ugIndex++] = ug;
}

void Score::AllUGsOn(void)
{
    for (unsigned i=0; i<ugIndex; i++)
        ugs[i]->TurnOn();
}
