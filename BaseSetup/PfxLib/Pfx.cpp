#include "Pfx.hpp"

Pfx::Pfx(void) : score(nullptr), playing(false), mInputBuffer(nullptr), inputBuffer(nullptr)
{
    OSStatus err = Init(kAudioDeviceUnknown, kAudioDeviceUnknown);
    
    if (err)
    {
        fprintf(stderr,"Pfx ERROR: Cannot Init Pfx");
        exit(1);
    }
}

Pfx::Pfx(AudioDeviceID input, AudioDeviceID output) : score(nullptr), playing(false), mInputBuffer(nullptr), inputBuffer(nullptr)
{
    OSStatus err = Init(input, output);
    
    if (err)
    {
		fprintf(stderr,"Pfx ERROR: Cannot Init Pfx");
		exit(1);
	}
}

Pfx::~Pfx(void)
{
	Cleanup();
}

OSStatus Pfx::Init(AudioDeviceID input, AudioDeviceID output)
{
    OSStatus err = noErr;

    err = BuildInputUnit(input);
	checkErr(err);
	
	err = SetupGraph(output);
	checkErr(err);
	
	err = SetupBuffers();
	checkErr(err);
	
	err = AUGraphInitialize(mGraph);
	return err;
}

void Pfx::Cleanup(void)
{
	Stop();
									
    UInt32 i;
    if (inputBuffer)
    {
        for (i=0; i<inBufferChannels; i++)
            delete [] inputBuffer[i];
        delete [] inputBuffer;
        inputBuffer = nullptr;
    }

	if (mInputBuffer)
    {
		for (i=0; i<mInputBuffer->mNumberBuffers; i++)
			free(mInputBuffer->mBuffers[i].mData);
		free(mInputBuffer);
		mInputBuffer = 0;
	}
	
    for (i=0; i<kNumChans; i++)
        delete [] mixChannels[i];

	AUGraphClose  (mGraph);
	DisposeAUGraph(mGraph);
    AudioUnitUninitialize        (mInputUnit);
    AudioComponentInstanceDispose(mInputUnit);
}

OSStatus Pfx::Start(void)
{
	OSStatus err = noErr;

    if (playing) return err;
    
    if (score == nullptr)
    {
        printf("no score assigned to pfx");
        exit(1);
    }
    err = AudioOutputUnitStart(mInputUnit);
    checkErr(err);
    
    err = AUGraphStart(mGraph);
    checkErr(err);
    playing = true;
	return err;
}

OSStatus Pfx::Stop(void)
{
	OSStatus err = noErr;

    if (!playing) return err;

    err = AudioOutputUnitStop(mInputUnit);
    checkErr(err);
    
    err = AUGraphStop(mGraph);
    checkErr(err);
    playing = false;

    return err;
}

void Pfx::RouteAudio(void)
{
    for (int i=0; i<kNumChans; i++)
        for (int j=0; j<samplesPerChannel; j++)
            mixChannels[i][j] = 0.0;
    
    if ((score == nullptr) || (!playing)) return;
    score->RouteAudio(mixChannels);
}

OSStatus Pfx::SetOutputDeviceAsCurrent(AudioDeviceID out)
{
    UInt32 size  = sizeof(AudioDeviceID);
    OSStatus err = noErr;
    
    AudioObjectPropertyAddress theAddress = { kAudioHardwarePropertyDefaultOutputDevice,
                                              kAudioObjectPropertyScopeGlobal,
                                              kAudioObjectPropertyElementMaster };
	
	if (out == kAudioDeviceUnknown)
	{
		err = AudioObjectGetPropertyData(kAudioObjectSystemObject, &theAddress, 0, nullptr, &size, &out);
        checkErr(err);
	}
    outputDeviceID = out;
	
    err = AudioUnitSetProperty(mOutputUnit, kAudioOutputUnitProperty_CurrentDevice, kAudioUnitScope_Global, 0,
							  &outputDeviceID, sizeof(outputDeviceID));
							
	return err;
}

OSStatus Pfx::SetInputDeviceAsCurrent(AudioDeviceID in)
{
    UInt32 size = sizeof(AudioDeviceID);
    OSStatus err = noErr;
    
    AudioObjectPropertyAddress theAddress = { kAudioHardwarePropertyDefaultInputDevice,
                                              kAudioObjectPropertyScopeGlobal,
                                              kAudioObjectPropertyElementMaster };
	
	if (in == kAudioDeviceUnknown)
	{  
		err = AudioObjectGetPropertyData(kAudioObjectSystemObject, &theAddress, 0, nullptr, &size, &in);
		checkErr(err);
	}
    inputDeviceID = in;
	
    err = AudioUnitSetProperty(mInputUnit, kAudioOutputUnitProperty_CurrentDevice, kAudioUnitScope_Global, 0,
							  &inputDeviceID, sizeof(inputDeviceID));
	return err;
}

OSStatus Pfx::SetupGraph(AudioDeviceID out)
{
	OSStatus err = noErr;
	
    err = NewAUGraph(&mGraph);
	checkErr(err);

    err = AUGraphOpen(mGraph);
	checkErr(err);

    err = AddOutputNodeAndUnit();
	checkErr(err);
    
	err = SetOutputDeviceAsCurrent(out);
	checkErr(err);
	
    AURenderCallbackStruct output;
	output.inputProc       = OutputProc;
	output.inputProcRefCon = this;
	
    err = AudioUnitSetProperty(mOutputUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input,
                               0, &output, sizeof(output));
	return err;
}

OSStatus Pfx::AddOutputNodeAndUnit(void)
{
	OSStatus err = noErr;
    AudioComponentDescription outDesc;

    outDesc.componentType           = kAudioUnitType_Output;
    outDesc.componentSubType        = kAudioUnitSubType_DefaultOutput;
    outDesc.componentManufacturer   = kAudioUnitManufacturer_Apple;
    outDesc.componentFlags          = 0;
    outDesc.componentFlagsMask      = 0;
    
	err = AUGraphAddNode(mGraph, &outDesc, &mOutputNode);
	checkErr(err);
	
	err = AUGraphNodeInfo(mGraph, mOutputNode, nullptr, &mOutputUnit);
	checkErr(err);

	return err;
}

OSStatus Pfx::BuildInputUnit(AudioDeviceID in)
{
	OSStatus err = noErr;
    
    AudioComponent              comp;
    AudioComponentDescription   desc;
	
	desc.componentType          = kAudioUnitType_Output;
	desc.componentSubType       = kAudioUnitSubType_HALOutput;
	desc.componentManufacturer  = kAudioUnitManufacturer_Apple;
	desc.componentFlags         = 0;
	desc.componentFlagsMask     = 0;
	
    comp = AudioComponentFindNext(nullptr, &desc);
	if (comp == nullptr) exit (-1);
	
    err = AudioComponentInstanceNew(comp, &mInputUnit);
    checkErr(err);
    
	err = EnableInput();
	checkErr(err);
	
	err = SetInputDeviceAsCurrent(in);
	checkErr(err);
	
	err = InputCallbackSetup();
	checkErr(err);
	
	err = AudioUnitInitialize(mInputUnit);

	return err;
}

OSStatus Pfx::EnableInput(void)
{	
    UInt32   enableIO;
	OSStatus err = noErr;
	
	enableIO = 1;
	err      =  AudioUnitSetProperty(mInputUnit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Input,
								1, &enableIO, sizeof(enableIO));
	checkErr(err);
	
	enableIO = 0;
	err      = AudioUnitSetProperty(mInputUnit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Output,
							  0, &enableIO, sizeof(enableIO));
	return err;
}

OSStatus Pfx::InputCallbackSetup(void)
{
	OSStatus err = noErr;

    AURenderCallbackStruct input;
    input.inputProc       = InputProc;
    input.inputProcRefCon = this;
	
	err = AudioUnitSetProperty(mInputUnit, kAudioOutputUnitProperty_SetInputCallback, kAudioUnitScope_Global,
							  0, &input, sizeof(input));
	return err;
}

inline void MakeBufferSilent(AudioBufferList* ioData)
{
    for (UInt32 i=0; i<ioData->mNumberBuffers; i++)
        memset(ioData->mBuffers[i].mData, 0, ioData->mBuffers[i].mDataByteSize);
}

OSStatus Pfx::SetupBuffers(void)
{
	OSStatus err = noErr;

	UInt32 bufferSizeBytes;
    AudioStreamBasicDescription asbd, asbdOutput;
    UInt32  propertySize;
	Float64 rate;
    AudioObjectPropertyAddress theAddress;
    
    /* get number of input channels */
    propertySize = sizeof(asbd);
    err = AudioUnitGetProperty(mInputUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 1, &asbd, &propertySize);
    checkErr(err);
    numInputChannels = asbd.mChannelsPerFrame;
    
	/* get number of samples per channel per interrupt */
    propertySize = sizeof(samplesPerChannel);
	err = AudioUnitGetProperty(mInputUnit, kAudioDevicePropertyBufferFrameSize, kAudioUnitScope_Global, 0, &samplesPerChannel, &propertySize);
    checkErr(err);

    Unit::SetBufferSize(samplesPerChannel);
    AllocateInputBuffer();

    /* channelwise buffer size in bytes, assuming Float32 samples */
    bufferSizeBytes = samplesPerChannel * sizeof(Float32);

    /* get sampling rate from input device */
    theAddress   = { kAudioDevicePropertyNominalSampleRate, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
    propertySize = sizeof(Float64);
    rate         = 0.0;
    err          = AudioObjectGetPropertyData(inputDeviceID, &theAddress, 0, nullptr, &propertySize, &rate);
	checkErr(err);
    
    Unit::SetSampleRate(rate);

    /* get stream format of output bus on input unit */
    propertySize = sizeof(asbd);
    err          = AudioUnitGetProperty(mInputUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 1, &asbd, &propertySize);
    checkErr(err);
    
    /* make sure sampling rate and number of channels are consistent input through output of inputUnit */
    asbd.mSampleRate       = rate;
    asbd.mChannelsPerFrame = numInputChannels;

    /* set inputUnit's output bus stream format to match input bus*/
    propertySize = sizeof(asbd);
	err          = AudioUnitSetProperty(mInputUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 1, &asbd, propertySize);
	checkErr(err);
    
    /* get number of output channels */
    propertySize = sizeof(asbdOutput);
    err = AudioUnitGetProperty(mOutputUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 0, &asbdOutput, &propertySize);
    checkErr(err);
    numOutputChannels = asbdOutput.mChannelsPerFrame;
    
    /* make sure output sampling rate matches input */
    rate = Unit::GetSamplingRate();
    if (asbdOutput.mSampleRate != rate)
    {
        propertySize = sizeof(Float64);
        err = AudioObjectSetPropertyData(outputDeviceID, &theAddress, 0, nullptr, propertySize, &rate);
        checkErr(err);
    }
    	
    asbd.mSampleRate       = rate;
    asbd.mChannelsPerFrame = numOutputChannels;

    /* set outputUnit's input bus stream format to match output bus*/
    propertySize = sizeof(asbd);
	err          = AudioUnitSetProperty(mOutputUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &asbd, propertySize);
	checkErr(err);

    /* allocate and zero out mixChannels */
    UInt32 i, j;
    for (i=0; i<kNumChans; i++)
    {
        mixChannels[i] = new double[samplesPerChannel];
        for (j=0; j<samplesPerChannel; j++)
            mixChannels[i][j] = 0.0;
    }

    /* allocate and zero out mInputBuffer */
    propertySize = offsetof(AudioBufferList, mBuffers[0]) + (sizeof(AudioBuffer) * numInputChannels);
	mInputBuffer = (AudioBufferList *)malloc(propertySize);
	mInputBuffer->mNumberBuffers = numInputChannels;
	
	for (i=0; i<numInputChannels; i++)
    {
		mInputBuffer->mBuffers[i].mNumberChannels = 1;
		mInputBuffer->mBuffers[i].mDataByteSize   = bufferSizeBytes;
		mInputBuffer->mBuffers[i].mData           = malloc(bufferSizeBytes);
	}
    
    MakeBufferSilent(mInputBuffer);

    return err;
}

void Pfx::AllocateInputBuffer(void)
{
    UInt32 i, j;
    if (numInputChannels == 0)  { printf("no channels for input buffer"); exit(1); }
    inBufferChannels = numInputChannels;
    if (inBufferChannels > Unit::kMaxChans) inBufferChannels = Unit::kMaxChans;
    inputBuffer = new Float32*[inBufferChannels];
    for (i=0; i<inBufferChannels; i++)
        inputBuffer[i] = nullptr;
    if (samplesPerChannel == 0) { printf("no frames for input buffer");   exit(1); }
    for (i=0; i<inBufferChannels; i++)
    {
        inputBuffer[i] = new Float32[samplesPerChannel];
        for (j=0; j<samplesPerChannel; j++)
            inputBuffer[i][j] = 0.0;
    }
}

void Pfx::FillInputBuffer(AudioBufferList* a)
{
    if (!inputBuffer) return;
    unsigned chs = numInputChannels;
    if (chs > Unit::kMaxChans) chs = Unit::kMaxChans;

    for (UInt32 i=0; i<chs; i++)
    {
        Float32* store  = inputBuffer[i];
        Float32* source = (Float32*)a->mBuffers[i].mData;
        for (UInt32 j=0; j<samplesPerChannel; j++)
            store[j] = source[j];
    }
}

OSStatus Pfx::InputProc(void* inRefCon, AudioUnitRenderActionFlags* ioActionFlags,
                        const AudioTimeStamp* inTimeStamp, UInt32 inBusNumber,
                        UInt32 inNumberFrames, AudioBufferList* ioData)
{
    OSStatus err  = noErr;
	Pfx*     This = (Pfx *)inRefCon;
		
	err = AudioUnitRender(This->mInputUnit, ioActionFlags, inTimeStamp, inBusNumber, inNumberFrames, This->mInputBuffer);
	checkErr(err);
    
    This->FillInputBuffer(This->mInputBuffer);
    
	return err;
}

OSStatus Pfx::OutputProc(void* inRefCon, AudioUnitRenderActionFlags* ioActionFlags,
                    const AudioTimeStamp* TimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames,
                    AudioBufferList* ioData)
{
    Pfx *This = (Pfx *)inRefCon;
    
    Float32* left  = (Float32*)ioData->mBuffers[0].mData;
    Float32* right = (Float32*)ioData->mBuffers[1].mData;
    
    This->RouteAudio();
    double** m    = This->GetMixChannels();
    double*  mixL = m[0];
    double*  mixR = m[1];
    
    for (UInt32 frame=0; frame<inNumberFrames; frame++)
    {
        left [frame] = (Float32)(mixL[frame]);
        right[frame] = (Float32)(mixR[frame]);
    }
    return noErr;
}

