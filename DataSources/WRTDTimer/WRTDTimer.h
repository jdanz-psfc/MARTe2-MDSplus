/**
 * @file WRTDTimer.h
 * @brief Header file for class WRTDTimer
 * @date 28/9/2022
 * @author Gabriele manduchi
 *
 * @warning Unless required by applicable law or agreed to in writing, 
 * software distributed under the Licence is distributed on an "AS IS"
 * basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the Licence permissions and limitations under the Licence.

 * @details This source file contains the definition of all the methods for
 * the class WRTDTimer (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 * This class uses several  clock methods to generate a clock at a given frequency
 * starting at a given time. The start time will be received by a UDP multicast message
 * and clock genr4ation will start at a given delay from the communicated time.
 * The parameters in configuraution are:
 * - ClockMode: string "CLOCK_TAI", CLOCK_REALTIME", "CLOCK_MONOTONIC", "CLOCK_PTP"
 * - EventName: string. Name (regular expression)of the event bringing start time indication. 
 * - MulticastGroup: string group to which listen for multicast messages
 * - UdpPort: int. Port at which listening for multicast messages
 * - Phase: double. Delay in seconds from the time specified by the received message 
 * - Delay: double. Delay in seconds from the time specified by the received message 
 * - Period: double. clock period in seconds
 * - LeapSeconds: leap seconds. Mandatory, will depend on clock mode
 * - CpuMask: CPU Mask for support thread
 * The genrated signals are
 * - Counter: unsigned int32 (clock counts, MARTe habit, not really required here)
 * - Time: Signed  Int64: relative time (in microseconds, can be negative in case Phase is negative) in respect of start time
* -  Trigger: unsigned int8: trigger set to 1 when time advances
 * - AbsoluteTime: Signed int64: absolute time (in microseconds)
 * 
 */

#ifndef SOURCE_COMPONENTS_DATASOURCES_WRTDTIMER_WRTDTIMER_H_
#define SOURCE_COMPONENTS_DATASOURCES_WRTDTIMER_WRTDTIMER_H_

/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/
#include "DataSourceI.h"
#include "EmbeddedServiceMethodBinderI.h"
#include "EventSem.h"
#include "RealTimeApplication.h"
#include "SingleThreadService.h"
#include <time.h>
#include "wrtd-common.h"

/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/
namespace MARTe {

/**
 * @brief A DataSource which provides a timing source for a MARTe application using WRTD.
 *  
 * @details The signals are identified by their declaration order in the \a Signals sections. This means that if the user needs
 * the last signal all the previous must be declared in the configuration.

 * The configuration syntax is (names are only given as an example):
 * <pre>
 * +Timer = {
 *     Class = WRTDTimer
 *     Delay = -1E6 //Optional, here start clock one second before event time
 *     CPUMask = 0x8 Seprate thread CPU Mask
 *     ClockName = "CLOCK_PTP"
 *     Event = "EVENT_*" //Name (regular expression)of the event bringing start time indication. 
 *     Port = 3 //Port at which listening for multicast messages
 *     Phase = 10 //return 10 us after clock occurrence 
 *     }
 *     Signals = {
 *         Counter = {
 *             Type = uint32 //int32 also supported
 *         }
 *         Time = {
 *             Type = int64 //int32 also supported, relative time from trigger time
 *             Frequency = 1000
 *         }
 *         AbsoluteTime = { //Optional, can be omitted
 *             Type = uint64 //Only type supported
 *         }
  *     }
 * }
 * </pre>
 *
 * @details ExecutionMode can be IndependentThread or RealTimeThread. In the first case a thread is spawned on the provided \a CPUMask and triggers the Synchronise() at every period.
 * If RealTimeThread, the time synchronisation is performed in the same thread scope.
 *
 *
 * @details Follows a description of the signals
 *   - Counter: cycle counter
 *   - Time: delay + phase + Counter*Period
 *   - Trigger: 1 when time advances, 0 otherwise (before receiving the trigger event)
 *   - AbsoluteTime:  absolute time
 *
 */
class WRTDTimer: public DataSourceI, public EmbeddedServiceMethodBinderI {
public:
    CLASS_REGISTER_DECLARATION()
    /**
     * @brief Default constructor
     * @post
     *   Counter = 0
     *   Time = 0
     */
    WRTDTimer ();

    /**
     * @brief Destructor. Stops the EmbeddedThread.
     */
    virtual ~WRTDTimer();

    /**
     * @brief See DataSourceI::AllocateMemory.
     */
    virtual bool AllocateMemory();

    /**
     * @brief See DataSourceI::GetNumberOfMemoryBuffers.
     * @return 1.
     */
    virtual uint32 GetNumberOfMemoryBuffers();

    /**
     * @brief See DataSourceI::GetNumberOfMemoryBuffers.
     */
    virtual bool GetSignalMemoryBuffer(const uint32 signalIdx,
                                       const uint32 bufferIdx,
                                       void *&signalAddress);

    /**
     * @brief See DataSourceI::GetNumberOfMemoryBuffers.
     * @details Only InputSignals are supported.
     * @return MemoryMapSynchronisedInputBroker if frequency > 0, MemoryMapInputBroker otherwise.
     */
    virtual const char8 *GetBrokerName(StructuredDataI &data,
                                       const SignalDirection direction);

    /**
     * @brief Waits on an EventSem for the period given by 1/Frequency to elapse on Execute.
     * @return true if the semaphore is successfully posted.
     */
    virtual bool Synchronise();

    /**
     * @brief Callback function for an EmbeddedThread.
     * @details Sleeps (Busy or Default) for the period given by 1/Frequency and post an EventSem which is waiting on
     *  the Synchronise method.
     * @param[in] info not used.
     * @return NoError if the EventSem can be successfully posted.
     */
    virtual ErrorManagement::ErrorType Execute(ExecutionInfo & info);

    /**
     * @brief Resets the counter and the timer to zero and starts the EmbeddedThread.
     * @details See StatefulI::PrepareNextState. Starts the EmbeddedThread (if it was not already started) and loops
     * on the ExecuteMethod.
     * @return true if the EmbeddedThread can be successfully started.
     */
    virtual bool PrepareNextState(const char8 * const currentStateName,
                                  const char8 * const nextStateName);

    /**
     * @brief Initialises the LinuxTimer
     * @param[in] data configuration in the form:
     * +Timer = {
     *     Class = LinuxTimer
     *     Phase = 100u
     *     ClockName = "CLOCK_PTP"
     *     Event = "TRIG_EVENT" 
     *     Port = 3 
     *     
     *     Signals = {
     *         Counter = {
     *             Type = uint32 //int32 uint64 int64 also supported
     *         }
     *         Time = {
     *             Type = int64 //int32 also supported
     *             Frequency = 1000
     *         }
     *         Trigger = {
     *             Type = uint8
     *         }
     *         AbsoluteTime = { //Optional
     *             Type = uint64 //Only uint64 supported
     *     }
     * }
     */
    virtual bool Initialise(StructuredDataI & data);

    /**
     * @brief Verifies that signals and type
     * @details Verifies at least two, and art most three signals are set; that the signals are
     * are of the expected type and Frequency > 0 was set in one of the two signals.
     * @param[in] data see DataSourceI::SetConfiguredDatabase
     * @return true if the rules above are met.
     */
    virtual bool SetConfiguredDatabase(StructuredDataI & data);

    /**
     * @brief Gets the affinity of the thread which is going to be used to asynchronously wait for the time to elapse.
     * @return the affinity of the thread which is going to be used to asynchronously wait for the time to elapse.
     */
    const ProcessorType& GetCPUMask() const;

    /**
     * @brief Gets the stack size of the thread which is going to be used to asynchronously wait for the time to elapse.
     * @return the stack size of the thread which is going to be used to asynchronously wait for the time to elapse.
     */
    uint32 GetStackSize() const;


    /**
    * @brief Purges the DataSource
    */
    //virtual void Purge(ReferenceContainer &purgeList);
   
    void handleTimeMessage();

private:

    /**
     * @brief The two supported sleep natures.trigger
     */
    
    /**
     * @brief Current signals
     */
    uint32 leapSeconds;
    uint32 counter;
    char8 time[8];
    uint8 trigger;
    uint64 absoluteTime;
    bool time32Bits;
    /**
     * @brief The semaphore for the synchronisation between the EmbeddedThread and the Synchronise method.
     */
    EventSem synchSem;

    /**
     * @brief The EmbeddedThread where the Execute method waits for the period to elapse.
     */
    SingleThreadService executor;

    
    /**
     * @brief True if the event has been received
     */
    bool messageReceived;

    /**
     * @brief The affinity of the thread that asynchronously generates the time.
     */
    ProcessorType cpuMask;

    
    StreamString eventName;
    StreamString clockName;
    StreamString multicastGroup;
    float64 phase;
    float64 delay;
    float64 period;
    float64 startTime;
    uint32 udpPort;
    bool stopped;
    //Called by a separate thread waiting for the message
     void setStartTime(float64 startTime)
    {
        this->startTime = startTime;
        messageReceived = true;
    }

    // Support WRTD functions
    uint32 wrtdGetClockId(const char *clock_name);
    int32 wrtdWaitUntil(uint32 clock_id, float64 until, uint32 verbose);
    float64 wrtdGetTime(const char *group, unsigned int port, const char *event_regex, unsigned int clock_id, double delay, int leapseconds, unsigned int verbose);
   };
}

/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif /* SOURCE_COMPONENTS_DATASOURCES_WRTDTIMER_WRTDTIMER_H_ */

