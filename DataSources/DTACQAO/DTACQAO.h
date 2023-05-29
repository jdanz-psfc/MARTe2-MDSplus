/**
 * @file DTACQAO.h
 * @brief Header file for class DTACQAO
 * @date 08/03/2023
 * @author Gabriele Manduchi
 *
 * @warning Unless required by applicable law or agreed to in writing, 
 * software distributed under the Licence is distributed on an "AS IS"
 * basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the Licence permissions and limitations under the Licence.

 * @details This header file contains the declaration of the class DTACQAO
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
 * DTACQAO DataSource shall write at every cycle a given number of AO and DO  
 * channels via UDP communication to D-TAcq AO device. The Parameters ar:
 * NumAOChans : Number of Analof Out Channels
 * NumDOChans : Number of Digital out Modules (32 chans per module)
 * IPAddress: IP affress for UDP send
 * Port : Port for UDP send
 * The DataSource accepts a variable number N of signal: Dac1 to DacN. A check is performed to ensure that N is not greater than
 * the number of available AO channels. NumSamples shall be 1. Only float64 type is supported
 * The data Source accepts a variable number M of  digital outputs: Do1 to DoM. A check is performed to ensure that M is not greater 
 * than NumDO * 32. Both int8 and uint8 types are supported.
 * For both Dac and Do outputs, parameter ChannelId specifies the index in the AO and Do arrays, respectively 
 */

#ifndef DTACQAO_H_
#define DTACQAO_H_

/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/
#include "DataSourceI.h"
#include "ProcessorType.h"
#include "MemoryMapSynchronisedOutputBroker.h"
#include "MessageI.h"
#include "RegisteredMethodsMessageFilter.h"
#include "UDPSocket.h"

#define DTACQAO_AO 1
#define DTACQAO_DO 2

/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/
namespace MARTe {
/**
 
 * */

class DTACQAO: public DataSourceI, public MessageI {
public:
    CLASS_REGISTER_DECLARATION()

    /**
     * @brief Default constructor.
     * @details Initialises all the optional parameters as described in the class description.
     */
    DTACQAO();

    /**
     * @brief Destructor.
     * @details Flushes the file and frees the circular buffer.
     */
    virtual ~DTACQAO();

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
     * @return MemoryMapAsyncOutputBroker if storeOnTrigger == 0, MemoryMapAsyncTriggerOutputBroker otherwise.
     */
    virtual const char8 *GetBrokerName(StructuredDataI &data,
            const SignalDirection direction);

    /**
     * @brief See DataSourceI::GetInputBrokers.
     * @return false.
     */
    virtual bool GetInputBrokers(ReferenceContainer &inputBrokers,
            const char8* const functionName,
            void * const gamMemPtr);

    /**
     * @brief See DataSourceI::GetOutputBrokers.
     * @details If storeOnTrigger == 0 it adds a MemoryMapAsyncOutputBroker instance to
     *  the inputBrokers, otherwise it adds a MemoryMapAsyncTriggerOutputBroker instance to the outputBrokers.
     * @pre
     *   GetNumberOfFunctions() == 1u
     */
    virtual bool GetOutputBrokers(ReferenceContainer &outputBrokers,
            const char8* const functionName,
            void * const gamMemPtr);

    /**
     * @brief Writes the buffer data into the specified file in the specified format.
     * @return true if the data can be successfully written into the file.
     */
    virtual bool Synchronise();

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
     */
    virtual bool SetConfiguredDatabase(StructuredDataI & data);

    /**
     * @brief Flushes the file.
     * @return true if the file can be successfully flushed.
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
     * Offset of each signal in the dataSourceMemory
     */
    uint32 *offsets;

    /**
     * Memory holding all the signals that are to be stored, for each cycle, in the output file.
     */
    char8 *dataSourceMemory;


    /**
     * The asynchronous triggered broker that provides the interface between the GAMs and the output file.
     */
    uint32 numDacs; //Number of DAC boards
    uint32 *numAoChans; //For each declared adc board, the number od AO channels
    uint32 numDos;  //Number of 32 bits digital outputs     
    uint32 totAoChans; //Total Number of AO channels
    uint8 *chanBuf;    //Input buffer
    uint32 numSignals; //Number of actually declared signals
    uint32 *signalIds; //For each declared signal the index (i.e. channel id for ao signals(type = float64), bit id for do bits(type = uint8))
    uint32 *chanOffsets; //Channel offsets in chanBuf (numSignals)
    uint8 *signalTypes; //For each signal: 1-> it is AO  0->it is DO
    StreamString ipAddress; //For  or UDP socket
    UDPSocket *udpSocket;
    char8 *packet;
    int32 port;
    
};
}


/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif /* DTACQAO_H_ */
	
