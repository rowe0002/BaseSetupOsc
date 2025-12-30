#ifndef __CAPlayThrough_H__
#define __CAPlayThrough_H__

#include <CoreAudio/CoreAudio.h>
#include <AudioToolbox/AudioToolbox.h>
#include <AudioUnit/AudioUnit.h>
#include "Score.hpp"

#define checkErr( err) \
if(err) {\
OSStatus error = static_cast<OSStatus>(err);\
fprintf(stdout, "Pfx Error: %ld ->  %s:  %d\n",  (long)error,\
__FILE__, \
__LINE__\
);\
fflush(stdout);\
return err; \
}

class Pfx
{
public:
    enum { chL = 0, chR, kNumChans };
    bool                playing;

private:
    AudioUnit           mInputUnit;
    AudioBufferList*    mInputBuffer;
    AudioDeviceID       inputDeviceID;
    AudioDeviceID       outputDeviceID;
    
    AUGraph             mGraph;
    AUNode              mOutputNode;
    AudioUnit           mOutputUnit;
    
    double*             mixChannels[kNumChans];
    Float32**           inputBuffer;
    UInt32              samplesPerChannel;
    UInt32              numInputChannels;
    UInt32              inBufferChannels;
    UInt32              numOutputChannels;
    Score*              score;
    
public:
    Pfx(void);
    Pfx(AudioDeviceID input, AudioDeviceID output);
    ~Pfx(void);
    
    void		Cleanup(void);
    OSStatus	Init(AudioDeviceID input, AudioDeviceID output);
    UInt32      GetInBufferChannels(void)  const { return inBufferChannels;      }
    double**    GetMixChannels(void)       const { return (double**)mixChannels; }
    UInt32      GetNumInputChannels (void) const { return numInputChannels;      }
    Float32**   GetInputBuffer(void)       const { return inputBuffer;           }
    UInt32      GetSamplesPerChannel(void) const { return samplesPerChannel;     }
    OSStatus	SetInputDeviceAsCurrent (AudioDeviceID in );
    OSStatus	SetOutputDeviceAsCurrent(AudioDeviceID out);
    void        SetScore(Score* s) { score = s; }
    OSStatus	Start(void);
    OSStatus	Stop(void);
    
private:
    OSStatus    AddOutputNodeAndUnit(void);
    void        AllocateInputBuffer(void);
    OSStatus    SetupGraph(AudioDeviceID out);
    
    OSStatus    BuildInputUnit(AudioDeviceID in);
    OSStatus    EnableInput(void);
    OSStatus    InputCallbackSetup();
    void        FillInputBuffer(AudioBufferList* a);
    void        RouteAudio(void);
    OSStatus    SetupBuffers(void);
    
    
    static OSStatus InputProc(void *inRefCon,
                              AudioUnitRenderActionFlags *ioActionFlags,
                              const AudioTimeStamp *inTimeStamp,
                              UInt32				inBusNumber,
                              UInt32				inNumberFrames,
                              AudioBufferList *		ioData);
    
    static OSStatus OutputProc(void *inRefCon,
                               AudioUnitRenderActionFlags *ioActionFlags,
                               const AudioTimeStamp *inTimeStamp,
                               UInt32				inBusNumber,
                               UInt32				inNumberFrames,
                               AudioBufferList *	ioData);
};

#endif //__CAPlayThrough_H__
