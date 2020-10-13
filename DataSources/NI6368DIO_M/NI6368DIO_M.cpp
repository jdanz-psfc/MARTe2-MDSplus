/**
 * @file NI6368DIO.cpp
 * @brief Source file for class NI6368DIO
 * @date 03/01/2017
 * @author Andre Neto
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
 * the class NI6368DIO_M (public, protected, and private). Be aware that some
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/
#include <fcntl.h>

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/
#include "AdvancedErrorManagement.h"
#include "MemoryMapSynchronisedInputBroker.h"
#include "MemoryMapSynchronisedOutputBroker.h"
#include "NI6368DIO_M.h"

/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/
namespace MARTe {
NI6368DIO_M::NI6368DIO_M() : DataSourceI() {
    clockId = -1;
    triggerId = -1;
    boardId = -1;
    boardFileDescriptor = -1;
   if(!mutexSemInitialized)  //This code shall be executed in sequence
    {
	mutexSem.Create();
	mutexSemInitialized = true;
    }
}

NI6368DIO_M::~NI6368DIO_M() {
    if(boardFileDescriptor != -1)
    {
	close(boardFileDescriptor);
    }    
}

bool NI6368DIO_M::AllocateMemory() {
    return true;
}

uint32 NI6368DIO_M::GetNumberOfMemoryBuffers() {
    return 1u;
}
bool NI6368DIO_M::GetSignalMemoryBuffer(const uint32 signalIdx, const uint32 bufferIdx, void*& signalAddress) {
    bool ok;
    if(mode != NI6368DIO_M_SYNCH_IN)
    {
        ok = (signalIdx == 0);
        if (ok) {
            signalAddress = static_cast<void *> (&port0Bits);
        }
    }
    else
    {
        ok = (signalIdx < 2);
        if (ok) {
           signalAddress = (signalIdx == 0)?static_cast<void *> (&time):static_cast<void *> (&port0Bits);
        }
    }

    return ok;
}
bool NI6368DIO_M::IsSupportedBroker(const SignalDirection direction, const uint32 functionIdx, const uint32 functionSignalIdx, const char8* const brokerClassName)
{
     return true;
}

const char8* NI6368DIO_M::GetBrokerName(StructuredDataI& data, const SignalDirection direction) {
    const char8 *brokerName = NULL_PTR(const char8 *);
 REPORT_ERROR(ErrorManagement::Debug, "GetBrokerName"); 
    if (direction == OutputSignals) 
    {
        brokerName = "MemoryMapSynchronisedOutputBroker";
    }
    else 
    {
        brokerName = "MemoryMapSynchronisedInputBroker";
    }
    return brokerName;
}

bool NI6368DIO_M::GetInputBrokers(ReferenceContainer& inputBrokers, const char8* const functionName, void* const gamMemPtr) {
  REPORT_ERROR(ErrorManagement::Debug, "GetInputBrokers");
    if(mode == NI6368DIO_M_OUT)
	return false;
    ReferenceT<MemoryMapSynchronisedInputBroker> broker("MemoryMapSynchronisedInputBroker");
    bool ok = broker.IsValid();

    if (ok) {
        ok = broker->Init(InputSignals, *this, functionName, gamMemPtr);
    }
    if (ok) {
        ok = inputBrokers.Insert(broker);
    }
 REPORT_ERROR(ErrorManagement::Debug, "GetInputBrokers fatta %d", ok);
 
    return ok;
}

bool NI6368DIO_M::GetOutputBrokers(ReferenceContainer& outputBrokers, const char8* const functionName, void* const gamMemPtr) {
 REPORT_ERROR(ErrorManagement::Debug, "GetOutputBrokers");   
    if(mode != NI6368DIO_M_OUT)
	return false;
    ReferenceT<MemoryMapSynchronisedOutputBroker> broker("MemoryMapSynchronisedOutputBroker");
    bool ok = broker.IsValid();

    if (ok) {
        ok = broker->Init(OutputSignals, *this, functionName, gamMemPtr);
    }
    if (ok) {
        ok = outputBrokers.Insert(broker);
    }

    return ok;
}

bool NI6368DIO_M::PrepareNextState(const char8* const currentStateName, const char8* const nextStateName) {
    return true;
}

bool NI6368DIO_M::Initialise(StructuredDataI& data) {
 REPORT_ERROR(ErrorManagement::Debug, "INITIALISE");   
    bool ok = DataSourceI::Initialise(data);
    if (ok) {
        ok = data.Read("Mode", mode);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "The Mode shall be specified");
        }
	if(ok)
	{
	    ok = (mode == NI6368DIO_M_IN || mode == NI6368DIO_M_SYNCH_IN || mode == NI6368DIO_M_OUT);
	    if(!ok){
            	REPORT_ERROR(ErrorManagement::ParametersError, "The Mode %d shall be 1 (NI6368DI_M_IN) or 2 (NI6368DI_M_SYNCH_IN) or 3  (NI6368DI_M_OUT)", mode);
            }
	}
    }
    if (ok) {
        ok = data.Read("DeviceName", deviceName);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "The DeviceName shall be specified");
        }
    }
    if (ok) {
        ok = data.Read("BoardId", boardId);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "The BoardId shall be specified");
        }
    }
    if (ok) {
        ok = data.Read("TriggerId", triggerId);
        if (!ok) {
            triggerId = -1;
	    ok = true;
        }
	ok = (triggerId >= -1 && triggerId < 32);
 	if(!ok)
	{
            REPORT_ERROR(ErrorManagement::ParametersError, "The TriggerId shall be between -1 and 31");
        }
    }
    bitMask = 0;
    if (ok && mode == NI6368DIO_M_OUT) {
        ok = data.Read("BitMask", bitMask);
  	if(!ok)
	{
            REPORT_ERROR(ErrorManagement::ParametersError, "BitMask shall be defined for Digital Out");
        }
    }
    if (ok) {
        ok = data.Read("ClockId", clockId);
        if (!ok) {
            clockId = -1;
	    ok = true;
        }
	ok = (clockId >= -1 && clockId < 32);
	if(!ok)
	{
            REPORT_ERROR(ErrorManagement::ParametersError, "The ClockId shall be between -1 and 31");
        }
    }
    if (ok) {
        ok = data.Read("TriggerTime", triggerTime);
        if (!ok) {
            triggerTime = 0;
	    ok = true;
        } 
    }

    if (ok) {
        ok = data.Read("Period", period);
        if (!ok && mode == NI6368DIO_M_SYNCH_IN) 
	{
            REPORT_ERROR(ErrorManagement::ParametersError, "Period shall be between defined");
        }
    }
    counter = 0; 
    time = static_cast<uint32>(triggerTime * 1E6);
    return ok;
}

bool NI6368DIO_M::SetConfiguredDatabase(StructuredDataI& data) {
    REPORT_ERROR(ErrorManagement::Debug, "PARTE SetConfiguredDatabase");   
    bool ok = DataSourceI::SetConfiguredDatabase(data);
    if(!ok) return ok;

    uint32 nOfFunctions = GetNumberOfFunctions();
    if(nOfFunctions > 1)
    {
	REPORT_ERROR(ErrorManagement::ParametersError, "Only one function allowed");
	ok = false;
	return ok;
    }
    uint32 nOfSignals = 0u;
    nOfSignals = GetNumberOfSignals();
    if(mode == NI6368DIO_M_IN or mode == NI6368DIO_M_OUT)
    {
        ok = (nOfSignals == 1);
        if(!ok){
            REPORT_ERROR(ErrorManagement::ParametersError, "Only one signal (bits) shall be specified");
        }
    }
    else //NI6368DIO_M_SYNCH_IN
    {
        ok = (nOfSignals == 2);
        if(!ok){
            REPORT_ERROR(ErrorManagement::ParametersError, "Time and bits shall be specified");
        }
    }
    for(uint32 sigIdx = 0; sigIdx < nOfSignals; sigIdx++)
    {
        if(ok)
        {
	    TypeDescriptor sigType = GetSignalType(sigIdx);
	    ok = (sigType == SignedInteger32Bit || sigType == UnsignedInteger32Bit);
            if (!ok) {
                REPORT_ERROR(ErrorManagement::ParametersError, "Signal type shall be either Int32 or UInt32");
            }
        }
        uint32 nElements = -1;
        if(ok)
        {
	    GetSignalNumberOfElements(sigIdx, nElements);
	    ok = (nElements == 1);
            if (!ok) {
                REPORT_ERROR(ErrorManagement::ParametersError, "Number of elements shall be 1");
 	    }
        }
    }

    StreamString fullDeviceName;
    //Configure the board
    if (ok) {
        ok = fullDeviceName.Printf("%s.%d.dio", deviceName.Buffer(), boardId);
    }
    if (ok) {
        ok = fullDeviceName.Seek(0LLU);
    }
    if (ok) {
        boardFileDescriptor = open(fullDeviceName.Buffer(), O_RDWR);
        ok = (boardFileDescriptor > -1);
        if (!ok) {
                REPORT_ERROR_PARAMETERS(ErrorManagement::ParametersError, "Could not open device %s", fullDeviceName);
	}
	if(notYetReset)  //No locking requires as this is executed sequentially in preparation 
	{
	    int32 retval = xseries_reset_dio(boardFileDescriptor);
	    ok = (retval == 0);
	    if(!ok) {
		REPORT_ERROR_PARAMETERS(ErrorManagement::ParametersError, "Error reseting DIO segment for device %s", fullDeviceName);
	    }
	}
	    
	if(ok && mode == NI6368DIO_M_OUT)
	{
	    xseries_stop_do(boardFileDescriptor);
	    xseries_do_conf_t do_conf = xseries_static_do(bitMask);
            int32 retval = xseries_load_do_conf(boardFileDescriptor, do_conf);
	    ok = (retval == 0);
	    if(!ok) {
		REPORT_ERROR_PARAMETERS(ErrorManagement::ParametersError, "Error loading DO configuration for device %s", fullDeviceName);
	    }
	    if(ok)
	    {
		int32 retval = xseries_start_do(boardFileDescriptor);
	        ok = (retval == 0);
	        if(!ok) {
		    REPORT_ERROR_PARAMETERS(ErrorManagement::ParametersError, "Error starting DO for device %s", fullDeviceName);
		}
	    }	
	}
	else //DI
 	{
	    xseries_stop_di(boardFileDescriptor);
	    xseries_di_conf_t di_conf = xseries_static_di(0xFFFFFFFF);
            int32 retval = xseries_load_di_conf(boardFileDescriptor, di_conf);
	    ok = (retval == 0);
	    if(!ok) {
		REPORT_ERROR_PARAMETERS(ErrorManagement::ParametersError, "Error loading DI configuration for device %s", fullDeviceName);
	    }
	    if(ok)
	    {
		int32 retval = xseries_start_di(boardFileDescriptor);
	        ok = (retval == 0);
	        if(!ok) {
		    REPORT_ERROR_PARAMETERS(ErrorManagement::ParametersError, "Error starting DI for device %s", fullDeviceName);
		}
	    }	
	}
    }
    return ok;
}

bool NI6368DIO_M::Synchronise()
 {
//The first instance at the first Synchronise will open the channe descriptor
    REPORT_ERROR(ErrorManagement::Debug, "PARTE SYNCHRONIZE");

    bool ok;
    if(mode == NI6368DIO_M_OUT)
    {
	mutexSem.FastLock();  //DO and DI may happen on different threads!
 	ok = (xseries_write_do(boardFileDescriptor,&port0Bits,1) != -1);
	mutexSem.FastUnLock();
    }
    else
    { 
	mutexSem.FastLock(); 
	ok = (xseries_read_di(boardFileDescriptor,&port0Bits,1) != -1);
	mutexSem.FastUnLock();
	if(clockId != -1)
	{
	    bool prevVal = ((port0Bits & (1 << clockId)) != 0);
	    while(true) {
		mutexSem.FastLock();
		ok = (xseries_read_di(boardFileDescriptor,&port0Bits,1) != -1);
		mutexSem.FastUnLock();
		bool currVal = ((port0Bits & (1 << clockId)) != 0);
		bool currClockVal = currVal;
		if(triggerId != -1)
		    currVal = (currVal && (port0Bits & (1 << triggerId)) != 0);
   
		if(currVal && !prevVal) //rising edge and trigger high (if defined)
		{
		    counter++;
		    time = (triggerTime + counter * period) * 1E6;
		    break;
		}
		prevVal = currClockVal;
	    }
	}
    }
    REPORT_ERROR(ErrorManagement::Debug, "FINISCE SYNCHRONIZE: %d", ok);
    return ok;
}



CLASS_REGISTER(NI6368DIO_M, "1.0")


FastPollingMutexSem NI6368DIO_M::mutexSem;
bool NI6368DIO_M::mutexSemInitialized = false;
bool NI6368DIO_M::notYetReset = false;

}

