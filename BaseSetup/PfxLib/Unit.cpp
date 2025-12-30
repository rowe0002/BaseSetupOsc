/*
 *  Unit.cpp
 *
 *  Created by Robert Rowe on 9/29/06.
 *  Copyright 2006 Robert Rowe. All rights reserved.
 *
 */

#include "Unit.hpp"
#include <cstddef>
#include <math.h>

const int    Unit::kMaxChans    = 2;
unsigned int Unit::bufferSize   = 0;
double       Unit::samplingRate = 0.0;

Unit::Unit(void) : channel(0), numChans(2), volumeTask(nullptr), active(false), bypass(false), inputUnit(nullptr), volume(1.0)
{
	Init();
}

Unit::Unit(int chans) : channel(0), volumeTask(nullptr), active(false), bypass(false), inputUnit(nullptr), volume(1.0)
{
	numChans = chans;
	Init();
}

void Unit::Init(void)
{
    unsigned i, j;

    if (samplingRate == 0.0)
		samplingRate = 44100.0;
	if (bufferSize	 == 0)
		bufferSize = 512;

	outputSamples = new double*[numChans];
	for (i=0; i<numChans; i++)
	{
		outputSamples[i] = new double[bufferSize];
		feedback     [i] = 0.0;
        for (j=0; j<bufferSize; j++)
            outputSamples[i][j] = 0.0;
	}

	analysisValue = 0.0;
	desiredVol	  = 1.0;
	effect		  = nullptr;
	inputChannel  = 0;
	unitInc       = 0.0;
	controlRate   = samplingRate / bufferSize;
	msPerSample   = 1000.0 / samplingRate;
}

Unit::~Unit(void)
{
	for (unsigned i=0; i<numChans; i++)
		delete [] outputSamples[i];
	delete [] outputSamples;
}

void Unit::Bypass(int sNo)
{
    unsigned i;
    
    if (inputUnit == nullptr)
    {
        for (i=0; i<numChans; i++)
            outputSamples[i][sNo] = 0.0;
        return;
    }
    
    double** in      = inputUnit->OutputSamples();
    unsigned inChans = inputUnit->GetNumChans();
    
    if (inChans == numChans)
    {
        for (i=0; i<numChans; i++)
            outputSamples[i][sNo] = in[i][sNo];
        return;
    }
    
    if ((inChans==1) && (numChans==2))
    {
        for (i=0; i<numChans; i++)
            outputSamples[i][sNo] = in[0][sNo];
        return;
    }
    
    if ((inChans==2) && (numChans==1))
    {
        outputSamples[0][sNo] = (in[0][sNo] + in[1][sNo]) * 0.5;
        return;
    }
}

void Unit::Bypass(void)
{
	unsigned i, j;
    
    if (inputUnit == nullptr)
    {
        for (i=0; i<numChans; i++)
            for (j=0; j<bufferSize; j++)
                outputSamples[i][j] = 0.0;
        return;
    }
    
	double** in      = inputUnit->OutputSamples();
	unsigned inChans = inputUnit->GetNumChans();
	
	if (inChans == numChans)
    {
		for (i=0; i<numChans; i++)
			for (j=0; j<bufferSize; j++)
				outputSamples[i][j] = in[i][j];
		return;
	}
	
	if ((inChans==1) && (numChans==2))
    {
		for (i=0; i<numChans; i++)
			for (j=0; j<bufferSize; j++)
				outputSamples[i][j] = in[0][j];
		return;
	}
		
	if ((inChans==2) && (numChans==1))
    {
		for (i=0; i<bufferSize; i++)
			outputSamples[0][i] = (in[0][i] + in[1][i]) * 0.5;
		return;
	}
}

double Unit::CheckRange(double sample)
{
	/* bash NANs and underflow/overflow hazards to zero (from Miller) */
	if (!((sample > 1.0e-20f && sample < 1.0e20f) || (sample < -1e-20f && sample > -1e20)))
		sample = 0.0;
	return sample;
}

void Unit::Clear(void)
{
	for (unsigned i=0; i<numChans; i++)
		for (unsigned j=0; j<bufferSize; j++)
			outputSamples[i][j] = 0.0;
}

double Unit::Clip(double sample)
{
	if (sample < -1.0) return -1.0;
	if (sample >  1.0) return  1.0;
	return sample; 
}

double Unit::GetSample(int s)
{
    while (s <  0)          s += bufferSize;
    while (s >= bufferSize) s -= bufferSize;
    return outputSamples[0][s];
}

double Unit::GetSampleXVolume(int s)
{
    while (s <  0)          s += bufferSize;
    while (s >= bufferSize) s -= bufferSize;
    return (outputSamples[0][s] * volume);
}

double Unit::GetSample(int c, int s)
{
    while (s <  0)          s += bufferSize;
    while (s >= bufferSize) s -= bufferSize;
    return outputSamples[c][s];
}

void Unit::GetOutputSamples(double* buffer)
{
	if (!active) return;

	int i;
	switch (numChans) {
		case 1: for (i=0; i<bufferSize; i++)
                {
                    ScaleVolume();
                    buffer[i] = Clip(outputSamples[0][i] * volume);
                }
                break;
		
		case 2:	for (i=0; i<bufferSize; i++)
                {
                    ScaleVolume();
                    buffer[i] = Clip((outputSamples[0][i]+outputSamples[1][i]) * 0.5 * volume);
                }
                break;
	}
}

void Unit::GetOutputSamples(double** buffer, unsigned channels)
{
	if (!active) return;

	unsigned i, j;

	if (channels == numChans) {
		for (j=0; j<bufferSize; j++)
        {
			ScaleVolume();
			for (i=0; i<channels; i++)
				buffer[i][j] = Clip(outputSamples[i][j] * volume);
		}
		return;
	}
	
	if ((channels==1) && (numChans==2)) {
		for (i=0; i<bufferSize; i++)
        {
			ScaleVolume();
			buffer[0][i] = Clip((outputSamples[0][i]+outputSamples[1][i]) * 0.5 * volume);
		}
		return;
	}
	
	if ((channels==2) && (numChans==1)) {
		for (j=0; j<bufferSize; j++)
        {
			ScaleVolume();
			for (i=0; i<2; i++)
				buffer[i][j] = Clip(outputSamples[0][j] * volume);
		}
		return;
	}
}

void Unit::MixOutputSamples(double* buffer)
{
	if (!active) return;

	int i;
	switch (numChans) {
		case 1: for (i=0; i<bufferSize; i++)
                {
                    ScaleVolume();
                    buffer[i] = Clip(buffer[i] + (outputSamples[0][i] * volume));
                }
                break;
		
		case 2:	for (i=0; i<bufferSize; i++)
                {
                    ScaleVolume();
                    buffer[i] = Clip(buffer[i] + (((outputSamples[0][i]+outputSamples[1][i])*0.5) * volume));
                }
                break;
	}
}

void Unit::MixOutputSamples(double** buffer, unsigned channels)
{
	if (!active) return;

    unsigned i, j;

	if (channels == numChans) {
		for (j=0; j<bufferSize; j++)
        {
			ScaleVolume();
			for (i=0; i<channels; i++)
				buffer[i][j] = Clip(buffer[i][j] + (outputSamples[i][j] * volume));
		}
		return;
	}
	
	if ((channels==1) && (numChans==2)) {
		for (i=0; i<bufferSize; i++)
        {
			ScaleVolume();
			buffer[0][i] = Clip(buffer[0][i] + (((outputSamples[0][i]+outputSamples[1][i])*0.5) * volume));
		}
		return;
	}
	
	if ((channels==2) && (numChans==1)) {
		for (j=0; j<bufferSize; j++)
        {
			ScaleVolume();
			for (i=0; i<2; i++)
				buffer[i][j] = Clip(buffer[i][j] + (outputSamples[0][j] * volume));
		}
		return;
	}
}

int Unit::FrequencyToMidi(double freq)
{
    if ((freq>=0.0) && (freq<=5000.0))
        return round(12.0*log2(freq/440.0) + 69.0);
    else
        return -1;
}

double Unit::MidiToFrequency(int midiPitch)
{
	if ((midiPitch>=0) && (midiPitch<=119))
		return 440.0 * pow(2.0, (double)(midiPitch-69)/12.0);
	else
		return -1.0;
}

double Unit::VelocityToAmplitude(int velocity)
{
    const double exponent = 2.0; // You can adjust this exponent
    return pow(velocity / 127.0, exponent);
}

void Unit::SetFeedback(double f, int chan)
{
	if ((chan<0) || (chan>=2)) return;
	feedback[chan] = f;
}

void Unit::SetFeedback(double f)
{
	feedback[0] = f;
}

void Unit::SetInputUnit(class Unit* in)
{
	inputUnit    = in;
	bypass		 = false;
}

void Unit::SetInputUnit(class Unit* in, int chan)
{
	inputUnit    = in;
	inputChannel = chan;
	bypass		 = false;
}

void Unit::TurnOn(class Unit* in)
{
	active    = true;
	inputUnit = in;
	bypass    = false;
}

void Unit::TurnOn(double vol)
{
	active = true;
	volume = vol;
}

void Unit::ScaleVolume(void)
{
	if (unitInc == 0.0) return;

	volume     += unitInc;
	double diff = desiredVol - volume;
	if (diff < 0.0) diff = -diff;
	if (diff < 0.05)
    {
		volume  = desiredVol;
		unitInc = 0.0;
	}
}

void Unit::Sample(int sNo)
{
    if (!active) return;
    if (bypass) { Bypass(sNo); return; }
    for (unsigned i=0; i<numChans; i++)
        outputSamples[i][sNo] = 0.0;
}

void Unit::Update(void)
{
    for (int i=0; i<bufferSize; i++)
        Sample(i);
}

void Unit::DownFromHere(long duration, double to)
{
	double rampSamples = static_cast<double>(duration) / msPerSample;
	if (rampSamples <= 0.0)
	{
		SetVolume(to);
		return;				// avoid division by zero
	}
	double from  = GetVolume();
	double range = to - from;
	desiredVol   = to;
	unitInc      = range/rampSamples;
}

void Unit::VolumeLine(double from, long duration, double to)
{
	SetVolume(from);
    double rampSamples = static_cast<double>(duration) / msPerSample;
	if (rampSamples <= 0.0)
	{
		SetVolume(to);
		return;				// avoid division by zero
	}
	double range = to - from;
	desiredVol   = to;
	unitInc      = range/rampSamples;
}
