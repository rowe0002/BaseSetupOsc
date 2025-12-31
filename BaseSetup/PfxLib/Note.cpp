/*
 *  Note.cpp
 *
 *  Created by Robert Rowe on 12/6/06.
 *  Copyright 2006 Robert Rowe. All rights reserved.
 *
 */

#include "Note.hpp"
#include "Event.hpp"

/* Note constructor */
Note::Note(class Event* event)
{
	this->event = event;
	pitch		= -1;					// indicate no pitch
	velocity	= -1;					// indicate no velocity
	duration	= -1;					// indicate no duration
}

/* Note copy constructor */
Note::Note(const Note& rhs)
{										// (leave event pointer the way it was)
	pitch	 = rhs.pitch;				// copy right-hand-side pitch
	velocity = rhs.velocity;			// copy right-hand-side velocity
	duration = rhs.duration;			// copy right-hand-side duration
}

/* Note equality operator */
Note& Note::operator=(const Note& rhs)
{										// (leave event pointer the way it was)
	pitch	 = rhs.pitch;				// copy right-hand-side pitch
	velocity = rhs.velocity;			// copy right-hand-side velocity
	duration = rhs.duration;			// copy right-hand-side duration
	return *this;
}

/* Change duration field of a Note */
long Note::SetDuration(long newDuration)
{
	duration = newDuration;				// change duration to new value
	if (duration > 0)					// if positive, return average event duration
		return event->CalculateEventDuration();
	else
		return 500L;					// otherwise return a default of 500 milliseconds
}
