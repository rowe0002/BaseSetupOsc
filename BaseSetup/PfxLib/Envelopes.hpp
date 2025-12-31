/*
 *  Envelopes.hpp
 *
 *  Created by Robert Rowe on 11/9/06.
 *  Copyright 2006 by Robert Rowe. All rights reserved.
 *
 */

#pragma	once

#include "Unit.hpp"

struct ADSRParams
{
    int    attackSamples;
    double attackIncrement;
    double attackTarget;

    int    decaySamples;
    double decayIncrement;

    int    sustainSamples;
    double sustainLevel;

    int    releaseSamples;
    double releaseIncrement;
};

class Envelopes : public Unit
{
public:
    enum envType { kShapes, kADSR };
    bool           completed;
    double         index;
    double         tableMS;
    unsigned int   tableSize;

private:
    unsigned long  duration;
    envType        eType;
    double         increment;
    ADSRParams     ADSRp;
    double         ADSRaccum;
    int            ADSRphase;
    int            ADSRsample;
    double*        points;

public:
    Envelopes(void);
    virtual ~Envelopes(void);
    
    void	AmplitudeMorph(int startType, int endType);                 // morph between two envelopes and place results in env buffer
	void	Attack		  (void);								        // A quick attack (using a Gaussian with small SD)
    void    Attack        (double* env, double length);                 // followed by a long decay (using a Gaussian with large SD)
	void	Gaussian	  (double* env, double length);					// 3 SDs from the mean of a simple Gaussian
	void	Gaussian	  (double* env, double length, double factor);  // 3 SDs from the mean of a simple Gaussian scaled by the 'factor'
    void calculateADSRParams(double duration, double attackPct, double decayPct, double releasePct, double sustainLevel);
    void	Hexagon		  (double* env, double length);                 // A trapezoidal shape
	void	M			  (double* env, double length);                 // The shape of an 'M'
	void	ReverseAttack (double* env, double length);                 // Produces the opposite of 'Attack' (see above)
	void	Sine		  (double* env, double length);                 // double version of half cycle of a sine wave
	void	Square		  (double* env, double length);                 // half cycle of a square wave
	void	Triangle	  (double* env, double length);                 // half cycle of a triangle wave
	/* Common Window Functions for FFT use */
	void	Hann		  (float* env,  float length);                  // Hann window

    void    Fire(long onset);
    void    Fire(long onset, double duration);
    double  GetPoint(unsigned int i) { return points[i]; }
    void    SetEnvelope(int type, double* env, unsigned envLen);
    void    SetEtype(envType e) { eType = e; }
    void    Sample(int sNo);
    void    TurnOn(void);
    void    TurnOn(envType e);
};
