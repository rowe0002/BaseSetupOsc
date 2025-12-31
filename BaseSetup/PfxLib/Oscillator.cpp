//
//  Oscillator.cpp
//  Created by Robert Rowe on 5/5/24.
//

#include "Oscillator.hpp"
#include <cstddef>
#include <math.h>

Oscillator::Oscillator(void)
{
    numChans      = 1;
    frequency     = 0.0;
    increment     = 1.0;
    index         = 0.0;
    table         = nullptr;
    tableSize     = 0.0;
    FillTable(kSine, 8192);
    for (int i=0; i<bufferSize; i++)
        outputSamples[0][i] = 0.0;
    usingEnvelope = false;
}

Oscillator::~Oscillator(void)
{
    if (table != nullptr)
        delete [] table;
}

void Oscillator::FillTable(tableType type, long size)
{
    tableSize = (float)size;
    if (table != nullptr)
        delete [] table;
    table     = new double[size];
    ComputeTableSamples(type);
}

void Oscillator::FillTable(tableType type)
{
    if (tableSize <= 0.0) return;
    ComputeTableSamples(type);
}

void Oscillator::ComputeTableSamples(tableType type)
{
    int i;
    double twoPi = 8.0 * atan(1.0);
    double value;
    
    switch (type)
    {
        case kSine:
        {
            double piSize = twoPi/tableSize;
            for (i=0; i<tableSize; i++)
                table[i] = sin(piSize*i);
            break;
        }
        case kSawtooth:
        {
            double sawInc = 2.0/tableSize;
            value = 1.0;
            for (i=0; i<tableSize; i++)
            {
                table[i] = value;
                value   -= sawInc;
            }
            break;
        }
        case kRamp:
        {
            double rampInc = 2.0/tableSize;
            value = -1.0;
            for (i=0; i<tableSize; i++)
            {
                table[i] = value;
                value   += rampInc;
            }
            break;
        }
        case kSquare:
        {
            double halfSize = tableSize/2.0;
            for (i=0; i<halfSize; i++)
                table[i] = +0.95;
            for (; i<tableSize; i++)
                table[i] = -0.95;
            break;
        }
        case kTriangle :
        {
            double quarterInc = 1.0/tableSize*4.0;
            int limit = tableSize/4;
            value = 0.0;
            for (i=0; i<limit; i++)
            {
                table[i] = value;
                value   += quarterInc;
            }
            limit = i + tableSize/2;
            for (; i<limit; i++)
            {
                table[i] = value;
                value   -= quarterInc;
            }
            for (; i<tableSize; i++)
            {
                table[i] = value;
                value   += quarterInc;
            }
            break;
        }
        case kTest:
        {
            for (i=0; i<tableSize; i++)
                table[i] = (i%2)?1.0:0.0;
            break;
        }
    }
}

void Oscillator::SetFreq(int pitch)
{
    SetFreq(MidiToFrequency(pitch));
}

void Oscillator::SetFreq(double freq)
{
    // If the oscillator table is not initialized or the table size is zero, return without doing anything
    if (table == nullptr || tableSize == 0.0)
        return;

    // Set the oscillator's frequency
    frequency = freq;
    // Calculate the increment needed to advance to the next sample based on the new frequency and table size, assuming the global sampling rate
    increment = frequency * tableSize / samplingRate;
}

void Oscillator::TurnOn(double freq)
{
    if (table == nullptr || tableSize == 0.0)
        return;

    active    = true;
    frequency = freq;
    increment = frequency * tableSize / samplingRate;
}

void Oscillator::TurnOn(double freq, double rate)
{
    if (table == nullptr || tableSize == 0.0)
        return;

    active    = true;
    frequency = freq;
    increment = frequency * tableSize / rate;
}

void Oscillator::Sample(int sNo)
{
    if (!active || tableSize == 0.0)
        return;

    double pt1    = table[(int)index];
    double i2     = index+1.0;
    if (i2 >= tableSize)
        i2 -= tableSize;
    double pt2    = table[(int)i2];
    double output = pt1 + ((index - ((int)index)) * (pt2 - pt1));

    index += increment;

    while (index >= tableSize)
        index -= tableSize;
    while (index < 0.0)
        index += tableSize;
    
    if (usingEnvelope)
    {
        env->Sample(sNo);
        output *= env->GetSample(sNo);
    }

    outputSamples[0][sNo] = output * volume;
}
