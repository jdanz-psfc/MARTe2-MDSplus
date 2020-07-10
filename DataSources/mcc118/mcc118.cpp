
/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/
#include "AdvancedErrorManagement.h"
#include "CLASSMETHODREGISTER.h"
#include "mcc118.h"
#include <mdsobjects.h>
#include <stdio.h>
#ifdef MCC_EMULATE
#include <cstdlib>
#include <ctime>
#else
#include "daqhats_utils.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/
namespace MARTe
{

    mcc118::mcc118() : DataSourceI(),
                       MessageI()
    {
        dataBuffer = NULL_PTR(float32 *);
        actNumChannels = 0;
        cpuMask = 0;
        gpioDevice = "/dev/";
        gpioPin = 0;
    }

    mcc118::~mcc118()
    {
        //Free allocated buffers

        if (dataBuffer != NULL_PTR(float32 *))
        {
            GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(dataBuffer));
        }
    }

    bool mcc118::AllocateMemory()
    {
        return true;
    }

    uint32 mcc118::GetNumberOfMemoryBuffers()
    {
        return 1u;
    }

    bool mcc118::GetSignalMemoryBuffer(const uint32 signalIdx, const uint32 bufferIdx, void *&signalAddress)
    {
        bool ok = (dataBuffer != NULL_PTR(float32 *));
        if (ok)
        {
            /*lint -e{613} dataSourceMemory cannot be NULL here*/
            float32 *memPtr = &dataBuffer[signalIdx];
            signalAddress = reinterpret_cast<void *&>(memPtr);
        }
        return ok;
    }

    const char8 *mcc118::GetBrokerName(StructuredDataI &data, const SignalDirection direction)
    {
        const char8 *brokerName = "";
        if (direction == InputSignals)
        {
            brokerName = "MemoryMapSynchronisedInputBroker";
        }
        return brokerName;
    }

    bool mcc118::GetOutputBrokers(ReferenceContainer &inputBrokers, const char8 *const functionName, void *const gamMemPtr)
    {
        return false;
    }

    bool mcc118::IsSupportedBroker(const SignalDirection direction, const uint32 functionIdx, const uint32 functionSignalIdx, const char8 *const brokerClassName)
    {
        return true;
    }

    bool mcc118::GetInputBrokers(ReferenceContainer &inputBrokers, const char8 *const functionName, void *const gamMemPtr)
    {

        bool ok = true;

	printf("GET BROKER\n");
	
        ReferenceT<MemoryMapSynchronisedInputBroker> broker("MemoryMapSynchronisedInputBroker");
        ok = broker.IsValid();
 
        printf("GET BROKER 1 %d\n", ok);   
	if (ok)
        {
            ok = broker->Init(InputSignals, *this, functionName, gamMemPtr);
        }

       printf("GET BROKER 2 %d\n", ok);
       if (ok)
        {
            ok = inputBrokers.Insert(broker);
        }
	printf("GET BROKER ENDED %d\n", ok);
        return ok;
    }

    bool mcc118::Synchronise()
    {
        bool ok = true;

	
//Wait for data; read data and copy 16 floats into dataBuffer
#ifdef MCC_EMULATE
        srand((unsigned int)time(NULL));
        float max_emulated = 10.;
#else
        uint32_t options = OPTS_DEFAULT;
        double value;
        struct gpioevent_data event;
        int32_t ret;
        ret = read(req.fd, &event, sizeof(event));
        if (ret == -1)
        {
            if (errno == -EAGAIN)
            {
                REPORT_ERROR(ErrorManagement::ParametersError,
                             "No GPIO Event available");
                return true;
            }
            else
            {
                REPORT_ERROR(ErrorManagement::ParametersError,
                             "GPIO Event IO returned %d", -errno);
                return false;
            }
        }

        if (ret != sizeof(event))
        {
            REPORT_ERROR(ErrorManagement::ParametersError,
                         "GPIO Event returned wrong amount of data %d != %d", ret, sizeof(event));
            return false;
        }
//        fprintf(stdout, "GPIO EVENT %llu: ", event.timestamp);
//        if (event.id == GPIOEVENT_EVENT_RISING_EDGE) {
#endif
        for (uint32_t i = 0; i < actNumChannels; i++)
        {
#ifdef MCC_EMULATE
	    float32 randVal = float(rand()) / float((RAND_MAX)) * max_emulated;
	    printf("%f\n", randVal);
            dataBuffer[i] = randVal;
#else
            ok = mcc118_a_in_read((i < 8) ? 0 : 1, i % 8 + 1, options, &value);
//            printf("chan =  %d - value = %f\n", i, value);
	    ok = !ok;
	    dataBuffer[i] = (float32)value;
#endif
        }
        return true;
    }

    /*lint -e{715}  [MISRA C++ Rule 0-1-11], [MISRA C++ Rule 0-1-12]. Justification: NOOP at StateChange, independently of the function parameters.*/
    bool mcc118::PrepareNextState(const char8 *const currentStateName, const char8 *const nextStateName)
    {
        return true;
    }

    bool mcc118::Initialise(StructuredDataI &data)
    {

 printf("INITIALIZAZION\n");
      bool ok = DataSourceI::Initialise(data);
        if (ok)
        {
            uint32 cpuMaskIn;
            ok = data.Read("CpuMask", cpuMaskIn);
            if (!ok)
            {
                cpuMask = -1;
            }
            else
            {
                cpuMask = cpuMaskIn; //Not used for now
            }
        }
        uint32 gpioPinIn;
        ok = data.Read("gpioPin", gpioPinIn);
        if (!ok)
        {
            gpioPin = -1;
        }
        else
        {
            gpioPin = gpioPinIn;
        }

        ok = data.Read("GPIODevice", gpioDevice);
        if (!ok)
        {
            REPORT_ERROR(ErrorManagement::ParametersError, "GPIODevice not specified.");
            return ok;
        }

        ok = data.MoveRelative("Signals");
        if (!ok)
        {
            REPORT_ERROR(ErrorManagement::ParametersError, "Signals node Missing.");
            return ok;
        }
        actNumChannels = data.GetNumberOfChildren();
        ok = (actNumChannels > 0 && actNumChannels <= 16);
        if (!ok)
        {
            REPORT_ERROR(ErrorManagement::ParametersError, "Incorrect number %d of signals. Must be 1-16", actNumChannels);
            return ok;
        }
        data.MoveToAncestor(1u);

printf("INITIALIZAZION ENDED\n");
        return ok;
    }

    bool mcc118::SetConfiguredDatabase(StructuredDataI &data)
    {
printf("CONFIGURE %d\n", actNumChannels);
        bool ok = DataSourceI::SetConfiguredDatabase(data);
        //Check signal properties and compute memory
        if (ok)
        { // Check that only one GAM is Connected to the MDSReaderNS
            uint32 auxNumberOfFunctions = GetNumberOfFunctions();
            ok = (auxNumberOfFunctions == 1u);
            if (!ok)
            {
                REPORT_ERROR(ErrorManagement::ParametersError, "Exactly one Function allowed to interact with this mcc118 DataSource. Number of Functions = %u",
                             auxNumberOfFunctions);
            }
        }
        dataBuffer = reinterpret_cast<float32 *>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(actNumChannels * sizeof(float32)));
        memset(dataBuffer, 0, actNumChannels * sizeof(float32));
        if (ok)
        {
            for (uint32 n = 0u; (n < actNumChannels) && ok; n++)
            {
                if (ok)
                {
                    uint32 nElements;
                    GetSignalNumberOfElements(n, nElements);
                    ok = (nElements == 1);
                    if (!ok)
                    {
                        REPORT_ERROR(ErrorManagement::ParametersError,
                                     "Wrong number elements for signal %d: scalar expected", n);
                    }
                }
                if (ok)
                {
                    uint32 nSamples;
                    ok = GetFunctionSignalSamples(InputSignals, 0u, n, nSamples);
                    if (ok)
                    {
                        ok = (nSamples == 1u);
                    }
                    if (!ok)
                    {
                        REPORT_ERROR(ErrorManagement::ParametersError, "The number of samples shall be exactly 1");
                    }
                }
                if (ok)
                {
                    TypeDescriptor currType;
                    currType = GetSignalType(n);
                    ok = (currType == Float32Bit);
                    if (!ok)
                    {
                        REPORT_ERROR(ErrorManagement::ParametersError, "Invalid type for channel %d", n);
                    }
                }
            }
#ifndef MCC_EMULATE
            if (ok)
            {
                //NOTE: not ParametersError
                printf("about to open board 0\n");
                ok = mcc118_open(0);
                ok = !ok;
		printf("\t that returned %d\n", ok);
                if (ok && actNumChannels > 8)
                {
		    printf("about to open board 1\n");
                    ok = mcc118_open(1);
                    ok = !ok;
		    printf("\t that returned %d\n", ok);
                }
                if (!ok)
                {
                    REPORT_ERROR(ErrorManagement::ParametersError, "Could not open MCC118 board(s)");
                }
            }
            if (ok)
            {
                req.lineoffset = gpioPin;
                req.handleflags = GPIOHANDLE_REQUEST_INPUT;
                req.eventflags = GPIOEVENT_REQUEST_RISING_EDGE;
                strcpy(req.consumer_label, "mcc-118-ext-clock");
                fd = open(gpioDevice.Buffer(), 0);
                if (fd == -1)
                {
                    ok = false;
                    REPORT_ERROR(ErrorManagement::ParametersError, "Could not open gpio device %s. Error is %d", gpioDevice.Buffer(), -errno);
                }
            }
            if (ok)
            {
                if (ioctl(fd, GPIO_GET_LINEEVENT_IOCTL, &req) == -1)
                {
                    ok = false;
                    REPORT_ERROR(ErrorManagement::ParametersError, "Could not issue  getemvent opn gpio device %s. Error is %d", gpioDevice.Buffer(), -errno);
                }
            }

#endif
        }
 printf("CONFIGURE ENDED %d\n", ok);
       return ok;
    }

    uint32 mcc118::GetNumberOfBuffers() const
    {
        return 1;
    }

    CLASS_REGISTER(mcc118, "1.0")
} // namespace MARTe
