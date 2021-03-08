
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
	synchStreamTime = NULL_PTR(float32 *);
	bufElements = NULL_PTR(uint32 *);
	streamListeners = NULL_PTR(StreamListener **);
        evStreams = NULL_PTR(MDSplus::EventStream **);
	nOfSignals = 0;
        cpuMask = 0xfu;
        stackSize = 0u;
	synchronizingIdx = -1;
	eventSem.Create();
	mutexSem.Create();
	numberOfBuffers = 0;
	period = 0;
	started = false;
	

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
    if (synchStreamTime != NULL_PTR(float32 *))
    {
      GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *& >(synchStreamTime));
    }

    if (streamListeners != NULL_PTR(StreamListener **))
    {
      for(uint32 i = 0; i < nOfSignals; i++) {
        delete streamListeners[i];
      }
      GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *& >(streamListeners));
    }
    if(evStreams != NULL_PTR(MDSplus::EventStream**))
    {
      for(uint32 i = 0; i < nOfSignals - 1; i++)
        delete evStreams[i];
      delete [] evStreams;
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
  
#ifdef DEBUG
    REPORT_ERROR(ErrorManagement::Debug, "Synchronise  SynchronizingIdx:%d bufElements[0]: %d  %d  %d", synchronizingIdx, bufElements[0], bufIdxs[0] , lastBufIdxs[0]);
#endif
  
    started = true;
    bool ok = true;
    if(counter == 0)
    {
        startCycleTicks = HighResolutionTimer::Counter();
    }
    else
    {
         periodGuess = (HighResolutionTimer::Counter() - startCycleTicks) * HighResolutionTimer::Period() / counter;
    }
    uint32 n;
    uint32 nOfSignals = GetNumberOfSignals();
    if (synchronizingIdx != -1)
        nOfSignals -= 1;  //If synchronizing, the last signal will be Time (in usecs)
    for (n = 0u; (n < nOfSignals) && (ok); n++) {
        TypeDescriptor type = GetSignalType(n);
        for(uint32 currEl = 0; currEl < bufElements[n]; currEl++)
        {
          mutexSem.FastLock();
            //If anarray is going to be received, synchronize it only at the first element
            if((synchronizingIdx == (int32)n) && currEl == 0)
            {         
                mutexSem.FastUnLock();
                eventSem.ResetWait(TTInfiniteWait);
                mutexSem.FastLock();
            }
            if(type == Float32Bit)
            {
                *reinterpret_cast<float32 *>(&dataSourceMemory[offsets[n]+currEl * sizeof(float32)]) = streamBuffers[n][currEl];
            }
            if(type == Float64Bit)
            {
                *reinterpret_cast<float64 *>(&dataSourceMemory[offsets[n]+currEl * sizeof(float64)]) = streamBuffers[n][currEl];
            }
            else if (type == Float64Bit)
            {
                *reinterpret_cast<float64 *>(&dataSourceMemory[offsets[n]+currEl * sizeof(float32)]) = streamBuffers[n][currEl];
            }
            else if (type == SignedInteger16Bit)
            {
                *reinterpret_cast<int16 *>(&dataSourceMemory[offsets[n]+currEl * sizeof(float32)]) = streamBuffers[n][currEl];
            }
            else if (type == SignedInteger32Bit)
            {
                *reinterpret_cast<int32 *>(&dataSourceMemory[offsets[n]+currEl * sizeof(float32)]) = streamBuffers[n][currEl];
            }
            else if (type == UnsignedInteger16Bit)
            {
                *reinterpret_cast<uint16 *>(&dataSourceMemory[offsets[n]+currEl * sizeof(float32)]) = streamBuffers[n][currEl];
            }
            else if (type == UnsignedInteger32Bit)
            {
                *reinterpret_cast<uint32 *>(&dataSourceMemory[offsets[n]+currEl * sizeof(float32)]) = streamBuffers[n][currEl];
            }
            if((synchronizingIdx == (int32)n) && (currEl == 0)) //If an array is received keep track only of the first time
            {
                if(period <= 0) //If period not defined take time information from synchronizing stream
                {
                    *reinterpret_cast<uint32 *>(&dataSourceMemory[offsets[nOfSignals]]) = synchStreamTime[0] * 1E6;
                }
                else
                {
                    *reinterpret_cast<uint32 *>(&dataSourceMemory[offsets[nOfSignals]]) = counter * (int)(period * 1E6);
                }
            }
            mutexSem.FastUnLock();
        }

    }
    counter++;
    
  return ok;
}
 
 
 
/*lint -e{715}  [MISRA C++ Rule 0-1-11], [MISRA C++ Rule 0-1-12]. Justification: NOOP at StateChange, independently of the function parameters.*/
bool StreamIn::PrepareNextState(const char8* const currentStateName, const char8* const nextStateName) {
    return true;
}

bool StreamIn::Initialise(StructuredDataI& data) {
    started = false;
    bool ok = DataSourceI::Initialise(data);
    if (ok) {
        ok = data.Read("NumberOfBuffers", numberOfBuffers);
	numberOfBuffers++; //An extra buffer is added to handle overfolw
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
    started = false;
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
            ok = (!(synchronizingIdx >= 0) || (nOfSignals > 1u));
            if (!ok) {
                REPORT_ERROR(ErrorManagement::ParametersError, "The number %d of signals must be at least 2 if SynchronizingIdx != -1", nOfSignals);
            }
	}
    }

    bufElements = reinterpret_cast<uint32 *>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(nOfSignals * sizeof(int32)));
    memset(bufElements, 0, nOfSignals * sizeof(int32));
    bufIdxs  = reinterpret_cast<uint32 *>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(nOfSignals * sizeof(int32)));
    memset(bufIdxs, 0, nOfSignals * sizeof(int32));
    lastBufIdxs = reinterpret_cast<uint32 *>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(nOfSignals * sizeof(int32)));
    memset(lastBufIdxs, 0, nOfSignals * sizeof(int32));
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
	{
	    streamBuffers[i] = reinterpret_cast<float32 *>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(numberOfBuffers * bufElements[i] * sizeof(float32)));
	    memset(streamBuffers[i], 0, numberOfBuffers * bufElements[i] * sizeof(float32));
	}
	if(synchronizingIdx >= 0)  //If this DataSource is a synchronizing one, allocate buffer for times
	{ //Note that one time ot of bufElements[n] shall be actually used
      	    synchStreamTime = reinterpret_cast<float32 *>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(numberOfBuffers * bufElements[synchronizingIdx]  * sizeof(float32)));
	}
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

    evStreams = new MDSplus::EventStream *[nOfSignals - 1];
    for (uint32 sigIdx = 0; sigIdx < nOfSignals - 1; sigIdx++) {
        if(sigIdx == (uint32)synchronizingIdx)
	{
	    streamListeners[sigIdx] = new StreamListener(channelNames[sigIdx], sigIdx, streamBuffers, bufIdxs, lastBufIdxs, bufElements, &eventSem, 
						     &mutexSem, numberOfBuffers, false, synchStreamTime, &started, &periodGuess);
	}
	else
	{
	    streamListeners[sigIdx] = new StreamListener(channelNames[sigIdx], sigIdx, streamBuffers, bufIdxs, lastBufIdxs, bufElements, &eventSem, 
						     &mutexSem, numberOfBuffers, false, NULL, &started, &periodGuess);
	}
        REPORT_ERROR(ErrorManagement::Debug, "REGISTER LISTENER %s", channelNames[sigIdx].Buffer());
        evStreams[sigIdx] = new MDSplus::EventStream(channelNames[sigIdx].Buffer());
	evStreams[sigIdx]->registerListener(streamListeners[sigIdx]);
	bufIdxs[sigIdx] = (numberOfBuffers - 1) * bufElements[sigIdx];
	lastBufIdxs[sigIdx] = 0;
        evStreams[sigIdx]->start();
    }
  }
   counter = 0;
   startCycleTicks = 0;
   periodGuess = 0.;
   return ok;
}




uint32 StreamIn::GetNumberOfBuffers() const {
    return 1;
}

void StreamListener::dataReceived(MDSplus::Data *samples, MDSplus::Data *times, int shot)
{
    
    if(!*started) return;
    std::vector<float> bufArr;
    std::vector<float> timeArr;
#ifdef DEBUG
    std::cout << "Received "<<times->decompile()<<" (times)  "<< samples->decompile() << 
      " (samples) elements from " << times->decompile() <<", signal idx: " << signalIdx << std::endl;
#endif
    try {
        char clazz, dtype;
        samples->getInfo(&clazz, &dtype);
        if(clazz == CLASS_A)
        {
            bufArr = samples->getFloatArray();
        }
        else
        {
            float sample = samples->getFloat();
            bufArr = std::vector<float>(1);
            bufArr[0] = sample;
        }
        times->getInfo(&clazz, &dtype);
        if(clazz == CLASS_A)
        {
            timeArr = times->getFloatArray();
        }
        else
        {
            float time = times->getFloat();
            timeArr = std::vector<float>(1);
            timeArr[0] = time;
        }
    }catch(MDSplus::MdsException &exc)
    {
        std::cout << "Error getting MDSplus Stream data" << std::endl;
        return;
    }
    if(bufElements[signalIdx] > 1)
    {
        float64 currPeriod = *periodGuessPtr;
        if(bufArr.size() != bufElements[signalIdx])
        {
            std::cout << "Received array lenght " << bufArr.size() << " in streaming is different from expected length " << bufElements[signalIdx] << std::endl;
            return;
        }
        if(timeArr.size() != 1)
        {
            std::cout << "Only single time allowed when data arrays are streamed\n";
            return;
        }

        mutexSem->FastLock();
        for (uint32 el = 0; el < bufArr.size(); el++)  
        {
            streamBuffers[signalIdx][el] = bufArr[el];
        }
        if(timeBuffer != NULL)
        {
          timeBuffer[0] = timeArr[0];//Here timeArr.size() == bufArr.size()
        }
    }
    else //Scalar values expected. If a stream message includes more times, take only the last one
    {
        if(bufArr.size() != timeArr.size())
        {
            std::cout << "Received array lenght "<<(uint32)bufArr.size()<<" in streaming different from time array length "<< timeArr.size()<<std::endl;
            return;
        }
        streamBuffers[signalIdx][0] = bufArr[bufArr.size()-1];
        if(timeBuffer != NULL)
        {
          timeBuffer[0] = timeArr[timeArr.size() - 1];//Here timeArr.size() == bufArr.size()
        }
    }
    if(timeBuffer != NULL)
        eventSem->Post();
    mutexSem->FastUnLock();
}


CLASS_REGISTER(StreamIn, "1.0")
}

