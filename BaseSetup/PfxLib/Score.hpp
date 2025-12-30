/*
 *  Score.h
 *
 *  Created by Robert Rowe on 9/29/06.
 *  Copyright 2006 Robert Rowe. All rights reserved.
 *
 */

#ifndef __Score__
#define __Score__

#include "Unit.hpp"

class Score
{
public:
    static const int kMaxUgs = 100;
    int          currentState;
    int          ugIndex;
    Unit*        ugs[kMaxUgs];

public:
                 Score(void);
    virtual     ~Score(void);
    void         AddUG(Unit* ug);
    void         AllUGsOn(void);
    int          CurrentState(void) const { return currentState; }
	virtual void RouteAudio(double** mixChannels) = 0;
};

#endif //__Score__
