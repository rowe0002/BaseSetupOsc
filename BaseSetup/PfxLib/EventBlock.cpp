/*
 *  EventBlock.cpp
 *
 *  Created by Robert Rowe on 12/6/06.
 *  Copyright 2006 Robert Rowe. All rights reserved.
 *
 */

#include "EventBlock.hpp" // doubly linked array of Events
#include "Note.hpp"

EventBlock::EventBlock(int size) : numEvents(size)
{
	int i;

	all = new Event*[size];         // allocate space for Event pointers

	for (i=0; i<size; i++)
		all[i] = new Event(this);   // fill with newly allocated Events

	for (i=0; i<size-1; i++)		// initialize pointers to next
		all[i]->SetNext(all[i+1]);
	all[i]->SetNext(all[0]);        // last event points to first

	all[0]->SetPrev(all[size-1]);	// first event points to last
	for (i=1; i<size; i++)
		all[i]->SetPrev(all[i-1]);  // initialize pointers to prev
	head = all[0];
	tail = all[size-1];
    
    for (i=0; i<12; i++) pcs[i] = 0;
}

/* EventBlock assignment operator */
EventBlock& EventBlock::operator=(const EventBlock& rhs)
{
    if (rhs.numEvents > numEvents) return *this;
    numEvents = rhs.numEvents;
	for (int i=0; i<numEvents; i++)
		*(all[i]) = *(rhs.all[i]);  // copy all Events from rhs to this

	return *this;
}

EventBlock::~EventBlock(void)
{
	for (int i=0; i<numEvents; i++)
		delete all[i];
	delete [] all;
}

// change allocation to accommodate additional events
void EventBlock::AppendBlock(EventBlock* aB)
{
//    int newTotal = numEvents + aB->NumEvents();
//    Event* e     = all[numEvents-1];
//    long baseTime = e->Time();
//    for (int i=0; i<aB->NumEvents(); i++)
//    {
//        e = e->Next();
//        *e = *aB->Member(i);
//        e->SetTime(e->Time()+baseTime);
//    }
//    numEvents = newTotal;
}

void EventBlock::PitchClassCount(void)
{
    int i, j;
    for (i=0; i<12; i++) pcs[i] = 0;
    Event* e = head;
    for (i=0; i<numEvents; i++)
    {
        for (j=0; j<e->ChordSize(); j++)
        {
            Note* n = e->Notes(j);
            pcs[n->GetPitch()%12] += 1;
        }
        e = e->Next();
    }
}

void EventBlock::SetSegmentID(int ID)
{
	for (int i=0; i<numEvents; i++)
		all[i]->SetSegmentID(ID);
}

void EventBlock::TimeShift(long tS)
{
    Event* e = head;
    for (int i=0; i<numEvents; i++)
    {
        e->SetTime(e->Time()+tS);
        e = e->Next();
    }
}

void EventBlock::Truncate(int last) // check last == 0
{
    for (int i=last; i<numEvents; i++)
        delete all[i];
    tail = all[last-1];
    tail->SetNext(head);
    head->SetPrev(tail);
    numEvents = last;
}
