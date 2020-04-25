
/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/
#include "AdvancedErrorManagement.h"
#include "CLASSMETHODREGISTER.h"
#include "StreamIn.h"
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

StreamIn::StreamIn() :
        DataSourceI(),
        MessageI() {
	dataSourceMemory = NULL_PTR(char8 *);
	offsets = NULL_PTR(uint32 *);
	bufIdxs = NULL_PTR(uint32 *);
	lastBufIdxs = NULL_PTR(uint32 *);
	streamBuffers = NULL_PTR(float32 **);
	bufElements = NULL_PTR(uint32 *);
	streamListeners = NULL_PTR(StreamListener **);

	nOfSignals = 0;
        cpuMask = 0xfu;
        stackSize = 0u;
	synchronizingIdx = -1;
	eventSem.Create();
	mutexSem.Create();
	numberOfBuffers = 0;

}

StreamIn::~StreamIn() {
//Free allocated buffers

    if (dataSourceMemory != NULL_PTR(char8 *)) {
        GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(dataSourceMemory));
    }
    if (offsets != NULL_PTR(uint32 *)) {
        GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *& >(offsets));
    }
    if (bufIdxs != NULL_PTR(uint32 *)) {
        GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *& >(bufIdxs));
    }
    if (lastBufIdxs != NULL_PTR(uint32 *)) {
        GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *& >(lastBufIdxs));
    }     
    if (bufElements != NULL_PTR(uint32 *)) {
        GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *& >(bufElements));
    }
    if (channelNames != NULL_PTR(StreamString *)) {
        delete []channelNames;
    }
    if (streamBuffers != NULL_PTR(float32 **))
    {
      for(uint32 i = 0; i < nOfSignals; i++) {
        GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *& > (streamBuffers[i]));
      }
      GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *& >(streamBuffers));
    }

    if (streamListeners != NULL_PTR(StreamListener **))
    {
      for(uint32 i = 0; i < nOfSignals; i++) {
        delete streamListeners[i];
      }
      GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *& >(streamListeners));
    }
}

bool StreamIn::AllocateMemory() {
    return true;
}

uint32 StreamIn::GetNumberOfMemoryBuffers() {
    return 1u;
}

bool StreamIn::GetSignalMemoryBuffer(const uint32 signalIdx, const uint32 bufferIdx, void*& signalAddress) {
    bool ok = (dataSourceMemory != NULL_PTR(char8 *));
    if (ok) {
        /*lint -e{613} dataSourceMemory cannot be NULL here*/
        char8 *memPtr = &dataSourceMemory[offsets[signalIdx]];
        signalAddress = reinterpret_cast<void *&>(memPtr);
    }
    return ok;
}

const char8* StreamIn::GetBrokerName(StructuredDataI& data, const SignalDirection direction) {
    const char8* brokerName = "";
    if (direction == InputSignals) {
            brokerName = "MemoryMapSynchronisedInputBroker";
    }
    return brokerName;
}

bool StreamIn::GetOutputBrokers(ReferenceContainer& inputBrokers, const char8* const functionName, void* const gamMemPtr) {
    return false;
}

bool StreamIn::IsSupportedBroker(const SignalDirection direction, const uint32 functionIdx, const uint32 functionSignalIdx, const char8* const brokerClassName)
{
     return true;
}

bool StreamIn::GetInputBrokers(ReferenceContainer& inputBrokers, const char8* const functionName, void* const gamMemPtr) {
  
    bool ok = true;

    ReferenceT<MemoryMapSynchronisedInputBroker> broker("MemoryMapSynchronisedInputBroker");
    ok = broker.IsValid();
    if (ok) {
        ok = broker->Init(InputSignals, *this, functionName, gamMemPtr);
    }

    if (ok) {
	ok = inputBrokers.Insert(broker);
    }
   return ok;
}

bool StreamIn::Synchronise() {
    bool ok = true;
    uint32 n;
    uint32 nOfSignals = GetNumberOfSignals();
    if (synchronizingIdx != -1)
	nOfSignals -= 1;  //If synchronizing, the last signal will be Time (in usecs)
    for (n = 0u; (n < nOfSignals) && (ok); n++) {
	TypeDescriptor type = GetSignalType(n);
        for(uint32 currEl = 0; currEl < bufElements[n]; currEl++)
        {
	    mutexSem.FastLock();
	    if((synchronizingIdx == (int32)n) && bufIdxs[n] == lastBufIdxs[n])
	    {         
	    	eventSem.Reset();
	    	mutexSem.FastUnLock();
	    	eventSem.Wait();
	    	mutexSem.FastLock();
	    }
	    if(type == Float32Bit)
	    {
	    	*reinterpret_cast<float32 *>(&dataSourceMemory[offsets[n]+currEl * sizeof(float32)]) = streamBuffers[n][bufIdxs[n]];
	    }
  	    if(type == Float32Bit)
	    {
	    	*reinterpret_cast<float32 *>(&dataSourceMemory[offsets[n]+currEl * sizeof(float32)]) = streamBuffers[n][bufIdxs[n]];
	    }
	    else if (type == Float64Bit)
	    {
	    	*reinterpret_cast<float64 *>(&dataSourceMemory[offsets[n]+currEl * sizeof(float32)]) = streamBuffers[n][bufIdxs[n]];
	    }
	    else if (type == SignedInteger16Bit)
	    {
	    	*reinterpret_cast<int16 *>(&dataSourceMemory[offsets[n]+currEl * sizeof(float32)]) = streamBuffers[n][bufIdxs[n]];
	    }
	    else if (type == SignedInteger32Bit)
	    {
	    	*reinterpret_cast<int32 *>(&dataSourceMemory[offsets[n]+currEl * sizeof(float32)]) = streamBuffers[n][bufIdxs[n]];
	    }
	    else if (type == UnsignedInteger16Bit)
	    {
	    	*reinterpret_cast<uint16 *>(&dataSourceMemory[offsets[n]+currEl * sizeof(float32)]) = streamBuffers[n][bufIdxs[n]];
	    }
	    else if (type == UnsignedInteger32Bit)
	    {
	    	*reinterpret_cast<uint32 *>(&dataSourceMemory[offsets[n]+currEl * sizeof(float32)]) = streamBuffers[n][bufIdxs[n]];
	    }
	    bufIdxs[n] = (bufIdxs[n] + 1)%(numberOfBuffers*bufElements[n]);
	    mutexSem.FastUnLock();
	}

    }
    if(synchronizingIdx != -1)  //If synchronizing, write time in us
    {
	*reinterpret_cast<uint32 *>(&dataSourceMemory[offsets[nOfSignals]]) = counter * (int)(period * 1E6);
    }
    counter++;

  return ok;
}
 
 
 
 
/*lint -e{715}  [MISRA C++ Rule 0-1-11], [MISRA C++ Rule 0-1-12]. Justification: NOOP at StateChange, independently of the function parameters.*/
bool StreamIn::PrepareNextState(const char8* const currentStateName, const char8* const nextStateName) {
    return true;
}

bool StreamIn::Initialise(StructuredDataI& data) {

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
            cpuMask = cpuMaskIn; //Not used for now
        }
    }
    if (ok) {
        ok = data.Read("StackSize", stackSize);
    }
    if (!ok) {
        REPORT_ERROR(ErrorManagement::ParametersError, "StackSize shall be specified");
    }
    if (ok) {
        ok = (stackSize > 0u);  //Not used for now
    }
    if (!ok) {
        REPORT_ERROR(ErrorManagement::ParametersError, "StackSize shall be > 0u");
    }
    if(ok) {
        ok = data.Read("SynchronizingIdx", synchronizingIdx);
       if (!ok) {
            REPORT_ERROR(ErrorManagement::Information, "SynchronizingIdx (index of synch input or -1) shall be specified");
        }
    }
    if(ok) {
        ok = data.Read("Period", period);
       if (synchronizingIdx != -1 && !ok) {
            REPORT_ERROR(ErrorManagement::Information, "Period shall be specified when SynchronizingIdx >= 0");
        }
    }

    ok = data.MoveRelative("Signals");
    if(!ok) {
	REPORT_ERROR(ErrorManagement::ParametersError,"Signals node Missing.");
	return ok;
    }
    uint32 nOfSignals = data.GetNumberOfChildren();
//Read Channel names 
    if(synchronizingIdx != -1)
	nOfSignals -= 1;
    channelNames = new StreamString[nOfSignals];
    for (uint32 sigIdx = 0; sigIdx < nOfSignals; sigIdx++) {
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
      data.MoveToAncestor(1u);
    }
    data.MoveToAncestor(1u);

   return ok;
}

bool StreamIn::SetConfiguredDatabase(StructuredDataI& data) {
    bool ok = DataSourceI::SetConfiguredDatabase(data);
    //Check signal properties and compute memory
    uint32 nOfSignals = 0u;  
    if (ok) { // Check that only one GAM is Connected to the MDSReaderNS
        uint32 auxNumberOfFunctions = GetNumberOfFunctions();
        ok = (auxNumberOfFunctions == 1u);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "Exactly one Function allowed to interact with this StreamIn DataSource. Number of Functions = %u",
                         auxNumberOfFunctions);
        }
    }
    if (ok) { //read number of nodes per function numberOfNodeNames
        //0u (second argument) because previously it is checked
        ok = GetFunctionNumberOfSignals(InputSignals, 0u, nOfSignals);        //0u (second argument) because previously it is checked
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "GetFunctionNumberOfSignals() returned false");
        }
	if (ok) {
            ok = ((synchronizingIdx >= 0) || !(nOfSignals > 1u));
            if (!ok) {
                REPORT_ERROR(ErrorManagement::ParametersError, "The number of signals must be at least 2 if SynchronizingIdx != -1");
            }
	}
    }

    bufElements = reinterpret_cast<uint32 *>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(nOfSignals * sizeof(int32)));
	bufIdxs  = reinterpret_cast<uint32 *>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(nOfSignals * sizeof(int32)));
    	lastBufIdxs = reinterpret_cast<uint32 *>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(nOfSignals * sizeof(int32)));
    if(ok)
    {
	for (uint32 n = 0u; (n < nOfSignals) && ok; n++) {
        {
	    if(ok)
	    {
		GetSignalNumberOfElements(n, bufElements[n]);
		if(!ok)  {
            	    REPORT_ERROR(ErrorManagement::ParametersError,
                         "Error getting number of elements for signal %d", n);
		}
	    }
	}
    }
    if(ok)
    {
      	streamBuffers = reinterpret_cast<float32 **>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(nOfSignals * sizeof(float32 *)));
      	for(uint32 i = 0; i < nOfSignals; i++)
	    streamBuffers[i] = reinterpret_cast<float32 *>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(numberOfBuffers * bufElements[i] * sizeof(float32)));
    }
    if (ok) {
        for (uint32 n = 0u; (n < nOfSignals) && ok; n++) {
            uint32 nSamples;
            ok = GetFunctionSignalSamples(InputSignals, 0u, n, nSamples);
            if (ok) {
                ok = (nSamples == 1u);
            }
            if (!ok) {
                REPORT_ERROR(ErrorManagement::ParametersError, "The number of samples shall be exactly 1");
            }
        }
    }
    TypeDescriptor *type = NULL_PTR(TypeDescriptor *);
    if (ok) { //read the type specified in the configuration file 
        type = new TypeDescriptor[nOfSignals];
        //lint -e{613} Possible use of null pointer. type previously allocated (see previous line).
        for (uint32 i = 0u; (i < nOfSignals) && ok; i++) {
            type[i] = GetSignalType(i);
            ok = !(type[i] == InvalidType);
            if (!ok) {
                REPORT_ERROR(ErrorManagement::ParametersError, "Invalid type");
            }
	    if (ok) { 
	      bool cond1 = (type[i] == UnsignedInteger64Bit);
	      bool cond2 = (type[i] == UnsignedInteger32Bit);
	      bool cond3 = (type[i] == UnsignedInteger16Bit);
	      bool cond4 = (type[i] == SignedInteger32Bit);
	      bool cond5 = (type[i] == SignedInteger64Bit);
	      bool cond6 = (type[i] == SignedInteger16Bit);
	      bool cond7 = (type[i] == Float32Bit);
	      bool cond8 = (type[i] == Float64Bit);
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

    if (ok) { //Count and allocate memory for dataSourceMemory, lastValue and lastTime
        offsets = new uint32[nOfSignals];
	//Count the number of bytes
        uint32 totalSignalMemory = 0u;
        if (type != NULL_PTR(TypeDescriptor *)) {
            if ((offsets != NULL_PTR(uint32 *)) ) {
	      
                for (uint32 i = 0u; (i < nOfSignals) && ok; i++) {
                    if (ok) { // count the time as well
		      offsets[i] = totalSignalMemory;
		      uint32 nBytes = 0u;
		      ok = GetSignalByteSize(i, nBytes);
		      if (!ok) {
                        REPORT_ERROR(ErrorManagement::ParametersError, "Error while GetSignalByteSize() for signal %u", i);
		      }
		      totalSignalMemory += nBytes;
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
	delete [] type;
        //Allocate memory
	if (ok) {
	   dataSourceMemory = reinterpret_cast<char8 *>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(totalSignalMemory));
	}
    }
 
//Instantiate listeners
    streamListeners = reinterpret_cast<StreamListener **>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(nOfSignals * sizeof(StreamListener *)));

    for (uint32 sigIdx = 0; sigIdx < nOfSignals - 1; sigIdx++) {
	streamListeners[sigIdx] = new StreamListener(sigIdx, streamBuffers, bufIdxs, lastBufIdxs, bufElements, &eventSem, &mutexSem, numberOfBuffers);

	evStream.registerListener(streamListeners[sigIdx], channelNames[sigIdx].Buffer());
	evStream.start();
	bufIdxs[sigIdx] = 0;
	lastBufIdxs[sigIdx] = 0;		
    }
  }
   counter = 0;
   return ok;
}




uint32 StreamIn::GetNumberOfBuffers() const {
    return 1;
}


void StreamListener::dataReceived(MDSplus::Data *samples, MDSplus::Data *times, int shot)
{
    if(bufElements[signalIdx] > 1)
    {
	std::vector<float> bufArr;
	try {
	    bufArr = samples->getFloatArray();
std::cout << "Received  " << samples << std::endl;
	} catch(MDSplus::MdsException &exc) {
	    printf("Exception issued when getting stream: %s", exc.what());
	}	    
	
	mutexSem->FastLock();
	for (uint32 el = 0; el < bufArr.size(); el++)
	{
	    streamBuffers[signalIdx][lastBufIdxs[signalIdx]] = bufArr[el];
	    lastBufIdxs[signalIdx] += 1;
	    if(lastBufIdxs[signalIdx] >= nOfBuffers * bufElements[signalIdx])
		lastBufIdxs[signalIdx] = 0;
	    if(lastBufIdxs[signalIdx] == bufIdxs[signalIdx])
	    {
	        printf("Overflow receiving data for channel %d",signalIdx);
	    }
	}
	eventSem->Post();
	mutexSem->FastUnLock();
    }
    else
    {
        float *bufSamples;
	float bufSample;
	int numSamples;
	char clazz, dtype;
	samples->getInfo(&clazz, &dtype);
	if(clazz == CLASS_A)
	{
	    try {
	    	bufSamples = samples->getFloatArray(&numSamples);
std::cout << "Received  " << samples << std::endl;
	    } catch(MDSplus::MdsException &exc) {
	    	printf("Exception issued when getting stream: %s", exc.what());
		return;
	    }
	}
	else
	{
	    try {
	        bufSample = samples->getFloat();
std::cout << "Received  " << bufSample << std::endl;
	        bufSamples = &bufSample;
	        numSamples = 1;
	    } catch(MDSplus::MdsException &exc) {
	        printf("Exception issued when getting stream: %s", exc.what());
	    } 
	} 
	mutexSem->FastLock();
	for(int sample = 0; sample < numSamples; sample++)
	{
	    streamBuffers[signalIdx][lastBufIdxs[signalIdx]] = bufSamples[sample];
	    lastBufIdxs[signalIdx] += 1;
	    if(lastBufIdxs[signalIdx] >= nOfBuffers)
	        lastBufIdxs[signalIdx] = 0;
	}
	if(clazz == CLASS_A)
	    delete[]bufSamples;
	eventSem->Post();
	mutexSem->FastUnLock();
	if(lastBufIdxs[signalIdx] == bufIdxs[signalIdx])
	    printf("Overflow receiving data for channel %d", signalIdx);

    }
}

CLASS_REGISTER(StreamIn, "1.0")
}

