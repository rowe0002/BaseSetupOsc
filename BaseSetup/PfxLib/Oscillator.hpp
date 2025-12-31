//
//  Oscillator.hpp
//  Created by Robert Rowe on 5/5/24.
//

#ifndef Oscillator_hpp
#define Oscillator_hpp

#include "Instrument.hpp"

class Oscillator : public Instrument
{
public:
    enum tableType { kSine, kSawtooth, kSquare, kRamp, kTriangle, kTest };

private:
    double  frequency;
    double  increment;
    double  index;
    double  tableSize;
    double* table;

public:
             Oscillator(void);
    virtual ~Oscillator(void);

    void     ComputeTableSamples(tableType type);
    void     FillTable(tableType type, long size);
    void     FillTable(tableType type);
    void     SetFreq(double freq)             override;
    void     SetFreq(int pitch)               override;
    void     TurnOn(double freq=440.0)        override;
    void     TurnOn(double freq, double rate) override;
    void     Sample(int sNo)                  override;
    void     ZeroPhase(void) { index = 0; }
};
#endif /* Oscillator_hpp */
