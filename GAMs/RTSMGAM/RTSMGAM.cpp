/**
 * @file FFTGAM.cpp
 * @brief Source file for class RTSMGAM
 * @date Jul 30, 2019
 * @author nn
 *

 * @details This source file contains the definition of all the methods for the
 * class FFTGAM (public, protected, and private). Be aware that some
 * methods, such as those inline could be defined on the header file, instead.
 */

#define DLL_API

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/

#include "RTSMGAM.h"
#include "AdvancedErrorManagement.h"
#include <iostream>
#include <stdio.h>
/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

namespace MARTe {

RTSMGAM::RTSMGAM() : GAM(){
    numStates = 0;
    numOutWaves = 0;
    stateTime = NULL_PTR(float64 *);                      
    stateNumNext = NULL_PTR(uint32 *);           
    stateOutBits = NULL_PTR(uint32 *);  
    stateNext = NULL_PTR(uint32 *);    
    stateNextMask = NULL_PTR(uint32 *);    
    stateNextPattern = NULL_PTR(uint32 *);  
    stateOutMode = NULL_PTR(uint32 *); 
}

RTSMGAM::~RTSMGAM() {
    if(numStates > 0)
    {
	GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(stateTime));
        GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(stateNumNext));
        GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(stateOutBits));
        GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(stateNext));
        GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(stateNextMask));
        GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(stateNextPattern));
        GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(stateOutMode));
        numStates = 0;
    }
}	

bool RTSMGAM::hasData(StructuredDataI & data, StreamString name)
{
    AnyType arrayDescr = data.GetType(name.Buffer());
    return (arrayDescr.GetDataPointer() != NULL_PTR(void *));
}
void RTSMGAM::readVector(StructuredDataI & data, StreamString name, uint32 &nElements, float32 * &vals)
{
    AnyType arrayDescr = data.GetType(name.Buffer());
 //No check required, perfoemed before by hasData()
    nElements = arrayDescr.GetNumberOfElements(0u);
    Vector<float32> currVect(nElements);
    data.Read(name.Buffer(), currVect);
    vals = new float32[nElements];
    for (uint32 i = 0; i < nElements; i++)
      vals[i] = currVect[i];
}

bool RTSMGAM::Initialise(StructuredDataI & data) {
	
	REPORT_ERROR(ErrorManagement::Debug, "RTSMGAM: INITIALIZE");
	
	bool ok = GAM::Initialise(data);
	if (!ok) {
		REPORT_ERROR(ErrorManagement::InitialisationError,
					 "Initialization failed.");
	}
	
	/**
	 * Firstly, number of inputs and outputs is read from the configuration file.
	 */
	
	if(ok) ok = data.MoveRelative("OutputSignals");
	if(!ok){
		REPORT_ERROR(ErrorManagement::ParametersError,
					 "Cannot find InputSignals node in configuration file.");
	}
	
	if(ok)
	{
	    uint32 numSignals = data.GetNumberOfChildren();
	    ok = (numSignals >= 1);
	    if(!ok){
		REPORT_ERROR(ErrorManagement::ParametersError,
					 "RTSM GAM Shall have at least one outpt signal (outBits)");
	    }
	    else
	    {
		numOutWaves = numSignals - 1;
	    }
	}
	
	// Back to the GAM node
	data.MoveToAncestor(1);

	if(ok) ok = data.MoveRelative("InputSignals");
	if(!ok){
		REPORT_ERROR(ErrorManagement::ParametersError,
					 "Cannot find InputSignals node in configuration file.");
	}
	
	if(ok)
	{
	    uint32 numSignals = data.GetNumberOfChildren();
	    ok = (numSignals == 1);
	    if(!ok){
		REPORT_ERROR(ErrorManagement::ParametersError,
					 "RTSM GAM Shall have exactly one input signal");
	    }
	}
	
	// Back to the GAM node
	data.MoveToAncestor(1);



        if (ok) {
            ok = data.Read("NumStates", numStates);
        }
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "Number of states shall be specified");
        }
        if(ok)
	{
            ok = numStates >0 && numStates <= 16;
            if (!ok) {
                REPORT_ERROR(ErrorManagement::ParametersError, "Number of states shall be between 1 and 16");
            }
	}
	
        if (ok) {
            ok = data.Read("TriggerIdx", triggerIdx);
	    if (!ok) {
		REPORT_ERROR(ErrorManagement::ParametersError, "Trigger Idx shall be specified");
	    }
        }
	
        if (ok) {
            ok = data.Read("TriggerTime", triggerTime);
	    if (!ok) {
		REPORT_ERROR(ErrorManagement::ParametersError, "Trigger time shall be specified");
	    }
        }
	
        if (ok) {
            ok = data.Read("Period", clockPeriod);
	    if (!ok) {
		REPORT_ERROR(ErrorManagement::ParametersError, "Frequency shall be specified");
	    }
        }
	
	for(uint32 stateIdx = 0; stateIdx < numStates; stateIdx++)
	{
	    StreamString stateName;
	    stateName.Printf("State%d", stateIdx + 1);
	    ok = data.MoveRelative(stateName.Buffer());
	    if(!ok){
		REPORT_ERROR(ErrorManagement::ParametersError,
					 "Cannot find State %d  in configuration file.", stateIdx + 1);
	    }
	    if(ok)
	    {
		ok = data.Read("DeadTime", states[stateIdx].deadTime);
	        if(!ok){
		    REPORT_ERROR(ErrorManagement::ParametersError,
					 "Cannot find DeadTime for State %d .", stateIdx + 1);
	        }
	    }
	    if(ok)
	    {
		ok = data.Read("OutBits", states[stateIdx].outBits);
	        if(!ok){
		    REPORT_ERROR(ErrorManagement::ParametersError,
					 "Cannot find OutBits for State %d .", stateIdx + 1);
	        }
	    }
	    for(uint32 waveIdx = 0; waveIdx < numOutWaves; waveIdx++)
	    {
		StreamString waveNameX;
		if(ok)
		{
		    waveNameX.Printf("Wave%d_x", waveIdx + 1);
		    ok = hasData(data, waveNameX);
		    if(!ok){
		        REPORT_ERROR(ErrorManagement::ParametersError,
					 "Cannot find X for State %d, Wave %d .", stateIdx + 1, waveIdx + 1);
		    }
		    if(ok)
		    {
			readVector(data, waveNameX, states[stateIdx].outWavesXSamples[waveIdx],
				   states[stateIdx].outWavesX[waveIdx]);
		    }
	        }
		if(ok)
		{
		    StreamString waveNameY;
		    waveNameY.Printf("Wave%d_y", waveIdx + 1);
		    ok = hasData(data, waveNameY);
		    if(!ok){
		        REPORT_ERROR(ErrorManagement::ParametersError,
					 "Cannot find Y for State %d, Wave %d .", stateIdx + 1, waveIdx + 1);
		    }
		    if(ok)
		    {
			readVector(data, waveNameY, states[stateIdx].outWavesYSamples[waveIdx],
				   states[stateIdx].outWavesY[waveIdx]);
		    }
	        }
		if(ok)
		{
		    StreamString waveNameMode;
		    waveNameMode.Printf("Wave%d_mode", waveIdx + 1);
		    ok = data.Read(waveNameMode.Buffer(), states[stateIdx].outWavesMode[waveIdx]);
		    if(!ok){
		        REPORT_ERROR(ErrorManagement::ParametersError,
					 "Cannot find mode for State %d, Wave %d .", stateIdx + 1, waveIdx + 1);
		    }
	        }
	    }
	    if(ok)
	    {
		ok = data.Read("NumNext", states[stateIdx].numNext);
	        if(!ok){
		    REPORT_ERROR(ErrorManagement::ParametersError,
					 "Cannot find DeadTime for State %d .", stateIdx + 1);
	        }
	    }
	    for(uint32 nextIdx = 0; nextIdx < states[stateIdx].numNext; nextIdx++)
	    {
		StreamString nextName;
		nextName.Printf("Next%d", nextIdx + 1);
		ok = data.MoveRelative(nextName.Buffer());
		if(!ok){
		  REPORT_ERROR(ErrorManagement::ParametersError,
					 "Cannot find Next state %d for State %d  in configuration file.", nextIdx+1, stateIdx + 1);
		}
		if(ok)
		{
		    ok = data.Read("Mask", states[stateIdx].nextMasks[nextIdx]);
		    if(!ok){
		      REPORT_ERROR(ErrorManagement::ParametersError,
					 "Cannot find mask for next  %d of State %d .", nextIdx + 1, stateIdx + 1);
		    }
		}
		if(ok)
		{
		    ok = data.Read("Pattern", states[stateIdx].nextPatterns[nextIdx]);
		    if(!ok){
		      REPORT_ERROR(ErrorManagement::ParametersError,
					 "Cannot find pattern for next  %d of State %d .", nextIdx + 1, stateIdx + 1);
		    }
		}
		if(ok)
		{
		    uint32 nextId;
		    ok = data.Read("State", nextId);
		    if(!ok){
		      REPORT_ERROR(ErrorManagement::ParametersError,
					 "Cannot find state for next  %d of State %d .", nextIdx + 1, stateIdx + 1);
		    }
		    		    if(ok)
		    {
			ok = (nextId >= 1 && nextId <= 16);
		        if(!ok){
		          REPORT_ERROR(ErrorManagement::ParametersError,
					 "Next state  %d of State %d shall be between 1 and 16.", nextIdx + 1, stateIdx + 1);
		        }
		    }
		    if(ok)
		    {
			states[stateIdx].nextIdxs[nextIdx] = nextId - 1;
		    }

		}
		data.MoveToAncestor(1u);
	    }
	    data.MoveToAncestor(1u);
        }
		
	return ok;
}

bool RTSMGAM::Setup() {
	
	bool ok;
	
	REPORT_ERROR(ErrorManagement::Debug, "SETUP");
	uint32 numEls;
	ok = GetSignalNumberOfElements(InputSignals, 0, numEls);
	if (!ok) {
	    REPORT_ERROR(ErrorManagement::Exception,  "Error in GetSignalNumberOfElements: Input signal InBits does not exist.");
	}
	if(ok)
	{
	    ok = (numEls == 1);
	}
	if(!ok){
	    REPORT_ERROR(ErrorManagement::Exception,  "Input signal InBits shall be a scalar");
	}
	if(ok)
	{
	    TypeDescriptor currType = GetSignalType(InputSignals, 0);
	    ok = (currType == SignedInteger32Bit || currType == UnsignedInteger32Bit);
	}
	if(!ok) {
	    REPORT_ERROR(ErrorManagement::Exception,  "Input signal InBits type shall either uint32 or int32");
	}
	ok = GetSignalNumberOfElements(OutputSignals, 0, numEls);
	if (!ok) {
	    REPORT_ERROR(ErrorManagement::Exception,  "Error in GetSignalNumberOfElements: output signal OutBits does not exist.");
	}
	if(ok)
	{
	    ok = (numEls == 1);
	}
	if(!ok){
	    REPORT_ERROR(ErrorManagement::Exception,  "Output signal OutBits shall be a scalar");
	}
	if(ok)
	{
	    TypeDescriptor currType = GetSignalType(OutputSignals, 0);
	    ok = (currType == SignedInteger32Bit || currType == UnsignedInteger32Bit);
	}
	if(!ok) {
	    REPORT_ERROR(ErrorManagement::Exception,  "Output signal OutBits type shall either uint32 or int32");
	}
	for(uint32 waveIdx = 0; waveIdx < numOutWaves; waveIdx++)
	{
	    ok = GetSignalNumberOfElements(OutputSignals, 1+waveIdx, numEls);
	    if (!ok) {
		REPORT_ERROR(ErrorManagement::Exception,  "Error in GetSignalNumberOfElements: output signal OutWave %d does not exist.", waveIdx);
	    }
	    if(ok)
	    {
		ok = (numEls == 1);
	    }
	    if(!ok){
		REPORT_ERROR(ErrorManagement::Exception,  "Output signal outWave %d shall be a scalar", waveIdx);
	    }
	    if(ok)
	    {
		TypeDescriptor currType = GetSignalType(OutputSignals, waveIdx+1);
		ok = (currType == Float32Bit);
	    }
	    if(!ok) {
		REPORT_ERROR(ErrorManagement::Exception,  "Output signal OutWave %d type shall be float32", waveIdx);
	    }
	}

	inBits = reinterpret_cast<uint32 *>(GetInputSignalMemory(0));
	outBits = reinterpret_cast<uint32 *>(GetOutputSignalMemory(0));
	for (uint32 waveIdx = 0; waveIdx < numOutWaves; waveIdx++) 
	{
	    outWaves[waveIdx] = reinterpret_cast<float32 *>(GetOutputSignalMemory(waveIdx+1));
	}
	cyclesFromTrigger = 0;	
	return ok;
    }
    
    float32 RTSMGAM::RTState::getWaveValue(uint32 waveIdx, float32 time)
    {
	uint32 idx;
	uint32 numSamplesX = outWavesXSamples[waveIdx];
	uint32 numSamplesY = outWavesYSamples[waveIdx];
	uint numSamples = numSamplesX;
	if(numSamples > numSamplesY)
	    numSamples = numSamplesY;
	if(time < outWavesX[waveIdx][0] || time > outWavesX[waveIdx][numSamples - 1])
	    return 0;
	if(time == outWavesX[waveIdx][numSamples - 1])
	    return outWavesY[waveIdx][numSamples - 1];



	for(idx = 0; idx < numSamples - 1; idx++)
	{
	     if(time >= outWavesX[waveIdx][idx] && time < outWavesX[waveIdx][idx+1])
	         break;
	}
	float32 retVal = outWavesY[waveIdx][idx] + (time - outWavesX[waveIdx][idx])*(outWavesY[waveIdx][idx+1] - outWavesY[waveIdx][idx])/(outWavesX[waveIdx][idx+1] - outWavesX[waveIdx][idx]); 
      printf("time :%f , idx: %d, waveX[idx]: %f waveX[idx+1]: %f, waveY[idx]: %f waveY[idx+1]: %f  value: %f\n",
	time, idx, outWavesX[waveIdx][idx], outWavesX[waveIdx][idx+1], outWavesY[waveIdx][idx], outWavesY[waveIdx][idx+1],retVal);
	return retVal;
    }

void RTSMGAM::step(uint32 inBits)
{
        uint32 nextStateIdx;

        for(nextStateIdx = 0; nextStateIdx < states[currStateIdx].numNext; nextStateIdx++)
        {
    //No mask means that state transition occurs at the end of the dead time  && (inBits & states[currStateIdx].nextMasks[nextStateIdx]) != (prevBits & states[currStateIdx].nextMasks[nextStateIdx])
            if((states[currStateIdx].nextMasks[nextStateIdx] == 0 &&
                 (currTime - states[currStateIdx].startTime) > states[currStateIdx].deadTime)||
                 (states[currStateIdx].nextMasks[nextStateIdx] != 0 && prevBits != 0xFFFFFFFF && (inBits & states[currStateIdx].nextMasks[nextStateIdx]) == states[currStateIdx].nextPatterns[nextStateIdx]))
            {
                states[currStateIdx].prevStateTime += (currTime - states[currStateIdx].startTime);
		REPORT_ERROR(ErrorManagement::Debug, "STATE SWITCH: Previous state %d NEXT STATE %d In Bits %X  Prev Bits %X \n", currStateIdx+1, states[currStateIdx].nextIdxs[nextStateIdx]+1, inBits, prevBits);
                currStateIdx = states[currStateIdx].nextIdxs[nextStateIdx];
		REPORT_ERROR(ErrorManagement::Debug, "NEXT STATE OUT MODE: %d\n", states[currStateIdx].outWavesMode[0]);
                states[currStateIdx].startTime = currTime;
                for(int i = 0; i < RTSM_MAX_WAVES; i++)
                    initialOut[i] = currOut[i];
                break;
            }
        }
//Digital output
        *outBits = states[currStateIdx].outBits;
//Now currStateTime refers to the right state (possibly just changed)
//OutWave generation


       // printf(" %d\n", states[currStateIdx].outWavesMode[0]);
        for(uint32 waveIdx = 0; waveIdx < numOutWaves; waveIdx++)
        {
            switch (states[currStateIdx].outWavesMode[waveIdx])  {
                case OUT_MODE_ABS_TIME:
                    *outWaves[waveIdx] = currOut[waveIdx] = states[currStateIdx].getWaveValue(waveIdx, currTime);
                    break;
                case OUT_MODE_REL_TIME:
                    *outWaves[waveIdx] = currOut[waveIdx] = states[currStateIdx].getWaveValue(waveIdx, currTime - states[currStateIdx].startTime);
                    break;
                case OUT_MODE_ACT_TIME:
                    *outWaves[waveIdx] = currOut[waveIdx] = states[currStateIdx].getWaveValue(waveIdx, states[currStateIdx].prevStateTime + currTime - states[currStateIdx].startTime);
                    break;
                case OUT_MODE_JOIN:
// The first state of the next list and the dead time will be considered for the computation of the join waveform scaling. Other transitions are nevertheless possible.
                    float64 nextInitial;
                    if(states[states[currStateIdx].nextIdxs[0]].outWavesMode[waveIdx] == OUT_MODE_ABS_TIME)
                        nextInitial = states[states[currStateIdx].nextIdxs[0]].getWaveValue(waveIdx, states[currStateIdx].startTime + states[currStateIdx].deadTime);
                    else //OUT_MODE_ACT_TIME
                        nextInitial = states[states[currStateIdx].nextIdxs[0]].getWaveValue(waveIdx, states[states[currStateIdx].nextIdxs[0]].prevStateTime);
                    float64 normalizedOut = states[currStateIdx].getWaveValue(waveIdx, (currTime - states[currStateIdx].startTime)/states[currStateIdx].deadTime);
                    *outWaves[waveIdx] = initialOut[waveIdx] + (normalizedOut * (nextInitial - initialOut[waveIdx]));
                    break;
            }
        }
        prevBits = inBits & 0xfffffffe;
    }



bool RTSMGAM::Execute() {
	
        REPORT_ERROR(ErrorManagement::Debug, "EXECUTE");
	if((triggerIdx == -1  && !isTriggered) ||
               (triggerIdx >= 0 && (*inBits & (0x01 << triggerIdx)) && !isTriggered)) 
	{
	    REPORT_ERROR(ErrorManagement::Debug, "TRIGGER!");
            for(uint32 stateIdx = 0; stateIdx < numStates; stateIdx++)
            {
                states[stateIdx].prevStateTime = triggerTime;
                states[stateIdx].startTime = triggerTime;
 	    }
 	    cyclesFromTrigger = 0;
            isTriggered = true;
	} 
        if(isTriggered)
        {
            currTime = triggerTime + clockPeriod * cyclesFromTrigger; 
	    cyclesFromTrigger++;
        } 
	else
	    currTime = triggerTime; 
	
        *outBits = 0;   
        for(uint32 i = 0; i < numOutWaves; i++)
	    *outWaves[i] = 0;
            //Advance State Machine
        if(isTriggered)
	    step(*inBits);
	return true;
}


CLASS_REGISTER(RTSMGAM, "1.0")

} /* namespace MARTe */
