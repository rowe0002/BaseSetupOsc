//
//  BaseSetup.cpp
//  BaseSetup
//
//  Created by Robert Rowe on 12/30/25.
//

#include "BaseSetup.hpp"
#include "Scheduler.hpp"

Scheduler* scheduler = nullptr;

BaseSetup::BaseSetup(void)
{
    osc = new Oscillator();
    AddUG(osc);
}

BaseSetup::~BaseSetup(void)
{
    delete osc;
}

void BaseSetup::RouteAudio(double** mixChannels)
{
    int i, j;
    
    for (i=0; i<Unit::bufferSize; i++)
        for (int j=0; j<ugIndex; j++)
            ugs[j]->Sample(i);
    
    for (i=0; i<2; i++)
        for (j=0; j<Unit::bufferSize; j++)
            mixChannels[i][j] = 0.0;
    
    osc->MixOutputSamples(mixChannels, 2);
}
