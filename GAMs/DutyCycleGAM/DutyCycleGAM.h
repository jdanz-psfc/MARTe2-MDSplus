/**
 * @file ConstantGAM.h
 * @brief Header file for class ConstantGAM
 * @date Nov 9, 2016 TODO Verify the value and format of the date
 * @author aneto TODO Verify the name and format of the author
 *
 * @copyright Copyright 2015 F4E | European Joint Undertaking for ITER and
 * the Development of Fusion Energy ('Fusion for Energy').
 * Licensed under the EUPL, Version 1.1 or - as soon they will be approved
 * by the European Commission - subsequent versions of the EUPL (the "Licence")
 * You may not use this work except in compliance with the Licence.
 * You may obtain a copy of the Licence at: http://ec.europa.eu/idabc/eupl
 *
 * @warning Unless required by applicable law or agreed to in writing, 
 * software distributed under the Licence is distributed on an "AS IS"
 * basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the Licence permissions and limitations under the Licence.

 * @details This header file contains the declaration of the class ConstantGAM
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
 */

#ifndef ResamplerGAM_H_
#define ResamplerGAM_H_

/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/
#include "GAM.h"
#include "StructuredDataI.h"
#include "MessageI.h"
#include <math.h>
extern "C"  {
#include <filter.h>
}
/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/
/**************************************************************
DutyCycle GAM shall produce a clock of a given frequency and duty cycle in one bit of the output 32 bit word. 
Output clock frequency, specified in parameter OutFrequency, is constrained to be a divisor of the thread cycle frequency, 
specified in the parameter InFrequency. The duty cycle shall be specified in parameter DutyCycle.
The bit index of the output clock is specified by parameter ClockIdx. If EnableIdx is not -1, it shall be the index of a 
gate bit in the input word: clock will be generated when gate bit is on. Except for ClockIdx bit, the output word will be 
OutBits = (InBits AND AndMask) OR OrMask where AndMask and OrMask are configuration parameters

****************************************************************/
namespace MARTe {
class DutyCycleGAM : public GAM {
public:
    CLASS_REGISTER_DECLARATION()

    DutyCycleGAM();

    virtual ~DutyCycleGAM();

    virtual bool Setup();

    virtual bool Execute();

    virtual bool Initialise(StructuredDataI & data);

    int32 enableIdx;       //Position of enable signal in the second word (int32) digital input pattern 
    uint32 clockIdx;     //Position of output clock in the output word (int32)

    uint32 upClocks;	//Number of clock cycles with output high
    uint32 downClocks;	//Number of clock cycles with output down

    uint32 inClockCount;     //Current input clock count
    bool outClockIsHigh;  //Current output clock high

    uint32 andMask; //Digital I/O masks
    uint32 orMask;
  
    float32 inFrequency;
    float32 outFrequency;
    float32 dutyCycle;
    
    uint32 *inBits;
    uint32 *outBits;
};
}

/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif 
	
