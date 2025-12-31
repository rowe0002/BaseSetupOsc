/*
 *  Envelopes.cpp
 *  Granular
 *
 *  Created by Robert Rowe on 11/9/06.
 *  Copyright 2006 by Robert Rowe. All rights reserved.
 *
 */

#include "Envelopes.hpp"
#include "Scheduler.hpp"
#include <math.h>

extern Scheduler* scheduler;

Envelopes::Envelopes(void) : ADSRaccum(0.0), ADSRsample(0), increment(1.0), index(0.0), duration(1000)
{
    numChans  = 1;
    tableMS   = 1000.0;
    tableSize = static_cast<unsigned int>(Unit::samplingRate);
    ADSRphase = 0;
    eType     = kADSR;
    completed = false;
    if (tableSize == 0) tableSize = 44100;
    points = new double[tableSize];      // reference envelope one second long
    Attack();
}

Envelopes::~Envelopes(void) { delete [] points; }

/*
 Morph between two envelopes and place in env buffer.  Note that no FFT envelopes are
 allowed as inputs
*/
void Envelopes::AmplitudeMorph(int startType, int endType)
{
	int i;
	double* Q		 = new double[tableSize];
	int envelopeType = startType;

    for (i=0; i<2; i++)
	{
		switch(envelopeType)
		{
		  case 1: Gaussian	   (Q, tableSize); break;
		  case 2: Triangle	   (Q, tableSize); break;
		  case 3: Square	   (Q, tableSize); break;
		  case 4: Attack	   (Q, tableSize); break;
		  case 5: Sine		   (Q, tableSize); break;
		  case 6: ReverseAttack(Q, tableSize); break;
		  case 7: Hexagon	   (Q, tableSize); break;
		  case 8: M			   (Q, tableSize); break;
		}
		if (i == 0)
			for (i=0; i<tableSize; i++)
				points[i] = Q[i];
		envelopeType = endType;
	}
	for (i=0; i<tableSize; i++)
		points[i] = Q[i] + (points[i]-Q[i]);

	delete [] Q;
}  

/*
 This envelope is created by combining the rising half of a gaussian with
 a small standard deviation with the falling half of a gaussian with
 a large standard deviation.  The result is a smooth curve that has a
 quick attack and a long decay.  Note that since a Gaussian curve
 asymptotically approaches zero, this function will never equal zero.
 Its integral will approximately be 1.
*/
void Envelopes::Attack(void) { Attack(points, tableSize); }

void Envelopes::Attack(double* env, double len)
{
    int i;
    const double e = 2.71828;
    double mean    = (double)(len/12);
    double SD      = (double)(len/32);

    for (i=0; i<(int)(mean); i++)
        env[i] = pow(e, -(pow(fabs(i-mean),2.0))/(2.0*pow(SD,2.0)));
    SD = (double)(len/3.5);
    for (i=(int)(mean); i<len; i++)
        env[i] = pow(e, -(pow(fabs(i-mean),2.0))/(2.0*pow(SD,2.0)));
}

/*
 This envelope is a simple gaussian curve that includes 3 standard deviations
 from the mean over the length.  Its ends will approach zero, but this function
 will never equal zero. The factor allows you to scale the curve as desired.  The
 integral of the envelope will approximately equal the factor.
*/
void Envelopes::Gaussian(double* env, double length, double factor)
{
	double SD	   = (double)((length)/6);
	const double e = 2.71828; 
	double mean	   = (double)(length/2); 

	for (int i=0; i<length; i++)
		env[i] = pow(e, -(pow(fabs(i-mean),2.0))/(2.0*pow(SD, 2.0))) * factor;
}

/*
 See Gaussian above and note that this version sets the
 factor to 1.0, meaning the integral of the envelope over
 its length will approximately by equal to 1.
*/
void Envelopes::Gaussian(double* env, double length)
{
	Gaussian(env, length, 1.0);
}

/*
 Even though this envelope is called Hexagon, it can be 
 misleading.  It really creates a trapezoid in the buffer, 
 which if flipped updside down and glued to another will
 create a hexagon.  But, since there is no ability to
 negatively scale this envelope, it will never produce
 a hexagonal wave.  Instead, think of this just as a 
 regular trapezoid.
*/
void Envelopes::Hexagon(double* env, double length)
{
    int i;
	int endSlope = (int)(length/3);
	for (i=0; i<endSlope; i++)
		env[i] = (double)((double)(i)/(double)(endSlope));
	for (i=endSlope; i<(int)(length-endSlope); i++)
		env[i] = 1.0;
	for (i=(int)(length-endSlope); i<length; i++)
		env[i] = (double)((3.0-((double)(i)/(double)(endSlope))));
}
 
/*
 This envelope creates an "M" shape but first rising from 0
 to 1 (linearly) over 1/3 the length of the buffer, then dropping
 to half its height over the next 1/6 of the buffer length, then
 rising back to 1 over the next 1/6, and finally falling back
 to 0 over its last 1/3.
*/
void Envelopes::M(double* env, double length)
{
    int i;
	int endSlope = (int)(length/3);
	for (i=0; i<endSlope; i++)
		env[i] = (double)((double)(i)/(double)(endSlope));
	for (i=endSlope; i<(int)(length/2); i++)
		env[i] = (double)(2.0-((double)(i)/(double)(endSlope)));
	for (i=(int)(length/2); i<(int)(length-endSlope); i++)
		env[i] = (double)(((double)(i)/(double)(endSlope))-(double)(1));
	for (i=(int)(length-endSlope); i<length; i++)
		env[i] = (double)(3.0-((double)(i)/(double)(endSlope)));
}

/*
 This is the reverse envelope as described in the Attack function.  It fills
 a buffer with the rising enge of a gaussian with a large standard deviation
 and then appends to it the falling edge of a gaussian with a small standard
 distribution.  This creates a smooth curve with a long attack and quick 
 decay.
*/
void Envelopes::ReverseAttack(double* env, double length)
{
    int i;
	const double e = 2.71828;
	double mean	   = (double)((11*length)/12);
	double SD	   = (double)(length/3.5);

	for (i=0; i<(int)(mean); i++)
		env[i] = pow(e,-(pow(fabs(i-mean),2.0))/(2.0*pow(SD,2.0)));
	SD = (double)(length/32);
	for (i=(int)(mean); i<length; i++)
		env[i] = pow(e,-(pow(fabs(i-mean),2.0))/(2.0*pow(SD,2.0)));
}

/*
 This creates the first half of a sine wave over the 
 length of the (double) buffer.
*/
void Envelopes::Sine(double* env, double length)
{
	const double pi = 4.0 * atan(1.0);
	for (int i=0; i<length; i++)
	{
		env[i] = sin((double)(i)*(pi/length));
		if (env[i] > 1.0) env[i] = 1.0;
	}
}

/*
 This envelope function creates the top half of a 
 pseudo-square wave, where the rising and falling edges
 are have slopes of 8/length and -8/length respectively 
 (instead of the theoretical infinity and -infinity).
*/
void Envelopes::Square(double* env, double length)
{
    int i;
	int endSlope = (int)(length/8);
	for (i=0; i<endSlope; i++)
		env[i] = (double)((double)(i)/(double)(endSlope));

	for (i=endSlope; i<(int)(length-endSlope); i++)
		env[i] = 1.0;

	for (i=(int)(length-endSlope); i<length; i++)
		env[i] = (double)((8.0-((double)(i)/(double)(endSlope))));
}

/*
 The first half of a triangle wave is created by filling
 the buffer with values that rise linearly to 1 over half
 of the buffer size and then that fall linearly to 0 over 
 the second half.
*/
void Envelopes::Triangle(double* env, double length)
{
	double	 i    = 0.0;
	double	 mean = length / 2.0;
	unsigned inc;
	for (inc=0; i<=mean; inc++)
	{
		env[inc] = i / mean;
		i		+= 1.0;
	}

	i = mean + 1.0;
	for (inc=mean+1; inc<length; inc++)
	{
		env[inc] = 2.0 - (i/mean);
		i		+= 1.0;
	}
}

/*
 A Hann window function for use with a FFT. 
*/
void Envelopes::Hann(float* env, float length)
{	
	const float PI = 4.0 * atan(1.0);
	for (int i=0; i<length; i++)
	{
		env[i] = 0.5 * (1 - cos((2*PI*i)/(length - 1)));
		if (env[i] > 1.0) env[i] = 1.0;
	}
}

void Envelopes::SetEnvelope(int type, double* env, unsigned envLen)
{
    switch (type)
    {
        case 0: Attack       (env, envLen); break;
        case 1: Gaussian     (env, envLen); break;
        case 2: Hexagon      (env, envLen); break;
        case 3: M            (env, envLen); break;
        case 4: ReverseAttack(env, envLen); break;
        case 5: Sine         (env, envLen); break;
        case 6: Square       (env, envLen); break;
        case 7: Triangle     (env, envLen); break;
        default: break;
    }
}

void Hit(void* args)
{
    Envelopes* e = static_cast<Envelopes*>(args);
    e->index = 0.0;
    e->TurnOn();
}

#include <iostream>

void Envelopes::calculateADSRParams(double duration, double attackPct, double decayPct, double releasePct, double sustainLevel)
{
    double samplesPerMS = Unit::samplingRate / 1000.0;
    // Calculate the duration of each phase in samples
    ADSRp.attackSamples  = static_cast<int>(duration * attackPct  * samplesPerMS);
    ADSRp.decaySamples   = static_cast<int>(duration * decayPct   * samplesPerMS);
    ADSRp.releaseSamples = static_cast<int>(duration * releasePct * samplesPerMS);
    ADSRp.sustainSamples = static_cast<int>(duration * samplesPerMS) - (ADSRp.attackSamples + ADSRp.decaySamples + ADSRp.releaseSamples);

    // Calculate increments/decrements for each phase
    ADSRp.attackIncrement = 1.0 / ADSRp.attackSamples;
    ADSRp.attackTarget    = 1.0; // Target amplitude of attack phase is always 1.0

    // For decay, we want to go from 1.0 to sustainLevel
    ADSRp.decayIncrement = (sustainLevel - 1.0) / ADSRp.decaySamples;

    // Sustain level is constant, so no increment needed
    ADSRp.sustainLevel = sustainLevel;

    // For release, we decrease from sustainLevel to 0
    ADSRp.releaseIncrement = -sustainLevel / ADSRp.releaseSamples;
}

/* Fire: schedules envelope attacks */
void Envelopes::Fire(long onset)
{
    // if the onset time is zero, fire immediately
    if (onset == 0L)
        Hit(static_cast<void*>(this));
    else
        // otherwise schedule event later
        if (scheduler)
            scheduler->ScheduleTask(onset, 0, Hit, static_cast<void*>(this));
}

/* Fire: schedules envelope attacks */
void Envelopes::Fire(long onset, double duration)
{
    increment = ((static_cast<double>(tableSize)/samplingRate)*1000.0)/duration;
    // if the onset time is zero, fire immediately
    if (onset == 0L)
        Hit(static_cast<void*>(this));
    else
        // otherwise schedule event later
        if (scheduler)
            scheduler->ScheduleTask(onset, 0, Hit, static_cast<void*>(this));
}

void Envelopes::TurnOn(void)
{
    active     = true;
    completed  = false;
    ADSRaccum  = 0.0;
    ADSRphase  = 0;
    ADSRsample = 0;
    eType      = kADSR;
}

void Envelopes::TurnOn(envType e)
{
    active     = true;
    completed  = false;
    ADSRaccum  = 0.0;
    ADSRphase  = 0;
    ADSRsample = 0;
    eType      = e;
}

void Envelopes::Sample(int sNo)
{
    for (int i=0; i<numChans; i++)
        outputSamples[i][sNo] = 0.0;
    
    if (!active)   return;
    if (completed) return;

    double gain = 1.0;
    if (eType == kShapes)
    {
        double pt1 = points[(int)index];
        double i2  = index+1.0;
        double pt2 = 0.0;
        if (i2 >= tableSize)
        {
            TurnOff();
            completed = true;
            index     = 0.0;
        }
        else
        {
            pt2 = points[(int)i2];
            index += increment;
            if (index >= tableSize)
            {
                TurnOff();
                index = 0.0;
            }
        }
        gain = pt1 + ((index - ((int)index)) * (pt2 - pt1));
    } else  // eType == kADSR
    {
        switch (ADSRphase) {
            case 0: // Attack
                ADSRaccum += ADSRp.attackIncrement;
                if (ADSRsample++ >= ADSRp.attackSamples) {
                    ADSRphase++;
                    ADSRaccum  = ADSRp.attackTarget; // Ensure we reach the exact target
                    ADSRsample = 0;
                }
                break;
            case 1: // Decay
                ADSRaccum += ADSRp.decayIncrement;
                if (ADSRsample++ >= ADSRp.decaySamples) {
                    ADSRphase++;
                    ADSRaccum  = ADSRp.sustainLevel; // Ensure we reach sustain level
                    ADSRsample = 0;
                }
                break;
            case 2: // Sustain
                ADSRaccum = ADSRp.sustainLevel;     // Sustain level is constant
                if (ADSRsample++ >= ADSRp.sustainSamples) {
                    ADSRphase++;
                    ADSRsample = 0;
                }
                break;
            case 3: // Release
                ADSRaccum += ADSRp.releaseIncrement;
                if (ADSRsample++ >= ADSRp.releaseSamples) {
                    ADSRphase++;      // Indicates the envelope is complete
                    ADSRaccum  = 0.0; // Ensure we end at silence
                    ADSRsample = 0;
                }
                break;
            default:
                completed = true;
                ADSRaccum = 0.0;    // End of envelope
                break;
        }
        gain = ADSRaccum;
    }

    outputSamples[0][sNo] = gain;
}
