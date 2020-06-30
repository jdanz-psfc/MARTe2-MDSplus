
/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/
#include "AdvancedErrorManagement.h"
#include "CLASSMETHODREGISTER.h"
#include "mcc118.h"
#include <mdsobjects.h>
#include <stdio.h>
/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/
namespace MARTe {

mcc118::mcc118() :
        DataSourceI(),
        MessageI() {
	dataBuffers = NULL_PTR(float32 *);
	actNumChannels = 0;
	cpuMask = 0;
}

mcc118::~mcc118() {
//Free allocated buffers

    if (dataBuffers != NULL_PTR(float32 *)) {
        GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(dataSourceMemory));
    }
}

bool mcc118::AllocateMemory() {
    return true;
}

uint32 mcc118::GetNumberOfMemoryBuffers() {
    return 1u;
}

bool mcc118::GetSignalMemoryBuffer(const uint32 signalIdx, const uint32 bufferIdx, void*& signalAddress) {
    bool ok = (dataBuffers != NULL_PTR(float32 *));
    if (ok) {
        /*lint -e{613} dataSourceMemory cannot be NULL here*/
        flot32 *memPtr = &dataBuffers[signalIdx];
        signalAddress = reinterpret_cast<void *&>(memPtr);
    }
    return ok;
}

const char8* mcc118::GetBrokerName(StructuredDataI& data, const SignalDirection direction) {
    const char8* brokerName = "";
    if (direction == InputSignals) {
            brokerName = "MemoryMapSynchronisedInputBroker";
    }
    return brokerName;
}

bool mcc118::GetOutputBrokers(ReferenceContainer& inputBrokers, const char8* const functionName, void* const gamMemPtr) {
    return false;
}

bool mcc118::IsSupportedBroker(const SignalDirection direction, const uint32 functionIdx, const uint32 functionSignalIdx, const char8* const brokerClassName)
{
     return true;
}

bool mcc118::GetInputBrokers(ReferenceContainer& inputBrokers, const char8* const functionName, void* const gamMemPtr) {
  
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

bool mcc118::Synchronise() {
    bool ok = true;
    uint32 n;

//Wait for data; read data and copy 16 floats into dataBuffers
  return ok;
}
 
 
 
 
/*lint -e{715}  [MISRA C++ Rule 0-1-11], [MISRA C++ Rule 0-1-12]. Justification: NOOP at StateChange, independently of the function parameters.*/
bool mcc118::PrepareNextState(const char8* const currentStateName, const char8* const nextStateName) {
    return true;
}

bool mcc118::Initialise(StructuredDataI& data) {

    bool ok = DataSourceI::Initialise(data);
    if (ok) {
        uint32 cpuMaskIn;
        ok = data.Read("CpuMask", cpuMaskIn);
        if (!ok) {
            cpuMask = -1;
        }
        else {
            cpuMask = cpuMaskIn; //Not used for now
        }
    }

    ok = data.MoveRelative("Signals");
    if(!ok) {
	REPORT_ERROR(ErrorManagement::ParametersError,"Signals node Missing.");
	return ok;
    }
    uint32 nOfSignals = data.GetNumberOfChildren();
    ok = (nOfSignals > 0 && nOfSignal <= 16);
    if(!ok) {
	REPORT_ERROR(ErrorManagement::ParametersError,"Incorrect number %d of signals. Must be 1-16", nOfSignals);
	return ok;
    }
    data.MoveToAncestor(1u);

   return ok;
}

bool mcc118::SetConfiguredDatabase(StructuredDataI& data) {
    bool ok = DataSourceI::SetConfiguredDatabase(data);
    //Check signal properties and compute memory
    uint32 nOfSignals = 0u;  
    if (ok) { // Check that only one GAM is Connected to the MDSReaderNS
        uint32 auxNumberOfFunctions = GetNumberOfFunctions();
        ok = (auxNumberOfFunctions == 1u);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "Exactly one Function allowed to interact with this mcc118 DataSource. Number of Functions = %u",
                         auxNumberOfFunctions);
        }
    }
    dataBuffers  = reinterpret_cast<float32 *>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(actNumChannels * sizeof(float32)));
    memset(dataBuffers, 0, actNumChannels * sizeof(float32));
    if(ok)
    {
	for (uint32 n = 0u; (n < actNumChannels) && ok; n++) {
        {
	    if(ok)
	    {
	        uint32 nElements;
		GetSignalNumberOfElements(n, nElements);
		ok = (nElements == 1);
		if(!ok)  {
            	    REPORT_ERROR(ErrorManagement::ParametersError,
                         "Wrong number elements for signal %d: scalar expected", n);
		}
	    }
	    if(ok)
	    {
		uint32 nSamples;
		ok = GetFunctionSignalSamples(InputSignals, 0u, n, nSamples);
		if (ok) {
		    ok = (nSamples == 1u);
		}
		if (!ok) {
		    REPORT_ERROR(ErrorManagement::ParametersError, "The number of samples shall be exactly 1");
		}
	    }
	    if(ok)
	    {
		  TypeDescriptor currType;
		  currType = GetSignalType(n);
		  ok = (currType == Float32Bit);
		  if (!ok) {
		      REPORT_ERROR(ErrorManagement::ParametersError, "Invalid type for channel %d", n);
		  }
	    }
	}
    }
    if(ok)
    {
      //Board Initialization code
    }
    return ok;
}




uint32 mcc118::GetNumberOfBuffers() const {
    return 1;
}


CLASS_REGISTER(mcc118, "1.0")
}

