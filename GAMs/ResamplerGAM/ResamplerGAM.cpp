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
    signalNumOfSamples = NULL_PTR(uint32 *);


    inputSignals = NULL_PTR(uint8 **);
    outputSignals = NULL_PTR(uint8 **);

    outBuffers = NULL_PTR(float64 **);
    inBuffers = NULL_PTR(float64 **);
    lowPassFilter = NULL_PTR(Filter *); 
    runTimeFilters = NULL_PTR(RunTimeFilter **);
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
    if(signalNumOfSamples !=  NULL_PTR(uint32 *))
    {
        GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(signalNumOfSamples));
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

    if(runTimeFilters != NULL_PTR(RunTimeFilter **))
    {
        for(uint32 i = 0; i < numSignals; i++)
        {
            GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(runTimeFilters[i]));
        }
        GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(runTimeFilters));
    }
    if(lowPassFilter != NULL_PTR(Filter *))
    {
        FreeFilter(lowPassFilter);
    }
}

bool ResamplerGAM::Initialise(StructuredDataI & data) {
	
	//REPORT_ERROR(ErrorManagement::Debug, "ResamplerGAM: INITIALIZE");
	
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

        uint8 useFilters = 0;
        ok = data.Read("ResampleMode", resampleMode);
        if(!ok) resampleMode = JUST_PICK;
        ok = data.Read("SamplingPeriod", samplingPeriod);
	if(!ok && resampleMode == PICK_FILTERED) {
		REPORT_ERROR(ErrorManagement::ParametersError,
					 "SamplingPeriod shall be specified when filter is used");
		return ok;
	}
        ok = true;
	if(resampleMode == PICK_FILTERED)
        {
	    int numPoles = 10;
	    float32 fc = 1./samplingPeriod;
	    float32 cutOff = 1./(samplingPeriod * 2. * resamplingFactor);
            if(lowPassFilter != NULL_PTR(Filter *))
                FreeFilter(lowPassFilter);
            lowPassFilter = prepareFilter(cutOff, fc, numPoles);
	}
	else //No filter
	{
   	    if(lowPassFilter != NULL_PTR(Filter *))
    	    {
                FreeFilter(lowPassFilter);
	    }
            lowPassFilter = NULL_PTR(Filter *);
	}

	return true;
}

bool ResamplerGAM::Setup() {
	
	bool ok;
	
	//REPORT_ERROR(ErrorManagement::Debug, "SETUP");
	
	/**
	 * Firstly, types and dimensions of signals are retrieved and checked.
	 */
        signalNumOfElements = reinterpret_cast<uint32 *>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(numSignals * sizeof(uint32)));
        signalNumOfSamples = reinterpret_cast<uint32 *>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(numSignals * sizeof(uint32)));

	signalTypes = reinterpret_cast<TypeDescriptor *>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(numSignals * sizeof(TypeDescriptor)));

	// Inputs
        resamplingFactor = 0;
	for (uint32 sigIdx = 0; sigIdx < numSignals; sigIdx++) {
		
	    signalTypes[sigIdx] = GetSignalType(InputSignals, sigIdx);
		
            ok = GetSignalNumberOfElements(InputSignals, sigIdx, signalNumOfElements[sigIdx]);
            if (!ok) {
                        
                REPORT_ERROR(ErrorManagement::Exception,
                                                 "Error in GetSignalNumberOfElements: input signal %i does not exist.", sigIdx);
                return ok;
                        
            }
            ok = GetSignalNumberOfSamples(InputSignals, sigIdx, signalNumOfSamples[sigIdx]);
            if (!ok) {
                        
                REPORT_ERROR(ErrorManagement::Exception,
                                                 "Error in GetSignalNumberOfSamples: input signal %i does not exist.", sigIdx);
                return ok;
                        
            }

	    TypeDescriptor outType = GetSignalType(OutputSignals, sigIdx);
	    ok = (outType == signalTypes[sigIdx]);
	    if (!ok) {
		REPORT_ERROR(ErrorManagement::Exception,
						 "Output type for signal %i shall be the same of the corresponding input.", sigIdx);
		return ok;
	    }
//The number of elements for the output signal shall be the same of the correspo nding input signal
//The number of samples for the output shall be the number of the samples for the inpp=ut/resampling factor
	    uint32 numOutElements;
	    ok = GetSignalNumberOfElements(OutputSignals, sigIdx, numOutElements);
	    if (!ok) {
		REPORT_ERROR(ErrorManagement::Exception,
						 "Error in GetSignalNumberOfElements: output signal %i does not exist.", sigIdx);
		return ok;	
	    }
	    if(numOutElements != signalNumOfElements[sigIdx])
            if (!ok) {
                REPORT_ERROR(ErrorManagement::Exception,
                                                 "The number of output elements for signal  %i shall be the same for the corresponding input.", sigIdx);
                return ok;      
            }
            uint32 numOutSamples;
            ok = GetSignalNumberOfSamples(OutputSignals, sigIdx, numOutSamples);
            if (!ok) {
                    REPORT_ERROR(ErrorManagement::Exception,
                                              "Error in GetSignalNumberOfSamples: output signal %i does not exist.", sigIdx);
                    return ok;	
            }
            if(resamplingFactor == 0)
            {
                if((signalNumOfSamples[sigIdx] % numOutSamples) != 0)
                {
                    
                    REPORT_ERROR(ErrorManagement::Exception,
                                              "Output signal %i shall have the  number of samples that is a divisor of the number of input samples", sigIdx);
                    return false;
                }
                resamplingFactor = signalNumOfSamples[sigIdx]/numOutSamples;
            }
            else
            {
                ok = (signalNumOfSamples[sigIdx] == numOutSamples * resamplingFactor);
                if (!ok) {
                    
                    REPORT_ERROR(ErrorManagement::Exception,
                                              "Output signal %i shall have the  number of samples of the corresponding input divided by ResamplingFactor.", sigIdx);
                    return ok;
                }
            }
	}


	//Allocate inptus, outputs and compute offsets
	inputSignals = reinterpret_cast<uint8 **>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(numSignals * sizeof(uint8 **)));
	outputSignals = reinterpret_cast<uint8 **>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(numSignals * sizeof(uint8 **)));
	outBuffers = reinterpret_cast<float64 **>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(numSignals * sizeof(float64 **)));
	inBuffers = reinterpret_cast<float64 **>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(numSignals * sizeof(float64 *)));
        if(resampleMode == PICK_FILTERED)
            runTimeFilters = reinterpret_cast<RunTimeFilter **>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(numSignals * sizeof(RunTimeFilter *)));
	for (uint32 sigIdx = 0; sigIdx < numSignals; sigIdx++) 
	{
	    inputSignals[sigIdx] = reinterpret_cast<uint8 *>(GetInputSignalMemory(sigIdx));
	    outputSignals[sigIdx] = reinterpret_cast<uint8 *>(GetOutputSignalMemory(sigIdx));
 	    outBuffers[sigIdx] = reinterpret_cast<float64 *>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(signalNumOfElements[sigIdx] * signalNumOfSamples[sigIdx] * sizeof(float64)));
	    inBuffers[sigIdx] = reinterpret_cast<float64 *>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(signalNumOfElements[sigIdx] * signalNumOfSamples[sigIdx] * sizeof(float64)));

            if(resampleMode == PICK_FILTERED)
            {
                runTimeFilters[sigIdx] = reinterpret_cast<RunTimeFilter *>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(signalNumOfElements[sigIdx]  * sizeof(RunTimeFilter)));
                for(uint32 i = 0; i < signalNumOfElements[sigIdx]; i++)
                {
                    initializeRunTimeFilter(&runTimeFilters[sigIdx][i]);
                }
            }
        }
	return ok;
}

bool ResamplerGAM::Execute() {
	
	//REPORT_ERROR(ErrorManagement::Debug, "EXECUTE");
	
	for (uint32 sigIdx = 0; sigIdx < numSignals; sigIdx++) 
        {
	    if(resampleMode == PICK_FILTERED || resampleMode == PICK_AVERAGE)
	    {
                for(uint32 sampleIdx = 0; sampleIdx < signalNumOfSamples[sigIdx]; sampleIdx++)
                {
                    for(uint32 elemIdx = 0; elemIdx < signalNumOfElements[sigIdx]; elemIdx++)
                    {
                        if (signalTypes[sigIdx]==UnsignedInteger8Bit) {
                            inBuffers[sigIdx][sampleIdx * signalNumOfElements[sigIdx] + elemIdx] = static_cast<float64>( *( ((uint8*)inputSignals[sigIdx])+sampleIdx * signalNumOfElements[sigIdx]+elemIdx ) );
                        } else if (signalTypes[sigIdx]==SignedInteger8Bit) {
                            inBuffers[sigIdx][sampleIdx * signalNumOfElements[sigIdx] + elemIdx] = static_cast<float64>( *( ((int8*)inputSignals[sigIdx])+sampleIdx * signalNumOfElements[sigIdx] + elemIdx ) );
                        } else if (signalTypes[sigIdx]==UnsignedInteger16Bit) {
                            inBuffers[sigIdx][sampleIdx * signalNumOfElements[sigIdx] + elemIdx] = static_cast<float64>( *( ((uint16*)inputSignals[sigIdx])+sampleIdx * signalNumOfElements[sigIdx] + elemIdx ) );
                        } else if (signalTypes[sigIdx]==SignedInteger16Bit) {
                            inBuffers[sigIdx][sampleIdx * signalNumOfElements[sigIdx] + elemIdx] = static_cast<float64>( *( ((int16*)inputSignals[sigIdx])+sampleIdx * signalNumOfElements[sigIdx] + elemIdx ) );
                        } else if (signalTypes[sigIdx]==UnsignedInteger32Bit) {
                            inBuffers[sigIdx][sampleIdx * signalNumOfElements[sigIdx] + elemIdx] = static_cast<float64>( *( ((uint32*)inputSignals[sigIdx])+sampleIdx * signalNumOfElements[sigIdx] + elemIdx ) );
                        } else if (signalTypes[sigIdx]==SignedInteger32Bit) {
                            inBuffers[sigIdx][sampleIdx * signalNumOfElements[sigIdx] + elemIdx] = static_cast<float64>( *( ((int32*)inputSignals[sigIdx])+sampleIdx * signalNumOfElements[sigIdx] + elemIdx ) );
                        } else if (signalTypes[sigIdx]==UnsignedInteger64Bit) {
                            inBuffers[sigIdx][sampleIdx * signalNumOfElements[sigIdx] + elemIdx] = static_cast<float64>( *( ((uint64*)inputSignals[sigIdx])+sampleIdx * signalNumOfElements[sigIdx] + elemIdx ) );
                        } else if (signalTypes[sigIdx]==SignedInteger64Bit) {
                            inBuffers[sigIdx][sampleIdx * signalNumOfElements[sigIdx] + elemIdx] = static_cast<float64>( *( ((int64*)inputSignals[sigIdx])+sampleIdx * signalNumOfElements[sigIdx] + elemIdx ) );
                        } else if (signalTypes[sigIdx]==Float32Bit) {
                            inBuffers[sigIdx][sampleIdx * signalNumOfElements[sigIdx] + elemIdx] = static_cast<float64>( *( ((float32*)inputSignals[sigIdx])+sampleIdx * signalNumOfElements[sigIdx] + elemIdx ) );
                        } else if (signalTypes[sigIdx]==Float64Bit) {
                            inBuffers[sigIdx][sampleIdx * signalNumOfElements[sigIdx] + elemIdx] = static_cast<float64>( *( ((float64*)inputSignals[sigIdx])+sampleIdx * signalNumOfElements[sigIdx] + elemIdx ) );
                    
                        } else {
                                    
                                    REPORT_ERROR(ErrorManagement::Exception,
                                                            "Error while loading from GAM memory. Signal no. %i: unsupported type.",sigIdx);
                            return false;
                                    
                        }
			
                    }
                }
                if(resampleMode == PICK_FILTERED)
                {
                    for(uint32 sampleIdx = 0; sampleIdx < signalNumOfSamples[sigIdx]; sampleIdx++)
                    {
                        for(uint32 elemIdx = 0; elemIdx < signalNumOfElements[sigIdx]; elemIdx++)
                        {
                            outBuffers[sigIdx][sampleIdx * signalNumOfElements[sigIdx] + elemIdx] = getFiltered(inBuffers[sigIdx][sampleIdx * signalNumOfElements[sigIdx] + elemIdx], lowPassFilter, &runTimeFilters[sigIdx][elemIdx]);
                        }
                    }
                    for (uint32 sampleIdx = 0; sampleIdx < signalNumOfSamples[sigIdx]/resamplingFactor; sampleIdx++) 
                    {
                        for (uint32 elemIdx = 0; elemIdx < signalNumOfElements[sigIdx]; elemIdx++) 
                        {
                            if (signalTypes[sigIdx] == UnsignedInteger8Bit) {
                                *( ((uint8*)outputSignals[sigIdx]) + sampleIdx * signalNumOfElements[sigIdx] + elemIdx ) = outBuffers[sigIdx][sampleIdx * resamplingFactor * signalNumOfElements[sigIdx] + elemIdx];
                            } else if (signalTypes[sigIdx]==SignedInteger8Bit) {
                                *( ((int8*)outputSignals[sigIdx]) + sampleIdx * signalNumOfElements[sigIdx] + elemIdx ) = outBuffers[sigIdx][sampleIdx * resamplingFactor * signalNumOfElements[sigIdx] + elemIdx];	
                            }			
                            else if (signalTypes[sigIdx] == UnsignedInteger16Bit) {
                                *( ((uint16*)outputSignals[sigIdx]) + sampleIdx * signalNumOfElements[sigIdx] + elemIdx ) = outBuffers[sigIdx][sampleIdx * resamplingFactor * signalNumOfElements[sigIdx] + elemIdx];
                            } else if (signalTypes[sigIdx]==SignedInteger16Bit) {
                                *( ((int16*)outputSignals[sigIdx]) + sampleIdx * signalNumOfElements[sigIdx] + elemIdx ) = outBuffers[sigIdx][sampleIdx * resamplingFactor * signalNumOfElements[sigIdx] + elemIdx];				
                            }			
                            else if (signalTypes[sigIdx] == UnsignedInteger32Bit) {
                                *( ((uint32*)outputSignals[sigIdx]) + sampleIdx * signalNumOfElements[sigIdx] + elemIdx ) = outBuffers[sigIdx][sampleIdx * resamplingFactor * signalNumOfElements[sigIdx] + elemIdx];
                            } else if (signalTypes[sigIdx]==SignedInteger32Bit) {
                                *( ((int32*)outputSignals[sigIdx]) + sampleIdx * signalNumOfElements[sigIdx] + elemIdx ) = outBuffers[sigIdx][sampleIdx * resamplingFactor * signalNumOfElements[sigIdx] + elemIdx];	
                            }			
                            else if (signalTypes[sigIdx] == UnsignedInteger64Bit) {
                                *( ((uint64*)outputSignals[sigIdx]) + sampleIdx * signalNumOfElements[sigIdx] + elemIdx ) = outBuffers[sigIdx][sampleIdx * resamplingFactor * signalNumOfElements[sigIdx] + elemIdx];
                            } else if (signalTypes[sigIdx]==SignedInteger64Bit) {
                                *( ((int64*)outputSignals[sigIdx]) + sampleIdx * signalNumOfElements[sigIdx] + elemIdx ) = outBuffers[sigIdx][sampleIdx * resamplingFactor * signalNumOfElements[sigIdx] + elemIdx];
                
                            } else if (signalTypes[sigIdx]==Float32Bit) {
                                *( ((float32*)outputSignals[sigIdx]) + sampleIdx * signalNumOfElements[sigIdx] + elemIdx ) = outBuffers[sigIdx][sampleIdx * resamplingFactor * signalNumOfElements[sigIdx] + elemIdx];
                            } else if (signalTypes[sigIdx]==Float64Bit) {
                                *( ((float64*)outputSignals[sigIdx]) + sampleIdx * signalNumOfElements[sigIdx] + elemIdx ) = outBuffers[sigIdx][sampleIdx * resamplingFactor * signalNumOfElements[sigIdx] + elemIdx];
                            }  //No other choiches, already checked before
                        }
                    }
                }
                else //resampleMode == PICK_AVERAGED
                {
                    for(uint32 elemIdx = 0; elemIdx < signalNumOfElements[sigIdx]; elemIdx++)
                    {
                        for(uint32 outSampleIdx = 0; outSampleIdx < signalNumOfSamples[sigIdx]/resamplingFactor; outSampleIdx++)
                        {
                            float32 currAvg = 0;
                            for(uint32 inSampleIdx = 0; inSampleIdx < resamplingFactor; inSampleIdx++)
                            {
                               currAvg += inBuffers[sigIdx][(outSampleIdx * resamplingFactor + inSampleIdx) * signalNumOfElements[sigIdx] + elemIdx];
                            }
                            currAvg /= resamplingFactor;
                            if (signalTypes[sigIdx] == UnsignedInteger8Bit) {
                                *( ((uint8*)outputSignals[sigIdx]) + outSampleIdx * signalNumOfElements[sigIdx] + elemIdx ) = currAvg;
                            } else if (signalTypes[sigIdx]==SignedInteger8Bit) {
                                *( ((int8*)outputSignals[sigIdx]) + outSampleIdx * signalNumOfElements[sigIdx] + elemIdx ) = currAvg;     
                            }                   
                            else if (signalTypes[sigIdx] == UnsignedInteger16Bit) {
                                *( ((uint16*)outputSignals[sigIdx]) + outSampleIdx * signalNumOfElements[sigIdx] + elemIdx ) = currAvg;
                            } else if (signalTypes[sigIdx]==SignedInteger16Bit) {
                                *( ((int16*)outputSignals[sigIdx]) + outSampleIdx * signalNumOfElements[sigIdx] + elemIdx ) = currAvg;                            
                            }                   
                            else if (signalTypes[sigIdx] == UnsignedInteger32Bit) {
                                *( ((uint32*)outputSignals[sigIdx]) + outSampleIdx * signalNumOfElements[sigIdx] + elemIdx ) = currAvg;
                            } else if (signalTypes[sigIdx]==SignedInteger32Bit) {
                                *( ((int32*)outputSignals[sigIdx]) + outSampleIdx * signalNumOfElements[sigIdx] + elemIdx ) = currAvg;    
                            }                   
                            else if (signalTypes[sigIdx] == UnsignedInteger64Bit) {
                                *( ((uint64*)outputSignals[sigIdx]) + outSampleIdx * signalNumOfElements[sigIdx] + elemIdx ) = currAvg;
                            } else if (signalTypes[sigIdx]==SignedInteger64Bit) {
                                *( ((int64*)outputSignals[sigIdx]) + outSampleIdx * signalNumOfElements[sigIdx] + elemIdx ) = currAvg;
                
                            } else if (signalTypes[sigIdx]==Float32Bit) {
                                *( ((float32*)outputSignals[sigIdx]) + outSampleIdx * signalNumOfElements[sigIdx] + elemIdx ) = currAvg;
                            } else if (signalTypes[sigIdx]==Float64Bit) {
                                *( ((float64*)outputSignals[sigIdx]) + outSampleIdx * signalNumOfElements[sigIdx] + elemIdx ) = currAvg;
                            }  //No other choiches, already checked before
                        }
                    }
                }
	    }
	    else //resampleMode == JUST_PICK
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
		
		
                for (uint32 sampleIdx = 0; sampleIdx < signalNumOfSamples[sigIdx]/resamplingFactor; sampleIdx++) 
                {
                    for (uint32 elemIdx = 0; elemIdx < signalNumOfElements[sigIdx]; elemIdx++) 
                    {
                        memcpy(outputSignals[sigIdx] + (sampleIdx * signalNumOfElements[sigIdx] + elemIdx) * elemSize, inputSignals[sigIdx] + (sampleIdx * signalNumOfElements[sigIdx] * resamplingFactor + elemIdx) * elemSize, elemSize);
                    }
                }
	    }
	}//Endfor
	return true;
    }


CLASS_REGISTER(ResamplerGAM, "1.0")

} /* namespace MARTe */
