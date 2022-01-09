/**
 * @file PickSampleGAM.cpp
 * @brief Source file for class ResamplerGAM
 * @date Jul 30, 2019
 * @author nn
 *

 * @details This source file contains the definition of all the methods for the
 * class PickSampleGAM (public, protected, and private). Be aware that some
 * methods, such as those inline could be defined on the header file, instead.
 */

#define DLL_API

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/

#include "PickSampleGAM.h"
#include "AdvancedErrorManagement.h"
#include <iostream>

/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

namespace MARTe {

PickSampleGAM::PickSampleGAM() : GAM(){
    numSignals = 0;
    signalSamples = NULL_PTR(uint32 *);
    signalByteSize = NULL_PTR(uint32 *);
    inputSignals = NULL_PTR(uint8 **);
    outputSignals = NULL_PTR(uint8 **);

}

PickSampleGAM::~PickSampleGAM() {

    if(signalSamples !=  NULL_PTR(uint32 *))
    {
        GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(signalSamples));
    }    
    if(signalByteSize !=  NULL_PTR(uint32 *))
    {
        GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(signalByteSize));
    }    
    if(inputSignals !=  NULL_PTR(uint8 **))
    {
	GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(inputSignals));
    }
    if(outputSignals !=  NULL_PTR(uint8 **))
    {
	GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(outputSignals));
    }
}

bool PickSampleGAM::Initialise(StructuredDataI & data) {
	
    REPORT_ERROR(ErrorManagement::Debug, "PickSampleGAM: INITIALIZE");
    
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
    return ok;
}

bool PickSampleGAM::Setup() {
	
    bool ok;
    
    REPORT_ERROR(ErrorManagement::Debug, "PickSampleGAM SETUP");
    
    /**
      * Firstly, types and dimensions of signals are retrieved and checked.
      */
    signalSamples = reinterpret_cast<uint32 *>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(numSignals * sizeof(uint32)));
    signalByteSize = reinterpret_cast<uint32 *>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(numSignals * sizeof(uint32)));
    TypeDescriptor signalType;
    uint32 signalElement;

    // Inputs
    for (uint32 sigIdx = 0; sigIdx < numSignals; sigIdx++) {
            
        signalType = GetSignalType(InputSignals, sigIdx);
            
        ok = GetSignalNumberOfSamples(InputSignals, sigIdx, signalSamples[sigIdx]);
        if (!ok) {
                    
            REPORT_ERROR(ErrorManagement::Exception,
                                              "Error in GetSignalNumberOfSamples: input signal %i does not exist.", sigIdx);
            return ok;
                    
        }
 
        ok = GetSignalByteSize(InputSignals,sigIdx, signalByteSize[sigIdx]);
        if (!ok) {
                    
            REPORT_ERROR(ErrorManagement::Exception,
                                              "Error in GetSignalNumberOfSamples: input signal %i does not exist.", sigIdx);
            return ok;
                    
        }
        ok = GetSignalNumberOfElements(InputSignals,sigIdx, signalElement);
        if (!ok) {
                    
            REPORT_ERROR(ErrorManagement::Exception,
                                              "Error in GetSignalNumberOfElements: input signal %i does not exist.", sigIdx);
            return ok;
                    
        }

        TypeDescriptor outType = GetSignalType(OutputSignals, sigIdx);
        ok = (outType == signalType);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::Exception,
                                              "Output type for signal %i shall be the same of the corresponding input.", sigIdx);
            return ok;
        }

//If output is declared as scalar (numberOfDimensions == 0) then Samples will be the input array size / resFactor. Otherwise, the array dimension shall be the input array size / resFactor.
        uint32 numOutElements;
        ok = GetSignalNumberOfElements(OutputSignals, sigIdx, numOutElements);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::Exception,
                                              "Error in GetSignalNumberOfElements: output signal %i does not exist.", sigIdx);
            return ok;	
        }
        ok = (numOutElements == signalElement);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::Exception,
                                              "Input/output number of elements different for  signal %i.", sigIdx);
            return ok;  
        }
        uint32 numOutSamples;
        ok = GetSignalNumberOfSamples(OutputSignals, sigIdx, numOutSamples);
        if (!ok) {
                REPORT_ERROR(ErrorManagement::Exception,
                                          "Error in GetSignalNumberOfSamples: output signal %i does not exist.", sigIdx);
                return ok;	
        }
        ok = (numOutSamples == 1);
        if (!ok) {
                
                REPORT_ERROR(ErrorManagement::Exception,
                                          "Output signal %i shall have one single sample.", sigIdx);
        return ok;
        }
    }

    //Allocate inptus, outputs and compute offsets
    inputSignals = reinterpret_cast<uint8 **>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(numSignals * sizeof(uint8 **)));
    outputSignals = reinterpret_cast<uint8 **>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(numSignals * sizeof(uint8 **)));


    for (uint32 sigIdx = 0; sigIdx < numSignals; sigIdx++) 
    {
        inputSignals[sigIdx] = reinterpret_cast<uint8 *>(GetInputSignalMemory(sigIdx));
        outputSignals[sigIdx] = reinterpret_cast<uint8 *>(GetOutputSignalMemory(sigIdx));
    }
    
    return ok;
}

bool PickSampleGAM::Execute() {
	
    REPORT_ERROR(ErrorManagement::Debug, "EXECUTE");
    
    for (uint32 sigIdx = 0; sigIdx < numSignals; sigIdx++) {
        memcpy(outputSignals[sigIdx], inputSignals[sigIdx], signalByteSize[sigIdx]); //Copy first sample
    }
    return true;
}


CLASS_REGISTER(PickSampleGAM, "1.0")

} /* namespace MARTe */
