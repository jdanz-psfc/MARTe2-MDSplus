/**
 * @file NI6259DIO.cpp
 * @brief Source file for class NI6259DIO
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
 * the class NI6259DIO_M (public, protected, and private). Be aware that some
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
#include "NI6259DIO_M.h"

/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/
namespace MARTe {
NI6259DIO_M::NI6259DIO_M() : DataSourceI() {
    clockId = -1;
    triggerId = -1;
    boardId = -1;
    if(!mutexSemInitialized)  //This code shall be executed in sequence
    {
	mutexSem.Create();
	mutexSemInitialized = true;
        for(int32 i = 0; i < MAX_NI6259_DEVICES; i++)
        {
	    portFileDescriptor[i] = -1;
   	    dioConfiguration[i] = pxi6259_create_dio_conf();
	    if(pxi6259_add_dio_channel(&dioConfiguration[i], 0, 0) != 0) ;//By default, all inputs


        }
    }
}

NI6259DIO_M::~NI6259DIO_M() {
    if(boardId >= 0)  
    {
    	mutexSem.FastLock();
	if(portFileDescriptor[boardId] != -1) //Close it only once
	{
	    close(portFileDescriptor[boardId]);
	    portFileDescriptor[boardId] = -1;
	}
 	mutexSem.FastUnLock();
    }	    
}


//Private methods to handle a single file and channel descriptor per board so that configurations
// can be updated by different instances of NI6259_DIO_M data source
bool NI6259DIO_M::openPortDescriptor() {
    mutexSem.FastLock();
    int32 fd;
     
    if(portFileDescriptor[boardId] == -1)
    {
        StreamString fullDeviceName;
    //Configure the board
        fullDeviceName.Printf("%s.%d.dio.0", deviceName.Buffer(), boardId);
	fullDeviceName.Seek(0LLU);
        bool ok = (pxi6259_load_dio_conf(boardFileDescriptor, &dioConfiguration[boardId]) == 0);
        if (!ok) {
            REPORT_ERROR_PARAMETERS(ErrorManagement::ParametersError, "Could not load configuration for device %s", fullDeviceName);
	    fd = -1;
        }
	else
	{
            fd = portFileDescriptor[boardId] = open(fullDeviceName.Buffer(), O_RDWR);
	}
	if(fd == -1)
	{
           REPORT_ERROR(ErrorManagement::ParametersError, "Could not open device %s", fullDeviceName);
 	}
    }
    else //Already open by another NI6259_M instance
    {
	    fd = portFileDescriptor[boardId];
    }
    mutexSem.FastUnLock();
    return (fd != -1);
}

bool NI6259DIO_M::AllocateMemory() {
    return true;
}

uint32 NI6259DIO_M::GetNumberOfMemoryBuffers() {
    return 1u;
}
bool NI6259DIO_M::GetSignalMemoryBuffer(const uint32 signalIdx, const uint32 bufferIdx, void*& signalAddress) {
    bool ok;
    if(mode != NI6259DIO_M_SYNCH_IN)
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
bool NI6259DIO_M::IsSupportedBroker(const SignalDirection direction, const uint32 functionIdx, const uint32 functionSignalIdx, const char8* const brokerClassName)
{
     return true;
}

const char8* NI6259DIO_M::GetBrokerName(StructuredDataI& data, const SignalDirection direction) {
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

bool NI6259DIO_M::GetInputBrokers(ReferenceContainer& inputBrokers, const char8* const functionName, void* const gamMemPtr) {
  REPORT_ERROR(ErrorManagement::Debug, "GetInputBrokers");
    if(mode == NI6259DIO_M_OUT)
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

bool NI6259DIO_M::GetOutputBrokers(ReferenceContainer& outputBrokers, const char8* const functionName, void* const gamMemPtr) {
 REPORT_ERROR(ErrorManagement::Debug, "GetOutputBrokers");   
    if(mode != NI6259DIO_M_OUT)
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

bool NI6259DIO_M::PrepareNextState(const char8* const currentStateName, const char8* const nextStateName) {
    return true;
}

bool NI6259DIO_M::Initialise(StructuredDataI& data) {
 REPORT_ERROR(ErrorManagement::Debug, "INITIALISE");   
    bool ok = DataSourceI::Initialise(data);
    if (ok) {
        ok = data.Read("Mode", mode);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "The Mode shall be specified");
        }
	if(ok)
	{
	    ok = (mode == NI6259DIO_M_IN || mode == NI6259DIO_M_SYNCH_IN || mode == NI6259DIO_M_OUT);
	    if(!ok){
            	REPORT_ERROR(ErrorManagement::ParametersError, "The Mode %d shall be 1 (NI6259DI_M_IN) or 2 (NI6259DI_M_SYNCH_IN) or 3  (NI6259DI_M_OUT)", mode);
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
    if (ok && mode == NI6259DIO_M_OUT) {
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
        if (!ok && mode == NI6259DIO_M_SYNCH_IN) 
	{
            REPORT_ERROR(ErrorManagement::ParametersError, "Period shall be between defined");
        }
    }
    counter = 0; 
    time = static_cast<uint32>(triggerTime * 1E6);
    return ok;
}

bool NI6259DIO_M::SetConfiguredDatabase(StructuredDataI& data) {
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
    if(mode == NI6259DIO_M_IN or mode == NI6259DIO_M_OUT)
    {
        ok = (nOfSignals == 1);
        if(!ok){
            REPORT_ERROR(ErrorManagement::ParametersError, "Only one signal (bits) shall be specified");
        }
    }
    else //NI6259DIO_M_SYNCH_IN
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
        if(mode == NI6259DIO_M_OUT)
	{
	    mutexSem.FastLock(); //dio configurations are possibly shared among instances
            ok = (pxi6259_add_dio_channel(&dioConfiguration[boardId], 0, bitMask) == 0);
            if (!ok) {
                REPORT_ERROR_PARAMETERS(ErrorManagement::ParametersError, "Could not set configuration for device %s", boardId, fullDeviceName);
	    }
     	}
	mutexSem.FastUnLock();
    }
    return ok;
}

bool NI6259DIO_M:: Synchronise()
 {
//The first instance at the first Synchronise will open the channe descriptor
    REPORT_ERROR(ErrorManagement::Debug, "PARTE SYNCHRONIZE");
    if(!openPortDescriptor())
    {
	return false;
    }
    bool ok;
    if(mode == NI6259DIO_M_OUT)
    {
    	mutexSem.FastLock();
       	ok = write(portFileDescriptor[boardId], &port0Bits, sizeof(uint32));
	mutexSem.FastUnLock();
    }
    else
    { 
	mutexSem.FastLock();
	ok = read(portFileDescriptor[boardId], &port0Bits, sizeof(uint32));
	mutexSem.FastUnLock();
	if(clockId != -1)
	{
	    bool prevVal = ((port0Bits & (1 << clockId)) != 0);
	    while(true) {
		mutexSem.FastLock();
		read(portFileDescriptor[boardId], &port0Bits, sizeof(uint32));	
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
    REPORT_ERROR(ErrorManagement::Debug, "FINISCE SYNCHRONIZE");
    return ok;
}



CLASS_REGISTER(NI6259DIO_M, "1.0")



FastPollingMutexSem NI6259DIO_M::mutexSem;
bool NI6259DIO_M::mutexSemInitialized;
int32 NI6259DIO_M::portFileDescriptor[MAX_NI6259_DEVICES];
pxi6259_dio_conf_t NI6259DIO_M::dioConfiguration[MAX_NI6259_DEVICES];



}

