/**
 * @file RTSMGAM.h
 * @brief Header file for class RTSMtGAM
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

#ifndef RTSMGAM_H_
#define RTSMGAM_H_

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
/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/
/**************************************************************
Resampler GAM will carry out resampling for an arbitrary number of inputs,
where every input is an array of arbitrary length. 
Supported types are float32, float64, int8, int16, int32, int64.
The device expects an equal number of outputs and every output will be of
dimension of the input divided by the resampling factor. The output types shall be the same of the corresponding inputs. 
An optional low pass filter can be enabled, filtering at half the resampled frequency.

****************************************************************/
namespace MARTe {


#define RTSM_MAX_STATES 16
#define RTSM_MAX_WAVES 8

/** Out mode definition */
#define OUT_MODE_ABS_TIME  1    //Waveform will be generated in respect to absolute time (i.e. time from start trigger)
#define OUT_MODE_REL_TIME  2    //Waveform will be generated in respect to the state time (t == 0 when state starts)
#define OUT_MODE_ACT_TIME  3    //Waveform will be generated in absolute time, considering however only the time this state has been active
#define OUT_MODE_JOIN 4         //Valid only when one state exists with mask == 0 and pattern == 0, used to "join" output level when the state enters and leaves 


  
  
  
  
class RTSMGAM : public GAM {

class  RTState {
public:
    uint32 numNext;
    uint32 nextIdxs[RTSM_MAX_STATES];
    uint32 nextMasks[RTSM_MAX_STATES];
    uint32 nextPatterns[RTSM_MAX_STATES];
    uint32 outBits;
    uint32 outWavesMode[RTSM_MAX_WAVES];
    uint32 outWavesXSamples[RTSM_MAX_WAVES];
    uint32 outWavesYSamples[RTSM_MAX_WAVES];
    float32 *outWavesX[RTSM_MAX_WAVES];
    float32 *outWavesY[RTSM_MAX_WAVES];
    float64 deadTime; 
    float64 startTime; //Time at which this state entered last time
    float64 prevStateTime; //Overall time spent in this state PREVIOUS entering this state if the state is the current one 

    float32 getWaveValue(uint32 waveIdx, float32 time);
  
};





public:
    CLASS_REGISTER_DECLARATION()

    RTSMGAM();

    virtual ~RTSMGAM();

    virtual bool Setup();

    virtual bool Execute();

    virtual bool Initialise(StructuredDataI & data);

private:
    int32 triggerIdx;    //Position of trigger signal in the digital input pattern (-1 to indicate that trigger is not used)

    uint32 numStates;
    uint32 numOutWaves;
    float64 *stateTime;                          //Minimum permanence time
    uint32 *stateNumNext;                         //Number of next states
    uint32 *stateOutBits;                  //Out bit pattern for every state
    uint32 *stateNext;            //Next states
    uint32 *stateNextMask;        //Next masks
    uint32 *stateNextPattern;    //NExt patterns.  
                                                //Rule for next enable: (Input & mask)==pattern 
                                                //mask == 0 and pattern == 0 mean always enabled 
    uint32 *stateOutMode;  //Output mode for waveforms

    float32 initialOut[RTSM_MAX_WAVES];
    float32 currOut[RTSM_MAX_WAVES];
    uint32 currStateIdx;
    RTState states[RTSM_MAX_STATES];

    uint32 prevBits;
    uint32 *inBits; 
    uint32 *outBits; 
    float32 *outWaves[RTSM_MAX_WAVES];


    bool isTriggered;
    int32 prevTime;
    float64 prevCycleTime;
    float32 triggerTime; //Assigned time to trigger (triggerIdx >= 0) or to first clock edge (triggerIdx == -1)
    float32 clockPeriod; //Used to reconstruct durrent time from triggerTime and numClockEdges;
    float32 currTime;  //Current time, valid after trigger
    uint64 cyclesFromTrigger;
    
    void step(uint32 inBits);
    void readVector(StructuredDataI & data, StreamString name, uint32 &nElements, float32 * &vals);
    bool hasData(StructuredDataI & data, StreamString name);
  
};
}

/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif 
	
