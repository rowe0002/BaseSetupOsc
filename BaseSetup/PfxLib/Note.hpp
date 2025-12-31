/*
 *  Note.hpp
 *
 *  Created by Robert Rowe on 12/6/06.
 *  Copyright 2006 Robert Rowe. All rights reserved.
 *
 */

#pragma once

class Note
{
private:
	class Event*	event;		// surrounding event
	int				pitch;		// MIDI pitch number
	int				velocity;	// MIDI velocity
	long			duration;	// duration in msec

public:
	Note(class Event *event);	// constructor
	Note(const Note& rhs);		// copy constructor
	Note& operator=(const Note& rhs);

	// access to data members
	class Event*	Event   (void) const		 { return event;			}
	int				GetPitch(void) const 		 { return pitch;			}
	int				Velocity(void) const 		 { return velocity;			}
	long			Duration(void) const 		 { return duration;			}

	// modification of data members
	void			SetPitch   (int  newPitch)	 { pitch	= newPitch;		}
	void			SetVelocity(int  newVelocity){ velocity = newVelocity;	}
	long			SetDuration(long newDuration);
};
