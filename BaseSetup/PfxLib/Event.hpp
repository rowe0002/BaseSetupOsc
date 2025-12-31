/*
 *  Event.h
 *
 *  Created by Robert Rowe on 12/6/06.
 *  Copyright 2006 Robert Rowe. All rights reserved.
 *
 */

#pragma	once

class Event
{
public:
    class Note**    notes;                    // array of Note pointers

protected:
	Event*			prev;
	Event*			next;
	
	enum			{ kMaxChans = 16, kMaxFeatures = 32 };

	unsigned int    maxNotes;				// max number of notes in this Event
	long			time;					// the absolute time of the event
    unsigned int    id;                     // ID number of event in score
	long			interval;				// inter-onset interval since prev
	int				numChans;				// how many output channels
	int				whichChans[kMaxChans];	// which channels they are
	long			eventDuration;			// averaged duration of event
	int				chordSize;				// how many notes actually present
	int				featureVals[kMaxFeatures];

	class EventBlock* eventBlock;
	int               segmentID;
	bool              lastInSegment;

public:
	Event(void);
	Event(class EventBlock* block);
	Event(const Event& rhs);
	Event&			operator=(const Event& rhs);
	virtual ~Event(void);

	long			CalculateEventDuration(void);
	
	// access to data members
	Event*			Next(void)			 const		{ return next;		  		}
	Event*			Prev(void)			 const		{ return prev;		  		}
	int				MaxNotes(void)		 const		{ return maxNotes;    		}
	long			Time(void)			 const		{ return time;        		}
	long			IOI(void)		 	 const		{ return interval;     		}
	int				NumChans(void)		 const		{ return numChans; 	  		}
	int				WhichChans(int w)	 const		{ 
		if ((w>=kMaxChans) || (w<0)) return 0; else	  return whichChans[w];		}
	long			EventDuration(void)  const		{ return eventDuration;		}
	int				ChordSize(void)		 const		{ return chordSize;			}
	class Note*		Notes(int n) const;
	int				FeatureValue(int id) const		{ return featureVals[id];	}
	int				SegmentID(void)		 const		{ return segmentID;			}
	bool			LastInSegment(void)  const		{ return lastInSegment;     }
	class EventBlock* EventBlock(void)	 const		{ return eventBlock;		}

	// modification of data members
    void            SetID(unsigned int i)           { id       = i;             }
	void			SetNext(Event* newNext)			{ next     = newNext;   	}
	void			SetPrev(Event* newPrev)			{ prev     = newPrev;   	}
	void			SetTime(long newTime)			{ time     = newTime;  		}
	void			SetIOI(long newIOI)				{ interval = newIOI; 		}
	void			CopyChans(int numChans, int* whichChans);
	void			SetChans(int nc, ...);
	void			SetEventDuration(long newDur)	{ eventDuration = newDur;   }
	void			SetChordSize(int newChordSize)	{ chordSize = newChordSize; }
	void			SetFeatureValue(int id, int value)
													{ featureVals[id] = value;  }
	void			SetSegmentID(int ID)			{ segmentID		  = ID;		}
	void			SetLastInSegment(bool yesno)    { lastInSegment   = yesno;  }
	
	bool			IsBefore(Event* comparison);
	bool			IsAfter(Event* other);
	bool			IsConcurrent(Event* other);
	bool			Overlaps(Event* other);
	int				NumEventsTo(Event* other);
};

