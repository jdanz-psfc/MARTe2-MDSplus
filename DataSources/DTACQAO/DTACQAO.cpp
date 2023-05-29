
/**
 * @file DTACQAO.cpp
 * @brief Source file for class DTACQAO
 * @date 20 jan 2023
 * @author Gabriele Manduchi
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
 * the class NI6259ADC (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/
#include "AdvancedErrorManagement.h"
#include "DTACQAO.h"
#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <iostream>
/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

namespace MARTe {
DTACQAO::DTACQAO() :DataSourceI() {
    numDacs = 0; //Number of DAC boards
    numDos = 0;  //Number of 32 bits digital outputs     
    totAoChans = 0; //Total Number of AO channels
    numAoChans = NULL_PTR(uint32 *); //For each declared dac board, the number od AO channels
    totAoChans = 0; //Total Number of AO channels
    chanBuf = NULL_PTR(uint8 *);    //Input buffer
    chanOffsets = NULL_PTR(uint32 *); //Channel offsets in chanBuf (numSignals)
    signalTypes = NULL_PTR(uint8 *);
    packet = NULL_PTR(char8 *);
    numSignals = 0; //Number of actually declared signals
    signalIds =NULL_PTR(uint32 *); //For each declared signal the index (i.e. channel id for ao signals(type = float64), bit id for do bits(type = uint8))
 } 

DTACQAO::~DTACQAO() 
{
    if(chanBuf)
    {
        delete[] chanBuf;
    }
    if(signalIds)
    {
        delete [] signalIds;
    }
    if(signalTypes)
    {
        delete [] signalTypes;
    }
    if(chanOffsets)
    {
        delete[] chanOffsets;
    }
    if(packet)
    {
        delete[]packet;
    }
    if(signalTypes)
    {
        delete[] signalTypes;
    }
}

bool DTACQAO::AllocateMemory() {
    return true;
}

uint32 DTACQAO::GetNumberOfMemoryBuffers() {
    return 1;
}

bool DTACQAO::GetSignalMemoryBuffer(const uint32 signalIdx, const uint32 bufferIdx, void*& signalAddress) {
    bool ok = signalIdx < numSignals;
    if(ok)
    {
        signalAddress = &chanBuf[chanOffsets[signalIdx]];
    }
 
 
 
    return ok;
}


const char8* DTACQAO::GetBrokerName(StructuredDataI& data, const SignalDirection direction) {
    const char8 *brokerName = "MemoryMapSynchronisedOutputBroker";
    return brokerName;
}

bool DTACQAO::GetOutputBrokers(ReferenceContainer& inputBrokers, const char8* const functionName, void* const gamMemPtr) {
    ReferenceT<MemoryMapSynchronisedOutputBroker> broker("MemoryMapSynchronisedOutputBroker");
    bool ok = broker.IsValid();
    if (ok) {
        ok = broker->Init(OutputSignals, *this, functionName, gamMemPtr);
    }
    if (ok) {
        ok = inputBrokers.Insert(broker);
    }

    return ok;
}
bool DTACQAO::IsSupportedBroker(const SignalDirection direction, const uint32 functionIdx, const uint32 functionSignalIdx, const char8* const brokerClassName)
{
     return true;
}

/*lint -e{715}  [MISRA C++ Rule 0-1-11], [MISRA C++ Rule 0-1-12]. Justification: returns false irrespectively of the input parameters.*/
bool DTACQAO::GetInputBrokers(ReferenceContainer& outputBrokers, const char8* const functionName, void* const gamMemPtr) {
    return false;
}

/*lint -e{715}  [MISRA C++ Rule 0-1-11], [MISRA C++ Rule 0-1-12]. Justification: the counter and the timer are always reset irrespectively of the states being changed.*/
bool DTACQAO::PrepareNextState(const char8* const currentStateName, const char8* const nextStateName) {
    return true;
}

bool DTACQAO::Synchronise() {
    memset(packet, 0, totAoChans * sizeof(uint16) + numDos * sizeof(uint32));
    for(uint32 currSig = 0; currSig < numSignals; currSig++)
    {
         if(signalTypes[currSig] == DTACQAO_AO)
        {
            int32 intVal =  round(*(float64 *)&chanBuf[chanOffsets[currSig]] * SHRT_MAX/10.);
            int16 shortVal;
            if(intVal > SHRT_MAX)
                shortVal = SHRT_MAX;
            else if (intVal < SHRT_MIN)
                shortVal = SHRT_MIN;
            else
                shortVal = intVal;
//std::cout << shortVal << "   " << signalIds[currSig] << "    " << numDos << "    " << totAoChans << std::endl;
            *(int16 *)&packet[signalIds[currSig]*sizeof(int16)] = shortVal;
        }
        else
        {
            *(uint32 *)&packet[totAoChans*sizeof(int16)+sizeof(uint32) * (signalIds[currSig]/sizeof(uint32))] |= 
                (*(uint8 *)&chanBuf[chanOffsets[currSig]] << (signalIds[currSig] % sizeof(uint32)));
        }
    }
    uint32 numBytes = (uint32)(totAoChans * sizeof(uint16) + numDos * sizeof(uint32));
    udpSocket->Write((const char8 *)packet, numBytes);
    return true;
}



bool DTACQAO::Initialise(StructuredDataI& data) {
    bool ok = DataSourceI::Initialise(data);
    if (ok) {
        ok = data.Read("NumDac", numDacs);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "The number of ADC modules shall be specified");
        }
    }
    AnyType arrayDescription;
    if (ok) {
        arrayDescription = data.GetType("NumAoChans");
        ok = arrayDescription.GetDataPointer() != NULL_PTR(void *);
        if(!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "Cannot read NumAoChans");
        }
    }
    if(ok)
    {
        if(arrayDescription.GetNumberOfElements(0u)!= numDacs) {
                REPORT_ERROR(ErrorManagement::ParametersError, "The number of input channels shall be an array with as many elements as NumAdc");
                ok = false;
        }
    }
    if(ok)
    {
        numAoChans = new uint32[numDacs];
        Vector<uint32> readVector(numAoChans, numDacs);
        ok = data.Read("NumAoChans", readVector);
        if(!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "Error reading NumAoChans");
        }
    }
    totAoChans = 0;
    for(uint32 i = 0; i < numDacs; i++)
    {
        totAoChans += numAoChans[i]; 
    }
std::cout << "TOT AO CHANS "<< totAoChans << std::endl;

    if (ok) {
       ok = data.Read("NumDo", numDos);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "The number of DIO modules shall be specified");
        }
    }
    if(ok)
    {
         ok = data.Read("IpAddress", ipAddress);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "ACQ400 IP Address for data readout shall be specified");
        }
    }
    if(ok)
    {
         ok = data.Read("Port", port);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "ACQ400 port for data readout shall be specified");
        }
    }
    if (ok) {
        ok = data.MoveRelative("Signals");
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "Could not move to the Signals section");
        }
    }
    numSignals = data.GetNumberOfChildren();
 
    if(ok)
    {
        signalIds = new uint32[numSignals];
        for(uint32 currSig = 0; currSig < numSignals && ok; currSig++)
        {
            data.MoveRelative(data.GetChildName(currSig));
            ok = data.Read("ChannelId", signalIds[currSig]);
            if(!ok) {
                REPORT_ERROR(ErrorManagement::ParametersError, "ChannelId shall be specified for signal %d", currSig);
            }
            data.MoveToAncestor(1u);
        }
        data.MoveToAncestor(1u);
    }
    return ok;
}

bool DTACQAO::SetConfiguredDatabase(StructuredDataI& data) {
    uint32 numAoSignals, numDoSignals;
    bool ok = DataSourceI::SetConfiguredDatabase(data);
    if(ok)
    {
        uint32 nOfFunctions = GetNumberOfFunctions();
        ok = (nOfFunctions == 1u);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "At most one function shall interact with this DataSourceI");
        }
    }
    if (ok) {
        numAoSignals = 0;
        numDoSignals = 0;
        signalTypes = new uint8[numSignals];
        chanOffsets = new uint32[numSignals];
        uint32 currOffset = 0;
        uint32 numSamples;
    }
    printf("\n\nNUM SIGNALS: %d  %d\n", numSignals, ok);
    if(ok)
    {
        uint32 currOffset = 0;
        for (uint32 currSig = 0; currSig < numSignals; currSig++)
        {
            uint32 numSamples = 0u;
            ok = GetFunctionSignalSamples(InputSignals, 0, currSig, numSamples);
            if(!ok)
            {
                numSamples = 1;
                ok = true;
            }
            if(numSamples != 1)
            {
                REPORT_ERROR(ErrorManagement::ParametersError, "All signals shall have one samples");
                ok = false;
                break;
            }
            if(GetSignalType(currSig) == Float64Bit)
            {
                numAoSignals++;
                signalTypes[currSig] = DTACQAO_AO;
                if(signalIds[currSig] < 0 || signalIds[currSig] >= totAoChans)
                {
                    REPORT_ERROR(ErrorManagement::ParametersError, "Invalid ChannelId for AO signal %d: Found %d, expected 0-%d", currSig, signalIds[currSig], totAoChans - 1);
                    ok = false;
                    break;
                }
                chanOffsets[currSig] = currOffset;
                currOffset += sizeof(float64)* numSamples;
            }
            else if(GetSignalType(currSig) == UnsignedInteger8Bit || GetSignalType(currSig) == SignedInteger8Bit )
            {
                numDoSignals++;
                signalTypes[currSig] = DTACQAO_DO;
                if(signalIds[currSig] < 0 || signalIds[currSig] >= numDos * 32)
                {
                    REPORT_ERROR(ErrorManagement::ParametersError, "Invalid ChannelId for DO signal %d: Found %d, expected 0-%d", currSig, signalIds[currSig], numDos * 32 - 1);
                    ok = false;
                    break;
                }
                chanOffsets[currSig] = currOffset;
                currOffset += sizeof(uint8) * numSamples;
            }
            else
            {
                REPORT_ERROR(ErrorManagement::ParametersError, "Invalid Type for signal %d, it shall be either flot64 or int8 or uint8", currSig);
                ok = false;
            }
        }
    }
    if(ok)
    {
        chanBuf = new uint8[numAoSignals * sizeof(float64) + numDoSignals * sizeof(uint8)];
        udpSocket = new UDPSocket();
        if(!udpSocket->Open())
        {
            REPORT_ERROR(ErrorManagement::ParametersError, "Cannot Open UDP socket");
            ok = false;
        }

        if(ok)
        {
            std::cout << "CONNECTING TO "<< ipAddress.Buffer() << "   " << port << std::endl;
            ok = udpSocket->Connect(ipAddress.Buffer(), port);
            if(!ok)
            {
                REPORT_ERROR(ErrorManagement::ParametersError, "Cannot connect to %d port %d", ipAddress.Buffer(), port);
            }
        }
    }
    if(ok)
    {
        uint32 packetLen = totAoChans * sizeof(int16) + numDos * sizeof(uint32);
        packet = new char8[packetLen];
    }
    return ok;
}


CLASS_REGISTER(DTACQAO, "1.0")
}


