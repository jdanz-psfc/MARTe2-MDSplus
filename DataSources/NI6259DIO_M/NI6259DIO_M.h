/**
 * @file NI6259_DIO_M.h
 * @brief Header file for class NI6259_DIO_M.h
 * @date 01/10/2020
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

#ifndef NI6259DIO_M_H_
#define NI6259DIO_M_H_

/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/
#include "DataSourceI.h"
#include "ProcessorType.h"
#include "MemoryMapSynchronisedInputBroker.h"
#include "MessageI.h"
#include "RegisteredMethodsMessageFilter.h"
#include "EventSem.h"
#include <mdsobjects.h>
#include <pxi-6259-lib.h>
#include <pxi6259-enums.h>

/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/
namespace MARTe {
/**
 * @brief A DataSourceI interface which allows Sending and receiving digital dignals via NI6259 
 * device over Port P0. 
 *
 * @details This implementation has two main differences from the original NI6259DIO implementation
 * in MARTe2-components distribution, namely:
 * - It supports also SYNCH_INPUT mode, i.e. synchronization over a clock provided to a given digital
 * input (transition from 0 to 1). An optional trigger provided to another given digital input can be 
 * defined. Transition from 0 to 1 will trigger the clock-driven sequence. The other  modes are INPUT 
 * and OUTPUT. Input and output cannot be intermixed in the same NI6259DIO_M instance 
 *
 * - It supports the sharing of the same physical device for multiple instances of NI6259DIO_M in the same
 * MARTe2 task, provided the no conflicts arise among input and output pins
 *
 *
 * */

#define MAX_NI6259_DEVICES 8
#define NI6259DIO_M_IN 1
#define NI6259DIO_M_SYNCH_IN 2
#define NI6259DIO_M_OUT 3
class NI6259DIO_M: public DataSourceI, public MessageI {
public:
    CLASS_REGISTER_DECLARATION()

    /**
     * @brief Default constructor.
     * @details 
     */
    NI6259DIO_M();

    /**
     * @brief Destructor.
     * @details 
     */
    virtual ~NI6259DIO_M();

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
 
    uint8 mode; //NI6259DIO_M_IN, NI6259DIO_M_SYNC_IN, NI6259DIO_M_OUT
    uint32 port0Bits; //DIO bits
    uint32 boardId;
    StreamString deviceName;
//bit mask (i.e. bits for that input/output)
    uint32 bitMask;

//Clock and trigger management

    int32 clockId;
    int32 triggerId;

//Used only if mode == NI6259DIO_M_SYNCH_IN
    uint32 time;
    uint32 counter;
    int32 boardFileDescriptor;
    float64 triggerTime;
    float64 period;
//Fields shared among NI6259DIO_M instances
    static FastPollingMutexSem mutexSem;
    static bool mutexSemInitialized;
    static int32 portFileDescriptor[MAX_NI6259_DEVICES];
    static pxi6259_dio_conf_t dioConfiguration[MAX_NI6259_DEVICES];

//Private methods to handle a single file and channel descriptor per board so that configurations
// can be updated by different instances of NI6259_DIO_M data source. Called in Synchronise()
    bool openPortDescriptor();
  };
  

}


/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif /* NI6259DIO_M_H_ */
	
