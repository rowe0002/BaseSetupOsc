//
//  Instrument.hpp
//  Instrument
//
//  Created by Robert Rowe on 7/30/24.
//

#ifndef Instrument_hpp
#define Instrument_hpp

#include "Unit.hpp"
#include "Envelopes.hpp"
#include "Event.hpp"
#include "EventBlock.hpp"
#include "Note.hpp"

typedef struct
{
    class Instrument* s;
    int   pitch;
    int   velocity;
    long  duration;
    bool  repeat;
    bool  available;
} NoteOutArgs;

class Instrument : public Unit
{
public:
    Envelopes* env;
    bool       usingEnvelope;

public:
    Instrument(void);
   ~Instrument(void);
    void  BeatNote(double onsetBeat, int pitch, int velocity, double duration);
    void  MakeNote(long onset, int pitch, int velocity, long duration);
    Task* MakeNote(long onset, int pitch, int velocity, long duration, int IOI);
    virtual void  NoteOut(int pitch, int velocity, long duration);
    void  Play(Event* e);
    void  PlayEventBlock(EventBlock* eB);
    virtual void SetFreq(double freq) {}
    virtual void SetFreq(int pitch)   {}
};

void CNote(void* args);

#endif /* Instrument_hpp */
