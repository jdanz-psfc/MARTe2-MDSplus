/**
 * @file ConsoleOut.cpp
 * @brief Source file for class ConsoleOut
 * @date 11/08/2017
 * @author Andre' Neto
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

 * @details This source file contains the definition of all the methods for
 * the class ConsoleOut (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/
#include "AdvancedErrorManagement.h"
#include "CLASSMETHODREGISTER.h"
#include "StreamOut.h"
#include <mdsobjects.h>
#include <stdio.h>
/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/
namespace MARTe {
static const int32 FILE_FORMAT_BINARY = 1;
static const int32 FILE_FORMAT_CSV = 2;

StreamOut::StreamOut() :
        DataSourceI(),
        MessageI() {
	dataSourceMemory = NULL_PTR(char8 *);
	offsets = NULL_PTR(uint32 *);
	streamBuffers = NULL_PTR(float32 **);
        numElements = NULL_PTR(uint32 *);
        numSamples = NULL_PTR(uint32 *);
	channelNames = NULL_PTR(StreamString *);
	counter = 0;
	pulseNumber = 0;
	nOfSignals = 0;
	timeStreaming = 1;
        numberOfBuffers = 0;
        cpuMask = 0xfu;
        stackSize = 0u;
	sigTypes = NULL_PTR(TypeDescriptor *);

}

/*lint -e{1551} -e{1579} the destructor must guarantee that the memory is freed and the file is flushed and closed.. The brokerAsyncTrigger is freed by the ReferenceT */
StreamOut::~StreamOut() {
    if (dataSourceMemory != NULL_PTR(char8 *)) {
        GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(dataSourceMemory));
    }
    if (offsets != NULL_PTR(uint32 *)) {
        GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *& >(offsets));
    }
    if (streamBuffers != NULL_PTR(float32 **))
    {
      for(uint32 i = 0; i < nOfSignals; i++) {
        GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *& > (streamBuffers[i]));
      }
      GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *& >(streamBuffers));
    }
    if(numElements != NULL_PTR(uint32 *))
      GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *& >(numElements));

    if(numSamples != NULL_PTR(uint32 *))
      GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *& >(numSamples));
    
    if(sigTypes != NULL_PTR(TypeDescriptor *))
      delete [] sigTypes;
    
}

bool StreamOut::AllocateMemory() {
    return true;
}

uint32 StreamOut::GetNumberOfMemoryBuffers() {
    return 1u;
}

bool StreamOut::GetSignalMemoryBuffer(const uint32 signalIdx, const uint32 bufferIdx, void*& signalAddress) {
    bool ok = (dataSourceMemory != NULL_PTR(char8 *));
    if (ok) {
        /*lint -e{613} dataSourceMemory cannot be NULL here*/
        char8 *memPtr = &dataSourceMemory[offsets[signalIdx]];
        signalAddress = reinterpret_cast<void *&>(memPtr);
    }
    return ok;
}

const char8* StreamOut::GetBrokerName(StructuredDataI& data, const SignalDirection direction) {
    const char8* brokerName = "";
    if (direction == OutputSignals) {
            brokerName = "MemoryMapAsyncOutputBroker";
    }
    return brokerName;
}

bool StreamOut::GetInputBrokers(ReferenceContainer& inputBrokers, const char8* const functionName, void* const gamMemPtr) {
    return false;
}

bool StreamOut::IsSupportedBroker(const SignalDirection direction, const uint32 functionIdx, const uint32 functionSignalIdx, const char8* const brokerClassName)
{
     return true;
}

bool StreamOut::GetOutputBrokers(ReferenceContainer& outputBrokers, const char8* const functionName, void* const gamMemPtr) {
  
    bool ok = true;
//    ReferenceT<MemoryMapSynchronisedOutputBroker> brokerSynch("MemoryMapSynchronisedOutputBroker");
    ReferenceT<MemoryMapAsyncOutputBroker> brokerAsynch("MemoryMapAsyncOutputBroker");
    ok = brokerAsynch.IsValid();
    if (ok) {
//	ok = brokerAsynch->Init(OutputSignals, *this, functionName, gamMemPtr);
        ok = brokerAsynch->InitWithBufferParameters(OutputSignals, *this, functionName, gamMemPtr, numberOfBuffers,
                                                   cpuMask, stackSize);
    }
    if (ok) {
	ok = outputBrokers.Insert(brokerAsynch);
    }
   return ok;
}

bool StreamOut::Synchronise() {
    bool ok = true;
    uint32 n;

    if(timeStreaming)
    {
        for (n = 0u; (n < nOfSignals) && (ok); n++) {
	    TypeDescriptor type = sigTypes[n];
	    if(type == Float32Bit)
	    {
               for(uint32 sample = 0; sample < numSamples[n]; sample++)
	           streamBuffers[n][bufIdx * numSamples[n] + sample] = *reinterpret_cast<float32 *>(&dataSourceMemory[offsets[n]+sample*sizeof(float32)]);
	    }
	    else if (type == Float64Bit)
	    {

               for(uint32 sample = 0; sample < numSamples[n]; sample++)
		{
	           streamBuffers[n][bufIdx * numSamples[n] + sample] = (reinterpret_cast<float64 *>(&dataSourceMemory[offsets[n]]))[sample];
		}

	    }
	    else if (type == SignedInteger16Bit)
	    {

               for(uint32 sample = 0; sample < numSamples[n]; sample++)
	           streamBuffers[n][bufIdx * numSamples[n] + sample] = (reinterpret_cast<int16 *>(&dataSourceMemory[offsets[n]]))[sample];
	    }
	    else if (type == SignedInteger32Bit)
	    {

               for(uint32 sample = 0; sample < numSamples[n]; sample++)
	           streamBuffers[n][bufIdx * numSamples[n] + sample] = (reinterpret_cast<int32 *>(&dataSourceMemory[offsets[n]]))[sample];
	    }
	    else if (type == UnsignedInteger16Bit)
	    {

               for(uint32 sample = 0; sample < numSamples[n]; sample++)
	           streamBuffers[n][bufIdx * numSamples[n] + sample] = (reinterpret_cast<uint16 *>(&dataSourceMemory[offsets[n]]))[sample];
	    }
	    else if (type == UnsignedInteger32Bit)
	    {

               for(uint32 sample = 0; sample < numSamples[n]; sample++)
	           streamBuffers[n][bufIdx * numSamples[n] + sample] = (reinterpret_cast<uint32 *>(&dataSourceMemory[offsets[n]]))[sample];
	    }
        }
        bufIdx = (bufIdx + 1)%bufSamples;
        counter++;
	if(bufIdx == 0)
        {
            float32 times[bufSamples*numSamples[timeIdx]];
            for(uint i = 0; i < bufSamples*numSamples[timeIdx]; i++)
		times[i] = streamBuffers[timeIdx][i]/1E6;
       
            for (n = 0u; (n < nOfSignals) && (ok); n++) {
	        if(n != timeIdx)
	        {
		   //   printf("Sending %d samples to channel %d  %s time: %f sample: %f\n", bufSamples * numSamples[n],n, channelNames[n].Buffer(), 
//			     times[0], streamBuffers[n][0]);
		    //  if (ok) MDSplus::EventStream::send(pulseNumber, channelNames[n].Buffer(), bufSamples*numSamples[n], times, streamBuffers[n]);

		    if(ok) streamManager.reportChannel(n,  bufSamples*numSamples[n], times, streamBuffers[n]);
		      
		      
		}
             }
             streamManager.sendAll(pulseNumber);
        } 
    }   
    else //Oscilloscope mode
    {
	if((bufIdx % eventDivision) == 0)
	{
            for (n = 0u; (n < nOfSignals) && (ok); n++) {
	        TypeDescriptor type = GetSignalType(n);
                float32 times[100];
                float32 eventBuffer[100];

  	        uint32 step = (numElements[n] / 100);
	        if((numElements[n] % 100) != 0)
		    step++;
	        uint32 currIdx = 0;
	        uint32 outIdx = 0;
	        while(currIdx < numElements[n])
	        {
		    float32 currSample = 0;
		    uint32 stepIdx = 0;
		    for(; stepIdx < step; stepIdx++)
		    {
		        if(currIdx >= numElements[n])
			    break;
	                if(type == Float32Bit)
	            	    currSample += (reinterpret_cast<float32 *>(&dataSourceMemory[offsets[n]]))[currIdx];
	                else if (type == Float64Bit)
	                {
	            	    currSample += (reinterpret_cast<float64 *>(&dataSourceMemory[offsets[n]]))[currIdx];
	                }
	                else if (type == SignedInteger16Bit)
	                {
	            	    currSample += (reinterpret_cast<int16 *>(&dataSourceMemory[offsets[n]]))[currIdx];
	                }
 	                else if (type == SignedInteger32Bit)
	                {
	              	    currSample += (reinterpret_cast<int32 *>(&dataSourceMemory[offsets[n]]))[currIdx];
	                }
	                else if (type == UnsignedInteger16Bit)
	                {
	            	    currSample += (reinterpret_cast<uint16 *>(&dataSourceMemory[offsets[n]]))[currIdx];
	                }
 	                else if (type == UnsignedInteger32Bit)
	                {
	            	    currSample += (reinterpret_cast<uint32 *>(&dataSourceMemory[offsets[n]]))[currIdx];
	                }
		        currIdx++;
		    }
		    eventBuffer[outIdx] = currSample / stepIdx;
            	    times[outIdx] = currIdx;
		    outIdx++;
		}
                StreamString signalName;
                ok = GetSignalName(n, signalName);

	        if (ok) MDSplus::EventStream::send(pulseNumber, signalName.Buffer(), outIdx, times, eventBuffer, true);
	    }
	}
	bufIdx++;
    }

    return ok;
}
 
 
 
 
/*lint -e{715}  [MISRA C++ Rule 0-1-11], [MISRA C++ Rule 0-1-12]. Justification: NOOP at StateChange, independently of the function parameters.*/
bool StreamOut::PrepareNextState(const char8* const currentStateName, const char8* const nextStateName) {
    return true;
}

bool StreamOut::Initialise(StructuredDataI& data) {
    bool ok = DataSourceI::Initialise(data);
    if (ok) {
        ok = data.Read("NumberOfBuffers", numberOfBuffers);
    }
    if (!ok) {
        REPORT_ERROR(ErrorManagement::ParametersError, "NumberOfBuffers shall be specified");
    }
    if (ok) {
        ok = (numberOfBuffers > 0u);
    }
    if (!ok) {
        REPORT_ERROR(ErrorManagement::ParametersError, "NumberOfBuffers shall be > 0u");
    }
    if (ok) {
        uint32 cpuMaskIn;
        ok = data.Read("CpuMask", cpuMaskIn);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "CpuMask shall be specified");
        }
        else {
            cpuMask = cpuMaskIn;
        }
    }
    if (ok) {
        ok = data.Read("StackSize", stackSize);
    }
    if (!ok) {
        REPORT_ERROR(ErrorManagement::ParametersError, "StackSize shall be specified");
    }
    if (ok) {
        ok = (stackSize > 0u);
    }
    if (!ok) {
        REPORT_ERROR(ErrorManagement::ParametersError, "StackSize shall be > 0u");
    }
    if (ok) {
        ok = data.Read("PulseNumber", pulseNumber);
	if(!ok)
	{
	  pulseNumber = 0;
	  ok = true;
	}
    }
    if(ok) {
        ok = data.Read("TimeIdx", timeIdx);
       if (!ok) {
            REPORT_ERROR(ErrorManagement::Information, "TimeIdx (index of time input) shall be specified");
        }
    }
    if(ok)
    {
	ok = data.Read("TimeStreaming", timeStreaming);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::Information, "TimeStreaming shall be specified");
        }
    }
    if(ok)
    {
        ok = data.Read("EventDivision", eventDivision);
        if(ok)
        {      
	    bufSamples = eventDivision;
	    if(bufSamples == 0)
	    {
	        bufSamples = 1;
	    }
	    if(eventDivision <= 0) 
		eventDivision = 1; 
        }
        else
        {
	    eventDivision = 1;
	    bufSamples = 1;
	    ok = true;
	    
        }
    }

    ok = data.MoveRelative("Signals");
    if(!ok) {
	REPORT_ERROR(ErrorManagement::ParametersError,"Signals node Missing.");
	return ok;
    }

    uint32 startIdx;
    if(timeStreaming)
	startIdx = 1; //Time signal has no channel associated
    else
	startIdx = 0;
    nOfSignals = data.GetNumberOfChildren();
    streamManager.init(nOfSignals);

     channelNames = new StreamString[nOfSignals];
     for (uint32 sigIdx = startIdx; sigIdx < nOfSignals; sigIdx++) {
      ok = data.MoveToChild(sigIdx);
      if(!ok) {
	  REPORT_ERROR(ErrorManagement::ParametersError,"Signals node %d has no child.", sigIdx);
	  return ok;
      }
      ok = data.Read("Channel", channelNames[sigIdx]);
      if(!ok) {
	  REPORT_ERROR(ErrorManagement::ParametersError,"Channel is missing (or is not a string) for signal %d.",sigIdx);
	  return ok;
      }
      
      
      streamManager.registerChannel(sigIdx, (char *)channelNames[sigIdx].Buffer());
      data.MoveToAncestor(1u);
    }
    data.MoveToAncestor(1u);

    return ok;
}

bool StreamOut::SetConfiguredDatabase(StructuredDataI& data) {
    bool ok = DataSourceI::SetConfiguredDatabase(data);
    //Check signal properties and compute memory
    
    if (ok) { // Check that only one GAM is Connected to the MDSReaderNS
        uint32 auxNumberOfFunctions = GetNumberOfFunctions();
        ok = (auxNumberOfFunctions == 1u);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "Exactly one Function allowed to interact with this StreamOut DataSource. Number of Functions = %u",
                         auxNumberOfFunctions);
        }
    }
    if (ok) { //read number of nodes per function numberOfNodeNames
        //0u (second argument) because previously it is checked

        ok = GetFunctionNumberOfSignals(OutputSignals, 0u, nOfSignals);        //0u (second argument) because previously it is checked
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "GetFunctionNumberOfSignals() returned false");
        }
	if(timeStreaming)
	{
            if (ok) {
                ok = (nOfSignals > 1u);
                if (!ok) {
                    REPORT_ERROR(ErrorManagement::ParametersError, "The number of signals must be at least 2");
                }
            }
	}
    }
    if (ok) {
        uint32 nOfInputSignals = GetNumberOfSignals();
	
	ok = (nOfInputSignals == nOfSignals); 
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError,
                         "nOfInputSignals %d must be equal than number %d of signals per function (since only one function can be connected to this data source)",
                         nOfInputSignals, nOfSignals);
        }
	if(!timeStreaming)
	    numElements = reinterpret_cast<uint32 *>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(nOfSignals * sizeof(int32)));
	else
	    numSamples = reinterpret_cast<uint32 *>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(nOfSignals * sizeof(int32)));

    }
    if (ok && !timeStreaming) {
        for (uint32 n = 0u; (n < nOfSignals) && ok; n++) {
            uint32 nSamples;
            ok = GetFunctionSignalSamples(OutputSignals, 0u, n, nSamples);
            if (ok) {
                ok = (nSamples == 1u);
            }
            if (!ok) {
                REPORT_ERROR(ErrorManagement::ParametersError, "The number of samples shall be exactly 1");
            }
        }
    }
    sigTypes = NULL_PTR(TypeDescriptor *);
    if (ok) { //read the type specified in the configuration file 
        sigTypes = new TypeDescriptor[nOfSignals];
        //lint -e{613} Possible use of null pointer. type previously allocated (see previous line).
        for (uint32 i = 0u; (i < nOfSignals) && ok; i++) {
            sigTypes[i] = GetSignalType(i);
            ok = !(sigTypes[i] == InvalidType);
            if (!ok) {
                REPORT_ERROR(ErrorManagement::ParametersError, "Invalid type");
            }
	    if (ok) { 
	      bool cond1 = (sigTypes[i] == UnsignedInteger64Bit);
	      bool cond2 = (sigTypes[i] == UnsignedInteger32Bit);
	      bool cond3 = (sigTypes[i] == UnsignedInteger16Bit);
	      bool cond4 = (sigTypes[i] == SignedInteger32Bit);
	      bool cond5 = (sigTypes[i] == SignedInteger64Bit);
	      bool cond6 = (sigTypes[i] == SignedInteger16Bit);
	      bool cond7 = (sigTypes[i] == Float32Bit);
	      bool cond8 = (sigTypes[i] == Float64Bit);
	      ok = cond1 || cond2 || cond3 || cond4 || cond5 || cond6 || cond7 || cond8;
	      if (!ok) {
                REPORT_ERROR(ErrorManagement::ParametersError, "Unsupported type. Possible time types are: uint64, int64, uin32 or int32\n");
	      }
	    }
	    else {
	      ok = false;
	    }
	}
    }
    if (ok) { //check number of elements
      for (uint32 i = 0u; (i < nOfSignals) && ok; i++) {
	uint32 numberOfElements;
	ok = GetSignalNumberOfElements(i, numberOfElements);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "Cannot read NumberOfElements");
	    return ok;
        }
	if(timeStreaming)
	{
            ok = GetFunctionSignalSamples(OutputSignals, 0u, i, numSamples[i]);
            if (!ok) {
                REPORT_ERROR(ErrorManagement::ParametersError, "Cannot read number of samples for signal %d ", i);
		return ok;
            }
            if(numSamples[i] == 1)
	    {
	          numSamples[i] = numberOfElements;
	    }
	    else
	    {
	      ok = (numberOfElements == 1);
              if (!ok) {
                REPORT_ERROR(ErrorManagement::ParametersError, "Samples and elements cannot be both > 1 for signal %d ", i);
		return ok;
              }
	    }
 	}	    
	else //Oscilloscope mode
	{
	    numElements[i] = numberOfElements;
	}
      }
    }
    if(ok && timeStreaming)
    {
      streamBuffers = reinterpret_cast<float32 **>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(nOfSignals * sizeof(float32 *)));
      for(uint32 i = 0; i < nOfSignals; i++)
	streamBuffers[i] = reinterpret_cast<float32 *>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(bufSamples * numSamples[i] * sizeof(float32)));
    }
 
    //lint -e{661} [MISRA C++ 5-0-16] Possible access out-of-bounds. nOfSignals is always 1 unit larger than numberOfNodeNames.
    if (ok) { //Count and allocate memory for dataSourceMemory, lastValue and lastTime
        offsets = new uint32[nOfSignals];
	//Count the number of bytes
        uint32 totalSignalMemory = 0u;
        if (sigTypes != NULL_PTR(TypeDescriptor *)) {
            if ((offsets != NULL_PTR(uint32 *)) ) {
	      
                for (uint32 i = 0u; (i < nOfSignals) && ok; i++) {
                    if (ok) { // count the time as well
		      offsets[i] = totalSignalMemory;
		      uint32 nBytes = 0u;
		      ok = GetSignalByteSize(i, nBytes);
		      if (!ok) {
                        REPORT_ERROR(ErrorManagement::ParametersError, "Error while GetSignalByteSize() for signal %u", i);
		      }
		      totalSignalMemory += nBytes * numSamples[i];
		    }
                }
            }
            else {
                ok = false;
	    }
	}
        else {
          ok = false;
	  }
        //Allocate memory
	if (ok) {
	   dataSourceMemory = reinterpret_cast<char8 *>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(totalSignalMemory));
	}
    }
    bufIdx = 0;


    return ok;
}




uint32 StreamOut::GetNumberOfBuffers() const {
    return 1;
}


CLASS_REGISTER(StreamOut, "1.0")
}

