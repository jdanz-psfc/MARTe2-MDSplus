/**
 * @file FunctionGeneratorGAM.cpp
 * @brief Source file for class FunctionGeneratorGAM
 * @date Nov 9, 2016 TODO Verify the value and format of the date
 * @author gmanduchi TODO Verify the name and format of the author
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

 * @details This source file contains the definition of all the methods for
 * the class ConstantGAM (public, protected, and private). Be aware that some
 * methods, such as those inline could be defined on the header file, instead.
 */

#define DLL_API

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/
#include "FFTGAM.h"
#include "AdvancedErrorManagement.h"
#include <iostream>
/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

namespace MARTe {

FFTGAM::FFTGAM() : GAM()
{
    outSignalMemory[0] = outSignalMemory[1] = NULL;
}

FFTGAM::~FFTGAM() {
    fftw_destroy_plan(p);
    fftw_free(in); 
    fftw_free(out);
}

bool FFTGAM::Setup() {
    bool ok = (GetNumberOfInputSignals() == 1);
    if (!ok) {
        REPORT_ERROR(ErrorManagement::ParametersError, "There shall be one input signal");
	return ok;
    }
    ok = (GetNumberOfOutputSignals() == 2);
    if (!ok) {
        REPORT_ERROR(ErrorManagement::ParametersError, "There shall be two output signals (module and phase)");
    }
    inputType  = GetSignalType(InputSignals, 0);
    if(inputType != Float64Bit 
	&& inputType != Float32Bit 
	&& inputType != UnsignedInteger16Bit
	&& inputType != SignedInteger16Bit
	&& inputType != UnsignedInteger32Bit
	&& inputType != SignedInteger32Bit)
    {
        ok = false;
        REPORT_ERROR(ErrorManagement::ParametersError, "Only Float (32/64 bits) and integers (16/32 bits) types supported as input");
    }
    uint32 numberOfDimensions;
    GetSignalNumberOfDimensions(InputSignals, 0, numberOfDimensions);
    ok = (numberOfDimensions == 1 || (numberOfDimensions == 0));
    if (!ok) {
        REPORT_ERROR(ErrorManagement::ParametersError, "Input signal will be either 1D (single samples) or scarar (multiple samples)");
    }
    
    if(numberOfDimensions == 1)
    {
    	ok = GetSignalNumberOfElements(InputSignals, 0, numberOfElements);
    }
    else
    {
	ok = GetSignalNumberOfSamples(InputSignals, 0, numberOfElements);
    }
    if (ok) {
            inSignalMemory = (float64 *) GetInputSignalMemory(0);
    }

    for(uint32 outIdx = 0; outIdx < 2; outIdx++)
    {
	TypeDescriptor outType = GetSignalType(OutputSignals, outIdx);
        if(outType != Float64Bit)
        {
            ok = false;
            REPORT_ERROR(ErrorManagement::ParametersError, "Output signal type shall be Float64");
        }
    	GetSignalNumberOfDimensions(OutputSignals, outIdx, numberOfDimensions);
        ok = (numberOfDimensions == 1);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "Output signal shall be 1D");
    	}
	uint32 outElements;	
    	GetSignalNumberOfElements(OutputSignals, outIdx, outElements);
	ok = (outElements == numberOfElements);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "Output signal shall have %d elements", numberOfElements);
    	}
    	if (ok) {
            outSignalMemory[outIdx] = (float64 *) GetOutputSignalMemory(outIdx);
	}
    }
    if(ok)
    {
        in = (double *)fftw_malloc(sizeof(double ) * numberOfElements); 
        out = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * numberOfElements); 
	p = fftw_plan_dft_r2c_1d(numberOfElements, in, out, FFTW_MEASURE);
    }
    return ok;
}

bool FFTGAM::Execute() {


    for(uint32 i = 0; i < numberOfElements; i++)
    {
in[i] = ((float64 *)inSignalMemory)[i];
continue;

	if (inputType == Float32Bit)
	{
		in[i] = ((float32 *)inSignalMemory)[i];
	}
	else if (inputType == Float64Bit)
	{
		in[i] = ((float64 *)inSignalMemory)[i];
	}
	else if (inputType == UnsignedInteger16Bit)
	{
		in[i] = ((uint16 *)inSignalMemory)[i];
	}
	else if (inputType == SignedInteger16Bit)
	{
		in[i] = ((int16 *)inSignalMemory)[i];
	}
	else if (inputType == UnsignedInteger32Bit)
	{
		in[i] = ((uint32 *)inSignalMemory)[i];
	}
	else if (inputType == SignedInteger32Bit)
	{
		in[i] = ((int32 *)inSignalMemory)[i];
	}
	else printf("OHIBO'!!!!!!!!\n");
    }
    fftw_execute(p);
    for(uint32 i = 0; i < numberOfElements; i++)
    {
	outSignalMemory[0][i] = sqrt(out[i][0]*out[i][0]+out[i][1]*out[i][1]);
	outSignalMemory[1][i] = atan2(out[i][1], out[i][0]);
    }
    return true;
}

bool FFTGAM::Initialise(StructuredDataI & data) {
    bool ok = GAM::Initialise(data);
    return ok;
}


CLASS_REGISTER(FFTGAM, "1.0")

}

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

