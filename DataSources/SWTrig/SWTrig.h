/**
 * @file FileWriter.h
 * @brief Header file for class FileWriter
 * @date 11/08/2017
 * @author Andre' Neto
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

 * @details This header file contains the declaration of the class FileWriter
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
 */

#ifndef SWTRIG_H_
#define SWTRIG_H_

/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/
#include "DataSourceI.h"
#include "ProcessorType.h"
#include "MemoryMapSynchronisedInputBroker.h"
#include "EmbeddedServiceMethodBinderI.h"
#include "SingleThreadService.h"
#include "MessageI.h"
#include "EventSem.h"
#include <mdsobjects.h>

/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/
namespace MARTe {
/**
 * @brief A DataSourceI interface which synchornizes clock generation to the reception of a MDSPlus event 
 * events.
 *
 * @details This Data Source is similar to LinuxTimes except for the fact that clock generation is started upon the reception 
 * of a MDSplus event. In addition, up to 8 triggers can be geneared, each associated with the reception of a given MDSplus event. 
 *
 * */
#define MAX_TRIGGER_OUTPUTS 8

class SWTrigEvent;
 
class SWTrig: public DataSourceI, public MessageI, public EmbeddedServiceMethodBinderI  {
public:
    CLASS_REGISTER_DECLARATION()

    /**
     * @brief Default constructor.
     * @details 
     */
    SWTrig();

    /**
     * @brief Destructor.
     * @details 
     */
    virtual ~SWTrig();

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
    void enableTrigger(int32 idx);

   virtual ErrorManagement::ErrorType Execute(ExecutionInfo & info);
  
private:
   /**
     * Current counter and timer
     */
    uint32 cycles;
    int32 time;

    bool clockStarted;
    /**
     * The semaphore for starting clock generation.
     */
    EventSem startSem;

    /**
     * The semaphore for the synchronisation between the EmbeddedThread and the Synchronise method.
     */
    EventSem synchSem;

    /**
     * The EmbeddedThread where the Execute method waits for the period to elapse.
     */
    SingleThreadService executor;

    /**
     * HighResolutionTimer::Counter() value after the last Sleep.
     */
    uint64 lastTimeTicks;

    /**
     * Sleeping period in units of ticks.
     */
    uint64 sleepTimeTicks;

    /**
     * Sleeping period.
     */
    uint32 timerPeriodUsecTime;

    enum TrigStates {NotTriggered, DelayingTrigger, Triggering, Triggered} trigState[MAX_TRIGGER_OUTPUTS];
    uint32 numTriggers;
    uint8 triggers[MAX_TRIGGER_OUTPUTS];  
    uint32 waitCycles[MAX_TRIGGER_OUTPUTS];
    uint32 waitCounter[MAX_TRIGGER_OUTPUTS];
    uint32 trigCounter[MAX_TRIGGER_OUTPUTS];
    uint32 trigCycles[MAX_TRIGGER_OUTPUTS];
    StreamString trigEventNames[MAX_TRIGGER_OUTPUTS];
    StreamString startEventName;
    SWTrigEvent *trigEvent[MAX_TRIGGER_OUTPUTS];
    SWTrigEvent *startEvent;
    float32 triggerTime;
    float32 frequency;
 };

 class SWTrigEvent: public MDSplus::Event
 {
   SWTrig *swTrig;
   uint32 idx;
 public:
   SWTrigEvent(char *name, uint32 idx, SWTrig *swTrig) : Event(name), swTrig(swTrig) , idx(idx){}
   void run()
   {
	swTrig->enableTrigger(idx);
   }
   
 };
  

}


/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif /* SWTRIG_H_ */
	
