
/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/
#include "AdvancedErrorManagement.h"
#include "CLASSMETHODREGISTER.h"
#include "SWTrig.h"
#include <mdsobjects.h>
#include <stdio.h>
/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/
namespace MARTe {

SWTrig::SWTrig() :
        DataSourceI(),
        MessageI() {
	trigState = NotTriggered;
	waitCycles = -1;
	trigCounter =0;
	trigCycles = 0;
	trigPtr  = new uint8; 
	cycleCounter = 0;
	trigEvent = NULL_PTR(SWTrigEvent *);
}

SWTrig::~SWTrig() {
  delete trigPtr;
}

bool SWTrig::AllocateMemory() {
    return true;
}

uint32 SWTrig::GetNumberOfMemoryBuffers() {
    return 1u;
}

bool SWTrig::GetSignalMemoryBuffer(const uint32 signalIdx, const uint32 bufferIdx, void*& signalAddress) {
    signalAddress = reinterpret_cast<void *&>(trigPtr);
    return true;
}

const char8* SWTrig::GetBrokerName(StructuredDataI& data, const SignalDirection direction) {
    const char8* brokerName = "";
    if (direction == InputSignals) {
            brokerName = "MemoryMapSynchronisedInputBroker";
    }
    return brokerName;
}

bool SWTrig::GetOutputBrokers(ReferenceContainer& inputBrokers, const char8* const functionName, void* const gamMemPtr) {
    return false;
}

bool SWTrig::IsSupportedBroker(const SignalDirection direction, const uint32 functionIdx, const uint32 functionSignalIdx, const char8* const brokerClassName)
{
     return true;
}

bool SWTrig::GetInputBrokers(ReferenceContainer& inputBrokers, const char8* const functionName, void* const gamMemPtr) {
  
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
void SWTrig::trigger()
{
  std::cout << "TRIGGER!!!" << std::endl;
    *trigPtr = 1;
    trigState = Triggering;
    trigCounter = 0;
}

bool SWTrig::Synchronise() {
    cycleCounter++;
//    std::cout << cycleCounter << "   " << waitCycles << std::endl;
    if(trigState == NotTriggered && waitCycles > 0 && cycleCounter >= (uint32)waitCycles)
    {
	trigger();
	return true;
    }
    if(trigState == Triggering)
    {
	if(trigCounter >= trigCycles)
	{
	    *trigPtr = 0; 
	    trigState = Triggered;
	}
        trigCounter++;
    }
    return true;
 }
 
 
 
 
/*lint -e{715}  [MISRA C++ Rule 0-1-11], [MISRA C++ Rule 0-1-12]. Justification: NOOP at StateChange, independently of the function parameters.*/
bool SWTrig::PrepareNextState(const char8* const currentStateName, const char8* const nextStateName) {
    return true;
}

bool SWTrig::Initialise(StructuredDataI& data) {
std::cout << "INIT" << std::endl;
    bool ok = DataSourceI::Initialise(data);
    if (ok) {
	StreamString eventName;
        ok = data.Read("TrigEvent", eventName);
	if(ok)
	{
	    trigEvent = new SWTrigEvent((char *)eventName.Buffer(), this);
	}
    }
    ok = data.Read("WaitCycles", waitCycles);
    if (!ok) {
        waitCycles = -1;
	if(!trigEvent)
	{
	    REPORT_ERROR(ErrorManagement::Information, "Neither Event nor WaitCycles specified. No Trigger will be generated.");
        }
    }
    ok = data.Read("TrigCycles", trigCycles);
    if(!ok)
    {
        trigCycles = 1;
	REPORT_ERROR(ErrorManagement::Information, "No trigCycles specified. Trigger will last one cycle.");
    }
    ok = data.MoveRelative("Signals");
    if(!ok) {
	REPORT_ERROR(ErrorManagement::ParametersError,"Signals node Missing.");
	return ok;
    }
    uint32 nOfSignals = data.GetNumberOfChildren();
    if(nOfSignals != 1)
    {
	  REPORT_ERROR(ErrorManagement::ParametersError,"SWTrig shall have exactly one output.");
	  return false;
    }
    data.MoveToAncestor(1);
    
std::cout << "INIT FATTA" << std::endl;
    return true;
}

bool SWTrig::SetConfiguredDatabase(StructuredDataI& data) {

  std::cout << "SET CNFIGURED" << std::endl;
    bool ok = DataSourceI::SetConfiguredDatabase(data);
    //Check signal properties and compute memory
    uint32 nOfSignals = 0u;  
    if (ok) { // Check that only one GAM is Connected to the MDSReaderNS
        uint32 auxNumberOfFunctions = GetNumberOfFunctions();
        ok = (auxNumberOfFunctions == 1u);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "Exactly one Function allowed to interact with this SWTrig DataSource. Number of Functions = %u",
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
            ok = (nOfSignals == 1u);
            if (!ok) {
                REPORT_ERROR(ErrorManagement::ParametersError, "The number of signals shall be exactly one");
            }
	}
    }

    if(ok)
    {
	uint32 nElements;
	ok = GetSignalNumberOfElements(0, nElements);
	if(!ok) 
	{
	    REPORT_ERROR(ErrorManagement::ParametersError,
                         "Error getting number of elements for signal");
	}
	if(nElements != 1)
	{
	    REPORT_ERROR(ErrorManagement::ParametersError,
                         "Output signal shall have one element (scalar)");
	    ok = false;
	}
    }
    if(ok)
    {
        uint32 nSamples;
        ok = GetFunctionSignalSamples(InputSignals, 0u, 0, nSamples);
         if (ok) {
             ok = (nSamples == 1u);
         }
         if (!ok) {
             REPORT_ERROR(ErrorManagement::ParametersError, "The number of samples shall be exactly 1");
        }
    }
    TypeDescriptor type;
    if (ok) { //read the type specified in the configuration file 
        type = GetSignalType(0);
        ok = !(type == InvalidType);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "Invalid type for signal");
        }
        ok = (type == UnsignedInteger8Bit);
	if(!ok)
	{
            REPORT_ERROR(ErrorManagement::ParametersError, "Signal type shall be uint8");
	}
    }

    *trigPtr = 0;  
    cycleCounter = 0;
    trigCounter = 0;
    trigState = NotTriggered;
    std::cout << "OK: " << ok << std::endl;
    return ok;
}




uint32 SWTrig::GetNumberOfBuffers() const {
    return 1;
}


CLASS_REGISTER(SWTrig, "1.0")
}

