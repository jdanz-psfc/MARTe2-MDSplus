/**
 * @file ResamplerGAM.cpp
 * @brief Source file for class ResamplerGAM
 * @date Jul 30, 2019
 * @author nn
 *

 * @details This source file contains the definition of all the methods for the
 * class ResamplerGAM (public, protected, and private). Be aware that some
 * methods, such as those inline could be defined on the header file, instead.
 */

#define DLL_API

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/

#include "ResamplerGAM.h"
#include "AdvancedErrorManagement.h"
#include <iostream>

/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

namespace MARTe {

ResamplerGAM::ResamplerGAM() : GAM(){
    numSignals = 0;
    resamplingFactor = 1;
    samplingPeriod = 0;
    signalTypes = NULL_PTR(TypeDescriptor *);
    signalNumOfElements = NULL_PTR(uint32 *);


    inputSignals = NULL_PTR(uint8 **);
    outputSignals = NULL_PTR(uint8 **);

    filterEnabled = false;
    outBuffers = NULL_PTR(float64 **);
    inBuffers = NULL_PTR(float64 **);
    lowPassFilters = NULL_PTR(Filter **); 
    runTimeFilters = NULL_PTR(RunTimeFilter *);
}

ResamplerGAM::~ResamplerGAM() {
    if(signalTypes !=  NULL_PTR(TypeDescriptor *))
    {
	GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(signalTypes));
    }

    if(signalNumOfElements !=  NULL_PTR(uint32 *))
    {
	GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(signalNumOfElements));
    }    
    if(inputSignals !=  NULL_PTR(uint8 **))
    {
	GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(inputSignals));
    }
    if(outputSignals !=  NULL_PTR(uint8 **))
    {
	GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(outputSignals));
    }
    if(outBuffers != NULL_PTR(float64 **))
    {
	for(uint32 i = 0; i < numSignals; i++)
	{
	    GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(outBuffers[i]));
	}
	GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(outBuffers));
    }

    if(inBuffers != NULL_PTR(float64 **))
    {
	for(uint32 i = 0; i < numSignals; i++)
	{
	    GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(inBuffers[i]));
	}
	GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(inBuffers));
    }

    if(runTimeFilters != NULL_PTR(RunTimeFilter *))
    {
	GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(runTimeFilters));
    }
    if(lowPassFilters != NULL_PTR(Filter **))
    {
	for(uint32 i = 0; i < numSignals; i++)
	{
	    if(lowPassFilters[i] != NULL_PTR(Filter *))
		FreeFilter(lowPassFilters[i]);
	}
	GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(lowPassFilters));
    }
}

bool ResamplerGAM::Initialise(StructuredDataI & data) {
	
	REPORT_ERROR(ErrorManagement::Debug, "ResamplerGAM: INITIALIZE");
	
	bool ok = GAM::Initialise(data);
	if (!ok) {
		REPORT_ERROR(ErrorManagement::InitialisationError,
					 "Initialization failed.");
		return ok;
	}
	
	/**
	 * Firstly, number of inputs and outputs is read from the configuration file.
	 */
	
	ok = data.MoveRelative("InputSignals");
	if(!ok){
		REPORT_ERROR(ErrorManagement::ParametersError,
					 "Cannot find InputSignals node in configuration file.");
		return ok;
	}
	
	numSignals = data.GetNumberOfChildren();
	
	// Back to the GAM node
	data.MoveToAncestor(1);
	
	ok = data.MoveRelative("OutputSignals");
	if(!ok){
		REPORT_ERROR(ErrorManagement::ParametersError,
					 "Cannot find OutputSignals node in configuration file.");
		return ok;
	}
	
	uint32 numOutputSignals = data.GetNumberOfChildren();
	ok = (numOutputSignals == numSignals);
	if(!ok) {
		REPORT_ERROR(ErrorManagement::ParametersError,
					 "The number of input signals shall be equal to the number of output signals");
		return ok;
	}
	// Back to the GAM node
	data.MoveToAncestor(1);

        ok = data.Read("SamplingPeriod", samplingPeriod);
	if(!ok) {
		REPORT_ERROR(ErrorManagement::ParametersError,
					 "SamplingPeriod shall be specified");
		return ok;
	}
        ok = data.Read("ResamplingFactor", resamplingFactor);
	if(!ok) {
		REPORT_ERROR(ErrorManagement::ParametersError,
					 "ResamplingFactor shall be specified");
		return ok;
	}
	
	uint8 useFilters = 0;
	ok = data.Read("UseLowPassFilter", useFilters);
	if(!ok) useFilters = 0;
	filterEnabled = (useFilters != 0);
        if(filterEnabled)
        {
//First free previous filters if any (shoud not be there)	
    	    if(lowPassFilters != NULL_PTR(Filter **))
    	    {
		for(uint32 i = 0; i < numSignals; i++)
		{
	    	    if(lowPassFilters[i] != NULL_PTR(Filter *))
			FreeFilter(lowPassFilters[i]);
		}
	    }
	    else
	    {
		 lowPassFilters = reinterpret_cast<Filter **>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(numSignals * sizeof(Filter *)));
	    }
	    int numPoles = 10;
	    float32 fc = 1./samplingPeriod;
	    float32 cutOff = 1./(samplingPeriod * 2. * resamplingFactor);
	    if(runTimeFilters == NULL_PTR(RunTimeFilter *))
    	    {
		 runTimeFilters = reinterpret_cast<RunTimeFilter *>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(numSignals * sizeof(RunTimeFilter)));
	    } 
	    for(uint32 i = 0; i < numSignals; i++)
	    {
		lowPassFilters[i] = prepareFilter(cutOff, fc, numPoles);
		initializeRunTimeFilter(&runTimeFilters[i]);
	    }
	}
	else //No filter
	{
   	    if(lowPassFilters != NULL_PTR(Filter **))
    	    {
		for(uint32 i = 0; i < numSignals; i++)
		{
	    	    if(lowPassFilters[i] != NULL_PTR(Filter *))
			FreeFilter(lowPassFilters[i]);
		}
	    }
 	    GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(lowPassFilters));
            lowPassFilters = NULL_PTR(Filter **);
	}

	return ok;
}

bool ResamplerGAM::Setup() {
	
	bool ok;
	
	REPORT_ERROR(ErrorManagement::Debug, "SETUP");
	
	/**
	 * Firstly, types and dimensions of signals are retrieved and checked.
	 */
	signalNumOfElements = reinterpret_cast<uint32 *>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(numSignals * sizeof(uint32)));

	signalTypes = reinterpret_cast<TypeDescriptor *>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(numSignals * sizeof(TypeDescriptor)));

	// Inputs
	for (uint32 sigIdx = 0; sigIdx < numSignals; sigIdx++) {
		
	    signalTypes[sigIdx] = GetSignalType(InputSignals, sigIdx);
		
	    ok = GetSignalNumberOfElements(InputSignals, sigIdx, signalNumOfElements[sigIdx]);
	    if (!ok) {
			
		REPORT_ERROR(ErrorManagement::Exception,
						 "Error in GetSignalNumberOfElements: input signal %i does not exist.", sigIdx);
		return ok;
			
	    }
	    ok = ((signalNumOfElements[sigIdx] % resamplingFactor) == 0);
	    if (!ok) {
			
		REPORT_ERROR(ErrorManagement::Exception,
						 "The number of elements of input signal %i shall be a multiple of ResamplingFactor.", sigIdx);
		return ok;
			
    	    }

	    TypeDescriptor outType = GetSignalType(OutputSignals, sigIdx);
	    ok = (outType == signalTypes[sigIdx]);
	    if (!ok) {
		REPORT_ERROR(ErrorManagement::Exception,
						 "Output type for signal %i shall be the same of the corresponding input.", sigIdx);
		return ok;
	    }

//If output is declared as scalar (numberOfDimensions == 0) then Samples will be the input array size / resFactor. Otherwise, the array dimension shall be the input array size / resFactor.
	    uint32 numOutDimensions;
	    ok = GetSignalNumberOfElements(OutputSignals, sigIdx, numOutDimensions);
	    if (!ok) {
		REPORT_ERROR(ErrorManagement::Exception,
						 "Error in GetSignalNumberOfElements: output signal %i does not exist.", sigIdx);
		return ok;	
	    }
	    if(numOutDimensions > 0)
	    {
	    	uint32 numOutElements;
	    	ok = GetSignalNumberOfElements(OutputSignals, sigIdx, numOutElements);
	    	if (!ok) {
			REPORT_ERROR(ErrorManagement::Exception,
						 "Error in GetSignalNumberOfElements: output signal %i does not exist.", sigIdx);
			return ok;	
	    	}
	    	ok = (signalNumOfElements[sigIdx] == numOutElements * resamplingFactor);
	    	if (!ok) {
			
			REPORT_ERROR(ErrorManagement::Exception,
						 "Output signal %i shall have the  number of elements of the corresponding input divided by ResamplingFactor.", sigIdx);
		return ok;
	     	}
	    }
	    else //Scalar output, check samples
	    {
	    	uint32 numOutSamples;
	    	ok = GetSignalNumberOfSamples(OutputSignals, sigIdx, numOutSamples);
	    	if (!ok) {
			REPORT_ERROR(ErrorManagement::Exception,
						 "Error in GetSignalNumberOfElements: output signal %i does not exist.", sigIdx);
			return ok;	
	    	}
	    	ok = (signalNumOfElements[sigIdx] == numOutSamples * resamplingFactor);
	    	if (!ok) {
			
			REPORT_ERROR(ErrorManagement::Exception,
						 "Output signal %i shall have the  number of samples of the corresponding input elements divided by ResamplingFactor.", sigIdx);
		return ok;
	     	}

	    }

	}


	//Allocate inptus, outputs and compute offsets
	inputSignals = reinterpret_cast<uint8 **>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(numSignals * sizeof(uint8 **)));
	outputSignals = reinterpret_cast<uint8 **>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(numSignals * sizeof(uint8 **)));
	outBuffers = reinterpret_cast<float64 **>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(numSignals * sizeof(float64 **)));
	inBuffers = reinterpret_cast<float64 **>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(numSignals * sizeof(float64 *)));


	for (uint32 sigIdx = 0; sigIdx < numSignals; sigIdx++) 
	{
	    inputSignals[sigIdx] = reinterpret_cast<uint8 *>(GetInputSignalMemory(sigIdx));
	    outputSignals[sigIdx] = reinterpret_cast<uint8 *>(GetOutputSignalMemory(sigIdx));
	    outBuffers[sigIdx] = reinterpret_cast<float64 *>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(numSignals * sizeof(float64)));
	    inBuffers[sigIdx] = reinterpret_cast<float64 *>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(numSignals * sizeof(float64)));
	}
	
	return ok;
}

bool ResamplerGAM::Execute() {
	
	REPORT_ERROR(ErrorManagement::Debug, "EXECUTE");
	
	for (uint32 sigIdx = 0; sigIdx < numSignals; sigIdx++) {
	    if(filterEnabled)
	    {
		for(uint32 elemIdx = 0; elemIdx < signalNumOfElements[sigIdx]; elemIdx++)
		{
		    if (signalTypes[sigIdx]==UnsignedInteger8Bit) {
			inBuffers[sigIdx][elemIdx] = static_cast<float64>( *( ((uint8*)inputSignals[sigIdx]) + elemIdx ) );
		    } else if (signalTypes[sigIdx]==SignedInteger8Bit) {
			inBuffers[sigIdx][elemIdx] = static_cast<float64>( *( ((int8*)inputSignals[sigIdx]) + elemIdx ) );
		    } else if (signalTypes[sigIdx]==UnsignedInteger16Bit) {
			inBuffers[sigIdx][elemIdx] = static_cast<float64>( *( ((uint16*)inputSignals[sigIdx]) + elemIdx ) );
		    } else if (signalTypes[sigIdx]==SignedInteger16Bit) {
			inBuffers[sigIdx][elemIdx] = static_cast<float64>( *( ((int16*)inputSignals[sigIdx]) + elemIdx ) );
		    } else if (signalTypes[sigIdx]==UnsignedInteger32Bit) {
			inBuffers[sigIdx][elemIdx] = static_cast<float64>( *( ((uint32*)inputSignals[sigIdx]) + elemIdx ) );
		    } else if (signalTypes[sigIdx]==SignedInteger32Bit) {
			inBuffers[sigIdx][elemIdx] = static_cast<float64>( *( ((int32*)inputSignals[sigIdx]) + elemIdx ) );
		    } else if (signalTypes[sigIdx]==UnsignedInteger64Bit) {
			inBuffers[sigIdx][elemIdx] = static_cast<float64>( *( ((uint64*)inputSignals[sigIdx]) + elemIdx ) );
		    } else if (signalTypes[sigIdx]==SignedInteger64Bit) {
			inBuffers[sigIdx][elemIdx] = static_cast<float64>( *( ((int64*)inputSignals[sigIdx]) + elemIdx ) );
		    } else if (signalTypes[sigIdx]==Float32Bit) {
			inBuffers[sigIdx][elemIdx] = static_cast<float64>( *( ((float32*)inputSignals[sigIdx]) + elemIdx ) );
		    } else if (signalTypes[sigIdx]==Float64Bit) {
			inBuffers[sigIdx][elemIdx] = static_cast<float64>( *( ((float64*)inputSignals[sigIdx]) + elemIdx ) );
		
		    } else {
				
				REPORT_ERROR(ErrorManagement::Exception,
							 "Error while loading from GAM memory. Signal no. %i: unsupported type.",sigIdx);
			return false;
				
		    }
			
		}
//Filter
		for(uint32 elemIdx = 0; elemIdx < signalNumOfElements[sigIdx]; elemIdx++)
		{
		    outBuffers[sigIdx][elemIdx] = getFiltered(inBuffers[sigIdx][elemIdx], lowPassFilters[sigIdx], &runTimeFilters[sigIdx]);
		}
		for (uint32 elemIdx = 0; elemIdx < signalNumOfElements[sigIdx]/resamplingFactor; elemIdx++) {
		    if (signalTypes[sigIdx] == UnsignedInteger8Bit) {
			*( ((uint8*)outputSignals[sigIdx]) + elemIdx ) = outBuffers[sigIdx][elemIdx * resamplingFactor];
		    } else if (signalTypes[sigIdx]==SignedInteger8Bit) {
			*( ((int8*)outputSignals[sigIdx]) + elemIdx ) = outBuffers[sigIdx][elemIdx * resamplingFactor];	
		    }			
		    else if (signalTypes[sigIdx] == UnsignedInteger16Bit) {
			*( ((uint16*)outputSignals[sigIdx]) + elemIdx ) = outBuffers[sigIdx][elemIdx * resamplingFactor];
		    } else if (signalTypes[sigIdx]==SignedInteger16Bit) {
			*( ((int16*)outputSignals[sigIdx]) + elemIdx ) = outBuffers[sigIdx][elemIdx * resamplingFactor];				
		    }			
		    else if (signalTypes[sigIdx] == UnsignedInteger32Bit) {
			*( ((uint32*)outputSignals[sigIdx]) + elemIdx ) = outBuffers[sigIdx][elemIdx * resamplingFactor];
		    } else if (signalTypes[sigIdx]==SignedInteger32Bit) {
			*( ((int32*)outputSignals[sigIdx]) + elemIdx ) = outBuffers[sigIdx][elemIdx * resamplingFactor];	
		    }			
		    else if (signalTypes[sigIdx] == UnsignedInteger64Bit) {
			*( ((uint64*)outputSignals[sigIdx]) + elemIdx ) = outBuffers[sigIdx][elemIdx * resamplingFactor];
		    } else if (signalTypes[sigIdx]==SignedInteger64Bit) {
			*( ((int64*)outputSignals[sigIdx]) + elemIdx ) = outBuffers[sigIdx][elemIdx * resamplingFactor];
	
		    } else if (signalTypes[sigIdx]==Float32Bit) {
			*( ((float32*)outputSignals[sigIdx]) + elemIdx ) = outBuffers[sigIdx][elemIdx * resamplingFactor];
		    } else if (signalTypes[sigIdx]==Float64Bit) {
			*( ((float64*)outputSignals[sigIdx]) + elemIdx ) = outBuffers[sigIdx][elemIdx * resamplingFactor];
		    }  //No other choiches, already checked before
		}
	    }
	    else //No filter enabled
	    {
		uint32 elemSize;
		if (signalTypes[sigIdx]==UnsignedInteger8Bit || signalTypes[sigIdx]==SignedInteger8Bit) 
		    elemSize = 1;
		else if (signalTypes[sigIdx]==UnsignedInteger16Bit || signalTypes[sigIdx]==SignedInteger16Bit) 
		    elemSize = 2;
		else if (signalTypes[sigIdx]==UnsignedInteger32Bit || signalTypes[sigIdx]==SignedInteger32Bit || signalTypes[sigIdx]==Float32Bit )
		    elemSize = 4;
		else
		    elemSize = 8;
		
		
		for (uint32 elemIdx = 0; elemIdx < signalNumOfElements[sigIdx]/resamplingFactor; elemIdx++) 
		{
		    memcpy(outputSignals[sigIdx] + elemIdx * elemSize, inputSignals[sigIdx] + elemIdx * resamplingFactor * elemSize, elemSize);
		}
	    }
	}//Endfor
	return true;
    }


CLASS_REGISTER(ResamplerGAM, "1.0")

} /* namespace MARTe */
