/*
 *  EventBlock.h
 *
 *  Created by Robert Rowe on 12/6/06.
 *  Copyright 2006 Robert Rowe. All rights reserved.
 *
 */

#pragma		once

#include "Event.hpp"

class EventBlock
{
protected:
	Event**     all;
	Event*      head;
	Event*      tail;
    int         numEvents;
    int         pcs[12];

public:
	EventBlock(int size=128);
	EventBlock& operator=(const EventBlock& rhs);
   ~EventBlock(void);
	inline Event*   Member(int m)   const { return all[m];    }
	inline Event* 	Head(void)		const { return head;      }
	inline Event* 	Tail(void)		const { return tail;      }
    inline int		NumEvents(void) const { return numEvents; }

    void AppendBlock(EventBlock* aB);
    void PitchClassCount(void);
    void SetSegmentID(int ID);
    void TimeShift(long tS);
    void Truncate(int last);
};
