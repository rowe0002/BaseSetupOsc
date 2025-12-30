/*
 *  Unit.hpp
 *
 *  Created by Robert Rowe on 9/29/06.
 *  Copyright 2006 Robert Rowe. All rights reserved.
 *
 */

#pragma  once

#include <vector>
using namespace std;

class Unit
{
public:
    enum channelType { kMono = 1, kStereo };    // A Unit can either be mono or stereo
    static unsigned int bufferSize;             // Size of buffer
    static const int    kMaxChans;              // Maximum number of channels
    unsigned int        channel;                // Which channel
    unsigned int        numChans;               // Unit's number of channels
	class Task*         volumeTask;             // Pointer to a volume task

protected:
	static double	 samplingRate;              // Sampling Rate
	
	bool             active;                    // Is this unit on?
	double           analysisValue;
	bool             bypass;                    // Should this unit be bypassed?
	double           controlRate;               // Buffer fills and empties per second
	double           desiredVol;                // the target volume
	class Unit*      effect;
	double           feedback[2];               // feedback value for each channel
	int              inputChannel;              // which channel input should feed
	class Unit*      inputUnit;                 // Unit that provides input
	double           msPerSample;               // ms per sample
	double**         outputSamples;             // pointer to Unit's buffer(s)
	double           unitInc;                   // An amount to increment every sample (used for various things)
	double           volume;                    // current Unit volume

public:
                   Unit();                      // Default Constructor - numChans initialized to stereo.  See Init() for more.
                   Unit(int chans);             // Should we make this 'explicit'? See Init() for more.
    virtual       ~Unit();                      // Destructor - clean up allocated buffers
	
	bool		   Active()			const  { return active;			}       // active accessor
	double		   AnalysisValue()	const  { return analysisValue;  }       // analysisValue accessor
	unsigned int   BufferSize()		const  { return bufferSize;		}       // bufferSize accessor
    void           Bypass(int sNo);
    void           SetBypass(bool onOff)   { bypass = onOff;        }       // set bypass to true or false

	double		   CheckRange(double sample);                               // bash NANs and underflow/overflow hazards to zero (from Miller Puckette)
	void		   Clear();                                                 // Set all buffer values to zero
	
	void		   DownFromHere(long duration, double to);                  //  Adjust volume from where it is now to 'to' over 'duration' ms.
	class Unit*	   Effect() const			{ return effect;		   }	// effect unit accessor
	unsigned       GetNumChans() const		{ return numChans;         }	// numChans accessor
	
	virtual void   GetOutputSamples(double* buffer);						// Place the contents of the Unit's buffer into the buffer passed in.
	virtual void   GetOutputSamples(double** buffer, unsigned channels);    // Place the contents of the Unit's buffer(s) into the buffer(s) pointed to by 'buffer'
    double         GetSample(int s);
    double         GetSampleXVolume(int s);
    double         GetSample(int c, int s);
    static double  GetSamplingRateMS(void)  { return samplingRate / 1000.0; }
    static double  GetSamplingRate(void)    { return samplingRate;     }
	double		   GetVolume(void) const	{ return volume;		   }	// volume accessor
	int			   InputChannel(void) const { return inputChannel;	   }	// inputChannel accessor
	class Unit*	   InputUnit(void)          { return inputUnit;        }	// inputUnit accessor
    bool           IsOn(void)               { return active;           }    // active accessor
	
    int            FrequencyToMidi(double freq);                            // convert frequency to corresponding MIDI note number
    double         VelocityToAmplitude(int velocity);
    double		   MidiToFrequency(int midiPitch);							// convert MIDI note number to corresponding frequency

	virtual void   MixOutputSamples(double* buffer);						// Add values in Unit's buffer to values currently in 'buffer'
	virtual void   MixOutputSamples(double** buffer, unsigned channels);    // Add values in Unit's buffer(s) to values currently in buffer(s) pointed to by 'buffer'
	double**	   OutputSamples(void)  const { return outputSamples;    }	// Access Unit's buffer's
	double*		   OutputSamples(int c) const { return outputSamples[c]; }	// Access a specific channel of the Unit's buffers
    virtual void   Sample(int sNo);
    void           SetActive(bool a)             { active     = a;       }  // set active
	static  void   SetBufferSize(unsigned int b) { bufferSize = b;       }  // bufferSize mutator
    void           SetChannel(unsigned int c)    { channel    = c;       }  // channel mutator
	void		   SetEffect(class Unit* e)      { effect     = e;       }  // effect unit mutator

	void		   SetFeedback(double f);                                   // set feedback of first channel to 'f'
	
	void		   SetFeedback(double f, int chan);							// set feedback of channel 'c' to 'f'
	void		   SetInputChannel(int c)     { inputChannel	= c;   }	// inputChannel mutator
	
	virtual void   SetInputUnit(class Unit* in);							// set input unit to 'in'
	virtual void   SetInputUnit(class Unit* in, int chan);					// set the input unit on channel 'chan' to 'in'
	void		   SetNumChans(int n)         { numChans		= n;   }	// numChans mutator
	static void	   SetSampleRate(double r)    { samplingRate	= r;   }	// samplingRate mutator
	void		   SetVolume(double newVol)   { volume		= newVol;  }	// volume mutator
	virtual void   TurnOn(void)               { active		= true;	   }	// Set active to true
	virtual void   TurnOn(long)               { active		= true;	   }	// Set active to true
	virtual void   TurnOn(double vol);										// Set active to true and set volume to 'vol'
	virtual void   TurnOn(double, double)     { active		= true;	   }	// Set active to true
	virtual void   TurnOn(class Unit* in);									// Set input unit to 'in' and set active to true
	virtual void   TurnOff(void)              { active		= false;   }	// Set active to false
    virtual void   Update(void);											// Sets Unit's buffer vals to 0.  This is typically overridden in derived classes.
	void		   VolumeLine(double from, long duration, double to);       // Adjust volume from 'from' to 'to' over 'duration' ms.

protected:
	void           Bypass(void);        // Send input directly to output
	double         Clip(double sample); // If samples are below -1 or above 1 set them to -1 and 1 respectively
	void           Init(void);          // Called on construction - create output buffer (one or two channels) and set member variables to reasonable defaults
	virtual void   ScaleVolume(void);
};
