//
//  BaseSetup.hpp
//  BaseSetup
//
//  Created by Robert Rowe on 12/30/25.
//

#include "Score.hpp"
#include "Oscillator.hpp"

class BaseSetup : public Score
{
private:
    Oscillator* osc;
    
public:
    BaseSetup(void);
   ~BaseSetup(void);
    
    void RouteAudio(double** mixChannels);
};
