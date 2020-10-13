/**
 * @file ConstantGAM.h
 * @brief Header file for class ConstantGAM
 * @date Nov 9, 2016 TODO Verify the value and format of the date
 * @author aneto TODO Verify the name and format of the author
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

 * @details This header file contains the declaration of the class ConstantGAM
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
 */

#ifndef ResamplerGAM_H_
#define ResamplerGAM_H_

/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/
#include "GAM.h"
#include "StructuredDataI.h"
#include "MessageI.h"
#include <math.h>
extern "C"  {
#include <filter.h>
}
/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/
/**************************************************************
Resampler GAM will carry out resampling for an arbitrary number of inputs,
where every input is an array of arbitrary length. 
Supported types are float32, float64, int8, int16, int32, int64.
The device expects an equal number of outputs and every output will be of
dimension of the input divided by the resampling factor. The output types shall be the same of the corresponding inputs. 
An optional low pass filter can be enabled, filtering at half the resampled frequency.

****************************************************************/
namespace MARTe {
class ResamplerGAM : public GAM {
public:
    CLASS_REGISTER_DECLARATION()

    ResamplerGAM();

    virtual ~ResamplerGAM();

    virtual bool Setup();

    virtual bool Execute();

    virtual bool Initialise(StructuredDataI & data);

    uint32 numSignals;
    uint32 resamplingFactor;
    float64 samplingPeriod;
    TypeDescriptor* signalTypes;
    uint32* signalNumOfElements;

    uint8 **inputSignals;
    uint8 **outputSignals;

    bool filterEnabled;
    float64 **inBuffers;
    float64 **outBuffers;
    Filter **lowPassFilters; 
    RunTimeFilter *runTimeFilters; 

    };
}

/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif 
	
