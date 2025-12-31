/*
 *  Event.cpp
 *
 *  Created by Robert Rowe on 12/6/06.
 *  Copyright 2006 Robert Rowe. All rights reserved.
 *
 */

#include "Event.hpp"    // note or chord event structure
#include "Note.hpp"		// pitch, velocity, and duration
#include <stdarg.h>     // definition of variable argument mechanism

/* Event constructor : initialize class data members */
Event::Event(void) : maxNotes(12)		// up to 12 notes in an event
{
	int i;

	prev		  = nullptr;			// each event is self-standing until
	next		  = nullptr;			// linked by an EventBlock
	eventBlock    = nullptr;

	time		  = 0L;					// absolute attack time
	interval	  = 0L;					// interval since previous Event
	numChans	  = 1;					// number of channels
	for (i=0; i<kMaxChans; i++)
		whichChans[i] = 0;				// which ones
	eventDuration = 0L;					// mark no duration yet

	chordSize	  = 0;					// number of notes in the Event
	notes = new Note*[maxNotes];
	for (i=0; i<maxNotes; i++)			// construct Notes
		notes[i] = new Note(this);		// pass along pointer to Event
	
	for (i=0; i<kMaxFeatures; i++)
		featureVals[i] = 0;				// set feature classifications to zero
	segmentID	  = -1;					// no segment assigned
	lastInSegment = false;				// does not end a segment
}

/* Event constructor : initialize class data members */
Event::Event(class EventBlock* block) : maxNotes(12)
{
    int i;

	prev		  = nullptr;			// each event is self-standing until
	next		  = nullptr;			// linked by EventBlock
	eventBlock    = block;

	time		  = 0L;					// absolute attack time
	interval	  = 0L;					// interval since previous Event
	numChans	  = 1;					// number of channels
	for (i=0; i<kMaxChans; i++)
		whichChans[i] = 0;				// which ones
	eventDuration = 0L;					// mark no duration yet

	chordSize	  = 0;					// number of notes in chord
	notes = new Note*[maxNotes];
	for (i=0; i<maxNotes; i++)			// construct Notes
		notes[i] = new Note(this);
	
	for (i=0; i<kMaxFeatures; i++)
		featureVals[i] = 0;				// set feature classifications to zero
	segmentID	  = -1;					// no segment assigned
	lastInSegment = false;				// does not end a segment
}

/* Event copy constructor */
Event::Event(const Event& rhs) : maxNotes(rhs.maxNotes)
{
    int i;

	eventBlock    = nullptr;
	
	time		  = rhs.time;			// absolute attack time
	interval	  = rhs.interval;		// interval since previous Event
	numChans	  = rhs.numChans;		// number of channels
	for (i=0; i<numChans; i++)
		whichChans[i] = rhs.whichChans[i];
	eventDuration = rhs.eventDuration;	// averaged duration of event

	chordSize	  = rhs.chordSize;		// number of notes in chord
	notes = new Note*[maxNotes];
	for (i=0; i<maxNotes; i++)			// allocate Notes
		notes[i] = new Note(this);
	for (i=0; i<maxNotes; i++)			// copy Notes
		*(notes[i]) = *(rhs.notes[i]);
	// copy feature classifications
	for (i=0; i<kMaxFeatures; i++)
		featureVals[i] = rhs.featureVals[i];
	segmentID	  = -1;					// no segment assigned
	lastInSegment = false;				// does not end a segment
}

/* Event equality operator */
Event& Event::operator=(const Event& rhs)
{
    int i;

	time		  = rhs.time;			// absolute attack time
	interval	  = rhs.interval;		// interval since previous Event
	numChans	  = rhs.numChans;		// number of channels
	for (i=0; i<numChans; i++)
		whichChans[i] = rhs.whichChans[i];
	eventDuration = rhs.eventDuration;	// averaged duration of event

	chordSize	  = rhs.chordSize;		// number of notes in chord
	for (i=0; i<maxNotes; i++)			// copy Notes
		*(notes[i]) = *(rhs.notes[i]);
	// copy feature classifications
	for (i=0; i<kMaxFeatures; i++)
		featureVals[i] = rhs.featureVals[i];
	
	segmentID     = rhs.segmentID;
	lastInSegment = rhs.lastInSegment;
	return *this;
}

/* Event destructor */
Event::~Event(void)
{
	for (int i=0; i<maxNotes; i++)
		delete notes[i];				// release notes memory
	delete [] notes;
}

/* CalculateEventDuration: determine the average duration of the Notes in 
   the Event and set the event duration to the average */

long Event::CalculateEventDuration(void)
{
	long durationSum	  = 0L;			// initialize average to zero
	int  durationComplete = 0;			// count Notes with completed duration (note off has arrived)

	for (int i=0; i<chordSize; i++) {
		long duration = notes[i]->Duration();
		if (duration > 0) {				// if Note duration is greater than zero,
			durationSum += duration;	// note off has arrived and duration goes
			++durationComplete;			// into average
		}
	}
	if (durationComplete) {				// if at least one duration is complete
		eventDuration = durationSum/durationComplete;
		return eventDuration;			// set event duration to the average
	} else
		return 500L;					// otherwise return a default of 500 milliseconds
}

/* CopyChans: Get channel settings from Player */
void Event::CopyChans(int numChans, int* whichChans)
{
	this->numChans = numChans;
	for (int i=0; i<numChans; i++)
		this->whichChans[i] = whichChans[i];
}

class Note*	Event::Notes(int n)	const
{
	if ((n>=0) && (n<maxNotes))			// make sure notes index is legal
		return notes[n];				// return pointer if it is
	else
		return nullptr;					// return nullptr if not
}

/* SetChans: Determine the channels on which the Event will transmit notes */
void Event::SetChans(int nc, ...)
{
	va_list args;

	va_start(args, nc);
	numChans = nc;						// record number of channels used
	for (int i=0; i<nc; i++)			// fill whichChans array with channel numbers
		whichChans[i] = va_arg(args, int);
	va_end(args);
}

/* IsBefore: returns true if this event is before the arg event, false otherwise */
bool Event::IsBefore(Event* other)
{
	return (time < other->Time());
}

/* IsAfter:  returns true if this event is after the arg event, false otherwise */
bool Event::IsAfter(Event* other)
{
	return (time > other->Time());
}

/* IsConcurrent:  returns true if this event is concurrent with the arg event, false otherwise */
bool Event::IsConcurrent(Event* other)
{
  return ((time == other->time) && (eventDuration == other->eventDuration));
}

/* Overlaps:  returns true if this event overlaps the arg event, false otherwise */
bool Event::Overlaps(Event* other)
{
    if (this->IsAfter(other))
        return (this->time < (other->time+other->eventDuration));
    else
        return (other->time < (this->time+this->eventDuration));
}

/* NumEventsTo: counts the number of events between two members of the
   same EventBlock */
int Event::NumEventsTo(Event* other)
{
	Event* traverse = this;
	if (traverse->EventBlock() != other->EventBlock())
		return 0;					// if in different blocks, return zero

	int count = 0;					// initialize count to zero
	while (traverse != other) {		// as long as event pointers differ
		++count;					// increment the count
		traverse = traverse->Next();// and continue to look
	}
	return count;					// return number of events between the two
}
