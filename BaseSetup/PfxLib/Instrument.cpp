//
//  Instrument.cpp
//  Instrument
//
//  Created by Robert Rowe on 7/30/24.
//

#include "Instrument.hpp"
#include "Scheduler.hpp"
#include <iostream>
using namespace std;

extern Scheduler* scheduler;

Instrument::Instrument(void)
{
    numChans = 1;
    env      = new Envelopes;
    env->SetEtype(Envelopes::kADSR);
    env->calculateADSRParams(1000, 0.01, 0.1, 0.4, 0.25);
    usingEnvelope = false;
}

Instrument::~Instrument(void)
{
    delete env;
}

void CNote(void* args)
{
    NoteOutArgs* n = static_cast<NoteOutArgs*>(args);
    n->s->NoteOut(n->pitch, n->velocity, n->duration);
    if (!n->repeat) delete n;
}

void Instrument::BeatNote(double onsetBeat, int pitch, int velocity, double duration)
{
    double msTime = (60000.0 / scheduler->MM);          // beat duration in milliseconds
    long   dur    = static_cast<long>(msTime*duration); // length of note in milliseconds

    // if the onset time is zero, send out note immediately
    if (onsetBeat == 0.0)
        NoteOut(pitch, velocity, dur);
    else
        // otherwise schedule event later
        if (scheduler)
        {
            NoteOutArgs* n = new NoteOutArgs;
            n->s           = this;
            n->pitch       = pitch;
            n->velocity    = velocity;
            n->duration    = dur;
            n->repeat      = false;
            scheduler->ScheduleBeatTask(onsetBeat, 0, CNote, static_cast<void*>(n));
        }
}

/* MakeNote: schedules transmission of NoteOut messages to perform Note object */
void Instrument::MakeNote(long onset, int pitch, int velocity, long duration)
{
    if (onset == 0L) // if the onset time is zero, send out note immediately
    {
        NoteOut(pitch, velocity, duration);
        return;
    }
    
    if (scheduler == nullptr)
    {
        cout << "No scheduler" << endl;
        exit(1);
    }

    NoteOutArgs* n = new NoteOutArgs;
    n->s           = this;
    n->pitch       = pitch;
    n->velocity    = velocity;
    n->duration    = duration;
    n->repeat      = false;
    scheduler->ScheduleTask(onset, 0, CNote, static_cast<void*>(n));
}

/* MakeNote: schedules transmission of NoteOut messages to perform Note object at regular intervals*/
Task* Instrument::MakeNote(long onset, int pitch, int velocity, long duration, int IOI)
{
    if (scheduler == nullptr)
    {
        cout << "No scheduler" << endl;
        exit(1);
    }

//
//    NoteOutArgs* n = new NoteOutArgs;
//    n->s           = this;
//    n->pitch       = pitch;
//    n->velocity    = velocity;
//    n->duration    = duration;
//    n->repeat      = true;
//    return scheduler->ScheduleTask(onset, IOI, CNote, static_cast<void*>(n));
    return nullptr;
}

void Instrument::NoteOut(int pitch, int velocity, long durationMS)
{
    SetFreq(pitch);
    SetVolume(VelocityToAmplitude(velocity));
    env->TurnOn();
    usingEnvelope = true;
}

void Instrument::Play(Event* e)
{
    for (int i=0; i<e->ChordSize(); i++)
    {
        Note* n   = e->Notes(i);
        long  dur = n->Duration();
        if (dur <= 0)
            dur = (rand()%800) + 300;
        MakeNote(e->Time(), n->GetPitch(), n->Velocity(), dur);
    }
}

void Instrument::PlayEventBlock(EventBlock* eB)
{
    Event *e = eB->Head();
    for (int i=0; i<eB->NumEvents(); i++)
    {
        Play(e);
        e = e->Next();
    }
}
