/**
 * @file DTACQAI.h
 * @brief Header file for class DTACQAI.h
 * @date 20 Jan 2023
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

 * @details This header file contains the declaration of the class NI6259_DIO_M
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
 */

#ifndef DTACQAI_H_
#define DTACQAI_H_

/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/
#include "DataSourceI.h"
#include "ProcessorType.h"
#include "MemoryMapSynchronisedInputBroker.h"
#include "EventSem.h"
#include "SingleThreadService.h"
#include "UDPSocket.h"
#include "TCPSocket.h"

/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/
namespace MARTe {
/**
 * @brief A DataSourceI interface which allows reading Analog Inputs fro D-TACQ systems
 *
 * @details This DataSource Listens either to a TCP (if bulk mode selected) or UDP (if realtime mode selected) socket
 * reading raw data from the AI modules defined in the ACQ400 system and possibly the digital inputs.
 * Its configuration parameters are the following:
 * - Mode : Either BULK or REALTIME
 * - NumAdc: number of ADC modules included in acquisition
 * - AdcChans: number of channels for each ADC (array)
 * - NumDI: Number of Digital Input Modules. A 32 bit int is acquired for each module
 * - CalGains: calibration gain for each ADC channel (array)
 * - CalOffsets: Calibration offset for each ADC channel (array)
 * The DataSource accepts a variable number N of signal: Adc1 to AdcN. A check is performed to ensure that N is not greater than
 * the number of available AI channels. NumSamples will be the same for all AI and DI channels and can be greater than 1 only for BULK mode.
 * Only float64 type is supported
 * The data Source accepts a variable number M of  digital inputs: Di1 to DiM. A check is performed to ensure that M is not greater 
 * than NumDI*32. Both int8 and uint8 types are supported.
 *
 *
 * */
#define DTACQAI_BULK 1
#define DTACQAI_REALTIME 2
#define DTACQAI_AI 1
#define DTACQAI_DI 2
#define BUF_SAMPLE_FACTOR 500
class DTACQAI: public DataSourceI, public EmbeddedServiceMethodBinderI {
public:
    CLASS_REGISTER_DECLARATION()

    /**
     * @brief Default constructor.
     * @details 
     */
    DTACQAI();

    /**
     * @brief Destructor.
     * @details 
     */
    virtual ~DTACQAI();

    /**
     * @brief See DataSourceI::AllocateMemory. NOOP.
     * @return true.
     */
    virtual bool AllocateMemory();

    /**
     * @brief See DataSourceI::GetNumberOfMemoryBuffers.
     * @return 1.
     */
    virtual uint32 GetNumberOfMemoryBuffers();

    /**
     * @brief See DataSourceI::GetSignalMemoryBuffer.
     * @pre
     *   SetConfiguredDatabase
     */
    virtual bool GetSignalMemoryBuffer(const uint32 signalIdx,
            const uint32 bufferIdx,
            void *&signalAddress);

    /**
     * @brief See DataSourceI::GetBrokerName.
     * @details Only OutputSignals are supported.
     * @return MemoryMapSynchInputBroker.
     */
    virtual const char8 *GetBrokerName(StructuredDataI &data,
            const SignalDirection direction);

    /**
     * @brief See DataSourceI::GetInputBrokers.
     * @return MemoryMapSynchInputBroker.
     */
    virtual bool GetInputBrokers(ReferenceContainer &inputBrokers,
            const char8* const functionName,
            void * const gamMemPtr);

    /**
     * @brief See DataSourceI::GetOutputBrokers.
     * @retrun false
     */
    virtual bool GetOutputBrokers(ReferenceContainer &outputBrokers,
            const char8* const functionName,
            void * const gamMemPtr);

    /**
     * @brief Wait and copy for data in circular buffer into data buffer.
     * @return true if the data has been received
     */
    virtual bool Synchronise();

    /**
     * @brief Callback function for the EmbeddedThread that reads data from this ADC board.
     * @details Reads data from all the configured ADC channels and posts the synchronisation semaphore.
     * @return false if the synchronisation semaphore cannot be posted. Note that failure to read from the ADC will not
     * return an error as the reading operation will be retried forever.
     */
    virtual ErrorManagement::ErrorType Execute(ExecutionInfo & info);


    /**
     * @brief See DataSourceI::PrepareNextState. NOOP.
     * @return true.
     */
    virtual bool PrepareNextState(const char8 * const currentStateName,
            const char8 * const nextStateName);

    /**
     * @brief Loads and verifies the configuration parameters detailed in the class description.
     * @return true if all the mandatory parameters are correctly specified and if the specified optional parameters have valid values.
     */
    virtual bool Initialise(StructuredDataI & data);

    /**
     * @brief Final verification of all the parameters and activation of receiver thread
     * @details This method verifies that all the parameters 
     *  are valid and consistent with the parameters set during the initialisation phase.
     * In particular the following conditions shall be met:
     * - The number of dimension of all the signals is zero.
     * - At least one signal is set.
     * @return true if all the parameters are valid and if the thread is successfully opened.
     */
    virtual bool SetConfiguredDatabase(StructuredDataI & data);

    /**
     * @brief Check if the brober is supported
     * @return true if the brober is supported
     */
    virtual bool IsSupportedBroker(const SignalDirection direction, const uint32 functionIdx, 
				   const uint32 functionSignalIdx, const char8* const brokerClassName);

    /**
     * @brief Gets the number of buffers in the circular buffer.
     * @return the number of buffers in the circular buffer.
     */
    uint32 GetNumberOfBuffers() const;
   
private:

     /**
     * Memory holding all the signals that are to be stored, for each cycle, in the output file.
     */
 
    uint8 mode;   //DTACQAI_BULK or DTACQAI_REALTIME
    uint32 numAdcs; //Number of ADC boards
    int32 *numAiChans; //For each declared adc board, the number od AI channels
    uint32 numDis;  //Number of 32 bits digital inputs     
    uint32 totAiChans; //Total Number of AI channels
    int16 *aiChans;  //Converted AI input channels (BUF_SAMPLE_FACTOR * numSamples * totAiChannels)
    uint32 *diChans;  //DI input channels (BUF_SAMPLE_FACTOR * numSamples)
    uint32 *counters;  //counters (numSamples)
    uint32 *times;  //times (numSamples)
    uint32 *chanOffsets; //Channel offsets in chanBuf (numSignals)
    uint8 *chanBuf;    //Output buffer
    uint32 numSignals; //Number of actually declared signals
    uint32 *signalIds; //For each declared signal the index (i.e. channel id for ai signals(type = float64), bit id for di bits(type = uint8))
    uint32 *signalTypes; //For each declared signal the type (DTACQAI_BULK or DTACKAI_REALTIME)
    uint32 numSamples;  //Declared number of output samples
    uint32 numBufSamples; //Number of internal (circular) buffer samples (BUF_SAMPLE_FACTOR * numSamples)
    uint32 inBufIdx;      //Internal input sample idx (idx of next free sample in the circular buffer)
    uint32 outBufIdx;      //Internal Output sample idx (idx of first sample not yet read in the circular buffer)
    StreamString ipAddress; //For TCP (BULK) or UDP (RREALTIME) socket
    uint32 port;
    uint32 cpuMask;    //CPU Mask for detached thread listening to socket
    uint32 packetLen;  //Length of the packet received by D-TACQ system
    float64 *calOffsets;
    float64 *calGains;
    UDPSocket *udpSocket;
    TCPSocket *tcpSocket;
    EventSem synchSem;  
    char8 *packet;
    float64 triggerTime;
    uint32 startTime;
    char8 samplesConsumed;
    uint32 freqDivision;
     /**
     * The EmbeddedThread where the Execute method waits for the ADC data to be available.
     */
    SingleThreadService executor;
    /**
     * Semaphore associated with the counter reset
     */
    FastPollingMutexSem bufMux;

    uint8 firstPacket;
    uint32 lastCounter;
    uint32 spadSize;
    
  };
  

}


/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif /* DTACQAI_H_ */
	
