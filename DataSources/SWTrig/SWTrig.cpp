
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

//#define DEBUG 1
  
SWTrig::SWTrig():  DataSourceI(), EmbeddedServiceMethodBinderI(), executor(*this)
{

  for(uint32 trigIdx = 0; trigIdx < MAX_TRIGGER_OUTPUTS; trigIdx++)
    {
	trigState[trigIdx] = NotTriggered;
	waitCycles[trigIdx] = -1;
	trigCounter[trigIdx] =0;
	trigCycles[trigIdx] = 0;
	triggers[trigIdx] = 0; 
	waitCounter[trigIdx] = 0;
	trigEvent[trigIdx] = NULL_PTR(SWTrigEvent *);
    }
    startEvent = NULL_PTR(SWTrigEvent *);
    if (!startSem.Create()) {
        REPORT_ERROR(ErrorManagement::FatalError, "Could not create EventSem.");
    }
    if (!synchSem.Create()) {
        REPORT_ERROR(ErrorManagement::FatalError, "Could not create EventSem.");
    }
    lastTimeTicks = 0u;
    sleepTimeTicks = 0u;
    timerPeriodUsecTime = 0u;
    cycles = 0;
    time = 0;
    clockStarted = false;
}

SWTrig::~SWTrig() 
{
    for(uint32 trigIdx = 0; trigIdx < MAX_TRIGGER_OUTPUTS; trigIdx++)
    {
	if(trigEvent[trigIdx] != NULL_PTR(SWTrigEvent *))
	    delete trigEvent[trigIdx];
    }
    if(startEvent != NULL_PTR(SWTrigEvent *))
	delete startEvent;
    if (!synchSem.Post()) {
	REPORT_ERROR(ErrorManagement::FatalError, "Could not post SynchSem.");
    }
    if (!startSem.Post()) {
	REPORT_ERROR(ErrorManagement::FatalError, "Could not post StartSem.");
    }
    if (!executor.Stop()) {
        REPORT_ERROR(ErrorManagement::FatalError, "Could not stop SingleThreadService.");
    }
}

bool SWTrig::AllocateMemory() {
      return true;
}

uint32 SWTrig::GetNumberOfMemoryBuffers() {
    return 1u;
}

bool SWTrig::GetSignalMemoryBuffer(const uint32 signalIdx, const uint32 bufferIdx, void*& signalAddress) {
    if(signalIdx == 0)
        signalAddress = reinterpret_cast<void *>(&cycles);
    else if(signalIdx == 1)
        signalAddress = reinterpret_cast<void *>(&time);
    else if(signalIdx < MAX_TRIGGER_OUTPUTS * 2)
        signalAddress = reinterpret_cast<void *>(&triggers[signalIdx - 2]);
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

void SWTrig::enableTrigger(int32 idx)
{
    if(idx == -1)
    {
	REPORT_ERROR(ErrorManagement::Debug, "Received Start Event %s", startEventName.Buffer());
	clockStarted = true;
	if(!startSem.Post())
	    REPORT_ERROR(ErrorManagement::FatalError, "Cannot post start  semaphore");
    }
    else
    {
	REPORT_ERROR(ErrorManagement::Debug, "Received Trigger Event %s", trigEventNames[idx].Buffer());
	trigState[idx] = DelayingTrigger;
	waitCounter[idx] = 0;
    }
}

bool SWTrig::Synchronise() {
    ErrorManagement::ErrorType err;
    err = synchSem.ResetWait(TTInfiniteWait);
    for( uint32 trigIdx = 0; trigIdx < MAX_TRIGGER_OUTPUTS; trigIdx++)
    {
	if(trigState[trigIdx] != NotTriggered)
	{
	    if(trigState[trigIdx] == Triggering)
	    {
		trigCounter[trigIdx]++;
		if(trigCounter[trigIdx] >= trigCycles[trigIdx])
		{
		    triggers[trigIdx]= 0; 
		    trigState[trigIdx] = Triggered;
		}
	    }
	    else if(trigState[trigIdx] == DelayingTrigger)
	    {
		waitCounter[trigIdx]++;
		if(waitCounter[trigIdx] >= (uint32)waitCycles[trigIdx])
		{
		    REPORT_ERROR(ErrorManagement::Debug, "********Trigger  %d*********", trigIdx);
		    triggers[trigIdx] = 1;
		    trigState[trigIdx] = Triggering;
		    trigCounter[trigIdx] = 0;
		}
	    }
	}
    }
#ifdef DEBUG
     REPORT_ERROR(ErrorManagement::Debug, "clock tick");
#endif   
    return err.ErrorsCleared();
 }
 
 ErrorManagement::ErrorType SWTrig::Execute(ExecutionInfo& info) 
 {
    ErrorManagement::ErrorType err;
    if(!clockStarted && startEvent != NULL_PTR(SWTrigEvent *))
    {
	err = startSem.ResetWait(TTInfiniteWait);
    }
    if (lastTimeTicks == 0u) {
        lastTimeTicks = HighResolutionTimer::Counter();
    }
    uint64 startTicks = HighResolutionTimer::Counter();
    //If we lose cycle, rephase to a multiple of the period.
    uint32 nCycles = 0u;
    while (lastTimeTicks < startTicks) {
        lastTimeTicks += sleepTimeTicks;
        nCycles++;
    }
    lastTimeTicks -= sleepTimeTicks;

    //Sleep until the next period. Cannot be < 0 due to while(lastTimeTicks < startTicks) above
    uint64 sleepTicksCorrection = (startTicks - lastTimeTicks);
    uint64 deltaTicks = sleepTimeTicks - sleepTicksCorrection;

    float32 sleepTime = static_cast<float32>(static_cast<float64>(deltaTicks) * HighResolutionTimer::Period());
    Sleep::NoMore(sleepTime);
    lastTimeTicks = HighResolutionTimer::Counter();

    err = !synchSem.Post();
    cycles += nCycles;
    time = triggerTime * 1E6 + cycles * timerPeriodUsecTime;
    return err;
}

 
 
/*lint -e{715}  [MISRA C++ Rule 0-1-11], [MISRA C++ Rule 0-1-12]. Justification: NOOP at StateChange, independently of the function parameters.*/
bool SWTrig::PrepareNextState(const char8* const currentStateName, const char8* const nextStateName) 
{
    bool ok = true;
    if (executor.GetStatus() == EmbeddedThreadI::OffState) {
        ok = executor.Start();
    }
    cycles = 0u;
    time = 0;
    return ok;
}

bool SWTrig::Initialise(StructuredDataI& data) {
    REPORT_ERROR(ErrorManagement::Debug, "Initialise");
    bool ok = DataSourceI::Initialise(data);
    if (ok) {
        ok = data.Read("StartEvent", startEventName);
	if(ok)
	{
	    startEvent = new SWTrigEvent((char *)startEventName.Buffer(), -1, this);
	}
	else 
	    startEvent = NULL_PTR(SWTrigEvent *);
    }
    if(ok)
    {
        frequency = 0;
        ok = data.Read("Frequency", frequency);
	if(!ok || frequency <= 0)
	{
	    ok = false;
	    REPORT_ERROR(ErrorManagement::Information, "Clock Frequency shall be specified and shall be positive");
	}
	else
	{
	    timerPeriodUsecTime = (uint32)((1./frequency) * 1E6);
	}
    }
    if(ok)
    {
//        ok = data.Read("TriggerTime", triggerTime);
        ok = data.Read("StartTime", triggerTime);
	if(!ok)
	{
	    REPORT_ERROR(ErrorManagement::Information, "TriggerTime shall be specified ");
	}
    }
   uint32 cpuMask, stackSize;
   if (ok) {
        if (!data.Read("CPUMask", cpuMask)) {
            cpuMask = 0xFFu;
            REPORT_ERROR(ErrorManagement::Warning, "CPUMask not specified using: %d", cpuMask);
        }
    }
    if (ok) {
       if (!data.Read("StackSize", stackSize)) {
            stackSize = THREADS_DEFAULT_STACKSIZE;
            REPORT_ERROR(ErrorManagement::Warning, "StackSize not specified using: %d", stackSize);
        }
    }
    if (ok) {
        ok = (stackSize > 0u);

        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "StackSize shall be > 0u");
        }
    }
    if (ok) {
        executor.SetCPUMask(cpuMask);
        executor.SetStackSize(stackSize);
    }
    if(ok)
    {
	ok = data.MoveRelative("Signals");
	if(!ok) {
	    REPORT_ERROR(ErrorManagement::ParametersError,"Signals node Missing.");
	}
    }
    if(ok)
    {
        uint32 numSignals = data.GetNumberOfChildren();
	ok = (numSignals >= 2);
	if(!ok) {
	    REPORT_ERROR(ErrorManagement::ParametersError,"At least two signals (Counter and Time) shall be defined");
	}
	else
	{
	    numTriggers = numSignals - 2;
	    if(numTriggers > MAX_TRIGGER_OUTPUTS)
	        numTriggers = MAX_TRIGGER_OUTPUTS;
	    for(uint32 trigIdx = 0; trigIdx < numTriggers; trigIdx++)
	    {
	        StreamString currTrigName;
		currTrigName.Printf("Trigger%d", trigIdx + 1);
		ok = data.MoveRelative(currTrigName.Buffer());
		if(!ok)
		{
		     REPORT_ERROR(ErrorManagement::ParametersError,"Cannot move to signal Trigger%d", trigIdx + 1);
		     break;
		}
		ok = data.Read("TriggerEvent", trigEventNames[trigIdx]);
		if(!ok)
		{
		     REPORT_ERROR(ErrorManagement::ParametersError,"Cannot read event name for Trigger%d", trigIdx + 1);
		     break;
		}
		trigEvent[trigIdx] = new SWTrigEvent((char *)trigEventNames[trigIdx].Buffer(), trigIdx, this);
		ok =  data.Read("WaitCycles", waitCycles[trigIdx]);
		if (!ok) {
                    waitCycles[trigIdx] = 0;
		    ok = true;
		}
		ok = data.Read("TriggerCycles", trigCycles[trigIdx]);
		if(!ok)
                {
		    trigCycles[trigIdx] = 1;
		    REPORT_ERROR(ErrorManagement::Information, "No trigCycles specified for signal Trigger%d. Trigger will last one cycle.", trigIdx + 1);
		 }
		data.MoveToAncestor(1);
	    }
	}
    }
    data.MoveToAncestor(1);
    REPORT_ERROR(ErrorManagement::Debug, "Initialise Ended");
    return ok;
}

bool SWTrig::SetConfiguredDatabase(StructuredDataI& data) 
{
    REPORT_ERROR(ErrorManagement::Debug, "SetConfiguredDatabase");
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
            ok = (nOfSignals >= 2u);
            if (!ok) {
                REPORT_ERROR(ErrorManagement::ParametersError, "The number of signals shall be at least two");
            }
	}
    }

    if(ok)
    {
        for(uint sigIdx = 0; sigIdx < nOfSignals; sigIdx++)
	{
	    uint32 nElements;
	    ok = GetSignalNumberOfElements(sigIdx, nElements);
	    if(!ok) 
	    {
	        REPORT_ERROR(ErrorManagement::ParametersError,
                         "Error getting number of elements for signal");
	    }
	    if(nElements != 1)
	    {
	        REPORT_ERROR(ErrorManagement::ParametersError,
                         "Output signals shall have one element (scalar)");
	        ok = false;
	    }
	}
    }
    if(ok)
    {
        uint32 nSamples;
        for(uint sigIdx = 0; sigIdx < nOfSignals; sigIdx++)
	{
            ok = GetFunctionSignalSamples(InputSignals, 0u, 0, nSamples);
             if (ok) {
                 ok = (nSamples == 1u);
             }
             if (!ok) {
                 REPORT_ERROR(ErrorManagement::ParametersError, "The number of samples for output signals shall be exactly 1");
            }
	}
    }
    TypeDescriptor type;
    if (ok) { //read the type specified in the configuration file 
        type = GetSignalType(0);
        ok = !(type == InvalidType);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "Invalid type for Counter");
        }
        ok = (type == UnsignedInteger32Bit || type == SignedInteger32Bit);
	if(!ok)
	{
            REPORT_ERROR(ErrorManagement::ParametersError, "Counter type shall be either UnsignedInteger32Bit or SignedInteger32Bit");
	}
    }
    if (ok) { //read the type specified in the configuration file 
        type = GetSignalType(1);
        ok = !(type == InvalidType);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "Invalid type for Counter");
        }
        ok = (type == UnsignedInteger32Bit || type == SignedInteger32Bit);
	if(!ok)
	{
            REPORT_ERROR(ErrorManagement::ParametersError, "Time type shall be either UnsignedInteger32Bit or SignedInteger32Bit");
	}
    }
    if(ok)
    {
        for(uint trigIdx = 0; trigIdx < numTriggers; trigIdx++)
	{
            type = GetSignalType(2+trigIdx);
            ok = !(type == InvalidType);
            if (!ok) {
                REPORT_ERROR(ErrorManagement::ParametersError, "Invalid type for Trigger%d", trigIdx+1);
            }
            ok = (type == UnsignedInteger8Bit || type == SignedInteger8Bit);
	    if(!ok)
	    {
		REPORT_ERROR(ErrorManagement::ParametersError, "Trigger%d type shall be either UnsignedInteger8Bit or SignedInteger8Bit", trigIdx+1);
	    }
	}
    }
    //Start trheads
    if(startEvent != NULL_PTR(SWTrigEvent *))
	startEvent->start();
    for(uint32 trigIdx = 0; trigIdx < numTriggers; trigIdx++)
    {
	if(trigEvent[trigIdx] != NULL_PTR(SWTrigEvent *))
	trigEvent[trigIdx]->start();
    }
    
    for(uint trigIdx = 0; trigIdx < numTriggers; trigIdx++)
    {
	triggers[trigIdx] = 0;  
        waitCounter[trigIdx] = 0;
	trigCounter[trigIdx] = 0;
	if(trigEvent[trigIdx] == NULL_PTR(SWTrigEvent *))
	    trigState[trigIdx] = DelayingTrigger;
	else
	    trigState[trigIdx] = NotTriggered;
    }
    cycles = 0;
    time = 0;
    float64 sleepTimeT = (static_cast<float64>(HighResolutionTimer::Frequency()) / frequency);
    sleepTimeTicks = static_cast<uint64>(sleepTimeT);
    return ok;
}




uint32 SWTrig::GetNumberOfBuffers() const {
    return 1;
}


CLASS_REGISTER(SWTrig, "1.0")
}

