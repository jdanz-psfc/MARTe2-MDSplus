/**
 * @file StreamOut.h
 * @brief Header file for class StreamOut
 * @date 22/04/2020
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

 * @details This header file contains the declaration of the class StreamOut
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
 */

#ifndef STREAMOUT_H_
#define STREAMOUT_H_

/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/
#include "DataSourceI.h"
#include "ProcessorType.h"
#include "MemoryMapAsyncOutputBroker.h"
#include "MessageI.h"
#include "RegisteredMethodsMessageFilter.h"
#include <mdsobjects.h>

/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/
namespace MARTe {
/**
 
 * */
#define MDSPLUS_STREAM_OUT_MAX_SAMPLES 512
//Support class for packing channels
class StreamManager
{
    struct  BufDescr { 
      uint32 nSamples;
      float32 *samples;
      float32 *times;
      BufDescr *nxt;
    };
    struct HeadDescr {
      char * chanName;
      BufDescr *bufs;
    };
    
    HeadDescr *heads;
    BufDescr *buffers;
    uint32 numStreams;
    uint32 nChans;
    float totTimes[MDSPLUS_STREAM_OUT_MAX_SAMPLES], totSamples[MDSPLUS_STREAM_OUT_MAX_SAMPLES];
    
public:
    StreamManager()
    {
        numStreams = 0;
	nChans = 0;
	heads = NULL_PTR(HeadDescr *);
	buffers = NULL_PTR(BufDescr *);
    }
    ~StreamManager()
    {
	if (heads != NULL_PTR(HeadDescr *)) {
	    GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(heads));
	}
	if(nChans > 0)
	{
	   GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(buffers));
	}
    }
    void init(uint32 nChans)
    {
 	numStreams = 0;
	this->nChans = nChans;
	heads = reinterpret_cast<HeadDescr *>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(nChans * sizeof(HeadDescr)));
	buffers = reinterpret_cast<BufDescr *>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(nChans * sizeof(BufDescr)));
    }
    void registerChannel(uint32 channelIdx, char *chanName)
    {
        uint32 streamIdx;
        for (streamIdx = 0; streamIdx < numStreams && strcmp(heads[streamIdx].chanName, chanName); streamIdx++);
	if(streamIdx == numStreams)
	{
	    heads[streamIdx].bufs = &buffers[channelIdx];
	    heads[streamIdx].chanName = new char[strlen(chanName)+1];
	    strcpy(heads[streamIdx].chanName, chanName);
	    buffers[channelIdx].nxt = 0;
	    numStreams++;
	}
	else
	{
	    BufDescr *currBuf;
	    for(currBuf = heads[streamIdx].bufs; currBuf->nxt; currBuf = currBuf->nxt);
	    currBuf->nxt = &buffers[channelIdx];
	    buffers[channelIdx].nxt = 0;
	}
    }
    
    void reportChannel(uint32 channelIdx, uint32 nSamples, float32 *times, float32 *samples)
    {
	buffers[channelIdx].nSamples = nSamples;
	buffers[channelIdx].times = times;
	buffers[channelIdx].samples = samples;
    }
    
    void sendAll(int shotNumber)
    {
	int currSamples;
	for (uint32 streamIdx = 0; streamIdx < numStreams; streamIdx++)
	{
	    currSamples = 0;
	    for(BufDescr *currBuf = heads[streamIdx].bufs; currBuf; currBuf = currBuf->nxt)
	    {
		for(uint32 currIdx = 0; currIdx < currBuf->nSamples; currIdx++)
		{
		    if(currSamples < MDSPLUS_STREAM_OUT_MAX_SAMPLES)
		    {
			totTimes[currSamples] = currBuf->times[currIdx];
			totSamples[currSamples] = currBuf->samples[currIdx];
			currSamples++;
		    }
		}
	    }
	    MDSplus::EventStream::send(shotNumber, heads[streamIdx].chanName, currSamples, totTimes, totSamples);
	}
    }
};




class StreamOut: public DataSourceI, public MessageI {
public:
    CLASS_REGISTER_DECLARATION()

    /**
     * @brief Default constructor.
     * @details Initialises all the optional parameters as described in the class description.
     * Registers the RPC FlushFile, CloseFile and OpenFile callback functions.
     */
    StreamOut();

    /**
     * @brief Destructor.
     * @details Flushes the file and frees the circular buffer.
     */
    virtual ~StreamOut();

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

    /**
     * @brief Gets the number of post configured buffers in the circular buffer.
     * @return the number of post configured buffers in the circular buffer.
     */
    ErrorManagement::ErrorType sayHello(const int32 param);
    
    ReferenceT<RegisteredMethodsMessageFilter> filter;

     /**
     * The message to send if the Tree is successfully opened.
     */
    ReferenceT<Message> outStepMsg;
   
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
    uint32 numberOfBuffers;
    uint32 cpuMask;
    uint21 stackSize;
    uint32 nOfSignals; 
    uint64 counter;
    uint32 eventDivision; //If not defined, it is set to 1
    uint32 bufSamples;
    uint32 bufIdx;
    uint32 numChannels;
    float32 **streamBuffers;
    uint32 pulseNumber;
    uint32 timeIdx; //Index of time input signal
    uint8 timeStreaming; //If false all the elements of the signals will be displayed at every cycle (Oscilloscope)
    uint32 *numElements; //Valid only if not timeStreaming
    uint32 *numSamples; //Valid only if TimeStreaming
    StreamString *channelNames;
    TypeDescriptor *sigTypes;
    StreamManager streamManager;
    
};
}


/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif /* STREAMOUT_H_ */
	
