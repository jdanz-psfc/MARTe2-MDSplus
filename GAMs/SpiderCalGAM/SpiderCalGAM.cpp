#include "SpiderCalGAM.h"
#include "AdvancedErrorManagement.h"
#include <iostream>
#include <math.h>
#include <stdio.h>
namespace MARTe {


static const char * const inNames[] = { "VPGbias",  
	  "Vbias",
	  "IPG1",
	  "Pv",
	  "V_EG",
	  "V_acc",
	  "I_EG",
	  "I_acc",
	  "RF1_Pf",
	  "RF2_Pf",
	  "RF3_Pf",
	  "RF4_Pf",
	  "PLight1",
	  "PLight2",
	  "PLight3",
	  "PLight4",
	  "PLight5",
	  "PLight6",
	  "PLight7",
	  "PLight8",
	  "FSB_TT1",
	  "FSB_TT2",
	  "FSB_TT3",
	  "FSB_TT4",
	  "FSB_TT5",
	  "FSB_TT6",
	  "FSB_TT7",
	  "FSB_TT8",
	  "FSW_TT1",
	  "FSW_TT2",
	  "FSW_TT3",
	  "FSW_TT4",
	  "FSW_TT5",
	  "FSW_TT6",
	  "FSW_TT7",
	  "FSW_TT8",
	  "RF1",
	  "RF2",
	  "RF3",
	  "RF4",
	  "TC_BP21",
	  "TC_BP22",
	  "TC_BP31",
	  "TC_BP32",
	  "TC_BP41",
	  "TC_BP42",
	  "PG_TT12",
	  "PG_TT13",
	  "PG_TT22",
	  "PG_TT23",
	  "PG_TT32",
	  "PG_TT33",
	  "PG_TT42",
	  "PG_TT43",
	  "Tout_FSLW1_trend",
	  "Tout_FSLW2_trend",
	  "Tout_RF_trend",
	  "Tout_FSBP_trend",
	  "Tout_SCLW_trend",
	  "Tout_BP_trend",
	  "Tout_DP_trend",
	  "Tout_PG1_trend",
	  "Tout_PG2_trend",
	  "Tout_PG3_trend",
	  "Tout_PG4_trend",
	  "Tout_EG1_trend",
	  "Tout_EG2_trend",
	  "Tout_EG3_trend",
	  "Tout_EG4_trend",
	  "Tout_GG1_trend",
	  "Tout_GG2_trend",
	  "Tout_GG3_trend",
	  "Tout_GG4_trend",
	  "Tin_PC02_trend",
	  "Tin_PC03_trend",
	  "flow_PG",
	  "flow_EG",
	  "flow_GG",
	  "flow_FSLW",
	  "flow_RF",
	  "flow_FSBP",
	  "flow_SCLW",
	  "flow_BP",
	  "flow_DP"
};
static const char * const outNames[] = {
	  "VPGbias",  
	  "Vbias",
	  "IPG1",
	  "Pv",
	  "V_EG",
	  "V_acc",
	  "I_EG",
	  "I_acc",
	  "Power_on_EG",
	  "Peak_temp_blue",
	  "Peak_temp_red",
	  "RF1_Pf",
	  "RF2_Pf",
	  "RF3_Pf",
	  "RF4_Pf",
	  "PLight1",
	  "PLight2",
	  "PLight3",
	  "PLight4",
	  "PLight5",
	  "PLight6",
	  "PLight7",
	  "PLight8",
	  "RF1_pfave",
	  "RF2_pfave",
	  "RF3_pfave",
	  "RF4_pfave",
	  "PLight1ave",
	  "PLight2ave",
	  "PLight3ave",
	  "PLight4ave",
	  "PLight5ave",
	  "PLight6ave",
	  "PLight7ave",
	  "PLight8ave",
	  "FSB_TT1",
	  "FSB_TT2",
	  "FSB_TT3",
	  "FSB_TT4",
	  "FSB_TT5",
	  "FSB_TT6",
	  "FSB_TT7",
	  "FSB_TT8",
	  "FSB_TT1_mean",
	  "FSB_TT2_mean",
	  "FSB_TT3_mean",
	  "FSB_TT4_mean",
	  "FSB_TT5_mean",
	  "FSB_TT6_mean",
	  "FSB_TT7_mean",
	  "FSB_TT8_mean",
	  "FSW_TT1",
	  "FSW_TT2",
	  "FSW_TT3",
	  "FSW_TT4",
	  "FSW_TT5",
	  "FSW_TT6",
	  "FSW_TT7",
	  "FSW_TT8",
	  "FSW_TT1_mean",
	  "FSW_TT2_mean",
	  "FSW_TT3_mean",
	  "FSW_TT4_mean",
	  "FSW_TT5_mean",
	  "FSW_TT6_mean",
	  "FSW_TT7_mean",
	  "FSW_TT8_mean",
	  "RF1",
	  "RF2",
	  "RF3",
	  "RF4",
	  "RF1ave",
	  "RF2ave",
	  "RF3ave",
	  "RF4ave",
	  "TC_BP21",
	  "TC_BP22",
	  "TC_BP31",
	  "TC_BP32",
	  "TC_BP41",
	  "TC_BP42",
	  "BP21heat",
	  "BP22heat",
	  "BP31heat",
	  "BP32heat",
	  "BP41heat",
	  "BP42heat",
	  "PG_TT12",
	  "PG_TT13",
	  "PG_TT22",
	  "PG_TT23",
	  "PG_TT32",
	  "PG_TT33",
	  "PG_TT42",
	  "PG_TT43",
	  "PG_TT12heat",
	  "PG_TT13heat",
	  "PG_TT22heat",
	  "PG_TT23heat",
	  "PG_TT32heat",
	  "PG_TT33heat",
	  "PG_TT42heat",
	  "PG_TT43heat",
	  "PG12heat",
	  "PG13heat",
	  "PG22heat",
	  "PG23heat",
	  "PG32heat",
	  "PG33heat",
	  "PG42heat",
	  "PG43heat",
	  "Tout_FSLW1_trend",
	  "Tout_FSLW2_trend",
	  "Tout_RF_trend",
	  "Tout_FSBP_trend",
	  "Tout_SCLW_trend",
	  "Tout_BP_trend",
	  "Tout_DP_trend",
	  "Tout_PG1_trend",
	  "Tout_PG2_trend",
	  "Tout_PG3_trend",
	  "Tout_PG4_trend",
	  "Tout_EG1_trend",
	  "Tout_EG2_trend",
	  "Tout_EG3_trend",
	  "Tout_EG4_trend",
	  "Tout_GG1_trend",
	  "Tout_GG2_trend",
	  "Tout_GG3_trend",
	  "Tout_GG4_trend",
	  "Tin_PC02_trend",
	  "Tin_PC03_trend",
	  "DT_FSLW1",
	  "DT_FSLW2",
	  "DT_RF",
	  "DT_FSBP",
	  "DT_SCLW",
	  "DT_BP",
	  "DT_DP",
	  "DT_PG1",
	  "DT_PG2",
	  "DT_PG3",
	  "DT_PG4",
	  "DT_EG1",
	  "DT_EG2",
	  "DT_EG3",
	  "DT_EG4",
	  "DT_GG1",
	  "DT_GG2",
	  "DT_GG3",
	  "DT_GG4",
	  "DT_FSLW1_heat",
	  "DT_FSLW2_heat",
	  "DT_RF_heat",
	  "DT_FSBP_heat",
	  "DT_SCLW_heat",
	  "DT_BP_heat",
	  "DT_DP_heat",
	  "DT_PG1_heat",
	  "DT_PG2_heat",
	  "DT_PG3_heat",
	  "DT_PG4_heat",
	  "DT_EG1_heat",
	  "DT_EG2_heat",
	  "DT_EG3_heat",
	  "DT_EG4_heat",
	  "DT_GG1_heat",
	  "DT_GG2_heat",
	  "DT_GG3_heat",
	  "DT_GG4_heat",
	  "DT_Tot_heat",
};

SpiderCalGAM::SpiderCalGAM() : GAM(){}
SpiderCalGAM::~SpiderCalGAM(){}
double SpiderCalGAM::calibTCN(float inX)
{

	double K  = 47.513 * 1.e-3  / 5.;

	double data = inX * K * 1.e6;
	double C1 =  3.86896e-2;
	double C2 = -1.08267e-6;
	double C3 =  4.70205e-11;
	double C4 = -2.12169e-18;
	double C5 = -1.17272e-19;
	double C6 =  5.39280e-24;
	double C7 = -7.98156e-29;

	double cal = C1 * data + C2 *pow(data, 2) + C3 *pow(data, 3) + C4 * pow(data, 4) +
	  C5 * pow(data, 5) + C6 * pow(data, 6) + C7 * pow(data, 7); 	

	return cal;

}

float SpiderCalGAM::getSmoothed(float val, float *history)
{
    history[smoothIdx] = val;
    double smoothed = 0;
    if(currSample < SMOOTH_HISTORY)
      return val;
    for(int i = 0; i < SMOOTH_HISTORY; i++)
      smoothed += history[i];
    return smoothed/SMOOTH_HISTORY;
}

float SpiderCalGAM::getCalAveraged(float val, float *history)
{
    history[calIdx] = val;
    double averaged = 0;
    if(currSample < CAL_HISTORY)
      return val;
    for(int i = 0; i < CAL_HISTORY; i++)
      averaged += history[i];
    return averaged/CAL_HISTORY;
}
  
float SpiderCalGAM::getCalPGAveraged(float *history, int delay)
{
    double averaged = 0;
    int idx = trendIdx - delay - CAL_HISTORY;
    if(idx < 0) idx += MAX_TRENDHISTORY;
    for(int i = 0; i < CAL_HISTORY; i++)
    {
	averaged += history[idx];
	idx++;
	if(idx >= MAX_TRENDHISTORY)
	  idx = 0;
    }
    return averaged/CAL_HISTORY;
}






//Low pass filter 20 Hz, sampling freq: 10 kHz
float SpiderCalGAM::lowPassFilter(float x)
{
    lowPassPrevY = 0.00624*(x + lowPassPrevX-1) + 0.98751*lowPassPrevY-1;
    lowPassPrevX = x;
    return lowPassPrevY;
}
bool SpiderCalGAM::GetSignalNames(const SignalDirection direction,
									   StreamString* names) {
	
	bool   ok;
	uint32 numOfSignals;
	
	if (direction==InputSignals) {
		
		numOfSignals = GetNumberOfInputSignals();
		
	} else if (direction==OutputSignals) {
		
		numOfSignals = GetNumberOfOutputSignals();
		
	}
	
	for (uint32 i = 0; i < numOfSignals; i++) {
		ok = GetSignalName(direction, i, names[i]);
		if (!ok) return ok;
		
	}
	
	return ok;
	
}

bool SpiderCalGAM::Initialise(StructuredDataI & data) {
	
	//REPORT_ERROR(ErrorManagement::Debug, "MathExpressionGAM: INITIALIZE");
 	bool ok = GAM::Initialise(data);

	if (!ok) {
		REPORT_ERROR(ErrorManagement::InitialisationError,
					 "Initialization failed.");
		return ok;
	}
	
	period = 0.1; //10 Hz default
	ok = data.Read("Period", period);
	if (!ok) {
		REPORT_ERROR(ErrorManagement::Debug,
					 "Sleep time not set. Default value 0.1.");
		period = 0.1;
	}

	
	ok = data.MoveRelative("InputSignals");
	if(!ok){
		REPORT_ERROR(ErrorManagement::ParametersError,
					 "Cannot find InputSignals node in configuration file.");
		return ok;
	}
	
	numInputSignals = data.GetNumberOfChildren();
	ok = (numInputSignals == sizeof(inSignals)/sizeof(float * ));
	if(!ok){
		REPORT_ERROR(ErrorManagement::ParametersError,
					 "Invalid number of input signals. Found %d expected %d",
					 numInputSignals, (uint32)(sizeof(inSignals)/sizeof(float * ))
			    );
		return ok;
	}
	// Back to the GAM node
	data.MoveToAncestor(1);
	
	ok = data.MoveRelative("OutputSignals");
	if(!ok){
		REPORT_ERROR(ErrorManagement::ParametersError,
					 "Cannot find OutputSignals node in configuration file.");
		return ok;
	}
	numOutputSignals = data.GetNumberOfChildren();
	ok = (numOutputSignals == sizeof(outSignals)/sizeof(float *));
	if(!ok){
		REPORT_ERROR(ErrorManagement::ParametersError,
					 "Invalid number of output signals. Found %d expected %d",
					 numOutputSignals, (uint32)(sizeof(outSignals)/sizeof(float * )));
		return ok;
	}
	data.MoveToAncestor(1);
	lowPassPrevX = lowPassPrevY = 0;
	return ok;
	
}

bool SpiderCalGAM::Setup() {
	
	bool ok;

	StreamString currInNames[numInputSignals];
	StreamString currOutNames[numOutputSignals];
	
	///1. The list of all variables concurring to form this output is retrieved.
	GetSignalNames(InputSignals, currInNames);
	GetSignalNames(OutputSignals, currOutNames);
	for(uint32 sigIdx = 0; sigIdx < numInputSignals; sigIdx++)
	{
	    ok = (currInNames[sigIdx] == inNames[sigIdx]);
	    if(!ok){
		REPORT_ERROR(ErrorManagement::ParametersError,
					 "Wrong input signal name. Expected %s, found %s",
					 inNames[sigIdx], currInNames[sigIdx].Buffer() );
		return ok;
	    }
	}
	for(uint32 sigIdx = 0; sigIdx < numOutputSignals; sigIdx++)
	{
	    ok = (currOutNames[sigIdx] == outNames[sigIdx]);
	    if(!ok){
		REPORT_ERROR(ErrorManagement::ParametersError,
					 "Wrong output signal name. Expected %s, found %s",
					 outNames[sigIdx], currOutNames[sigIdx].Buffer() );
		return ok;
	    }
	}
	// Inputs
	for (uint32 sigIdx = 0; sigIdx < numInputSignals; sigIdx++) {
		
		TypeDescriptor type = GetSignalType(InputSignals, sigIdx);
		ok = (type == Float32Bit);
		if (!ok) {
			
			REPORT_ERROR(ErrorManagement::Exception,
						 "Type of input signal %i must be Float32.",
						 sigIdx);
			return ok;
			
		}
		uint32 numElements = 0;
		GetSignalNumberOfElements(InputSignals, sigIdx, numElements);
		ok = (numElements == 1);
		if (!ok) {
			
			REPORT_ERROR(ErrorManagement::Exception,
						 "Input Signal %i must be scalar.",
						 sigIdx);
			return ok;
			
		}
		
	}
	
	// Outputs
	for (uint32 sigIdx = 0; sigIdx < numOutputSignals; sigIdx++) {
		
		TypeDescriptor type = GetSignalType(OutputSignals, sigIdx);
		ok = (type == Float32Bit);
		if (!ok) {
			
			REPORT_ERROR(ErrorManagement::Exception,
						 "Type of output signal %i must be Float32.",
						 sigIdx);
			return ok;
			
		}
		uint32 numElements  = 0;
		GetSignalNumberOfElements(OutputSignals, sigIdx, numElements);
		ok = (numElements == 1);
		if (!ok) {
			
			REPORT_ERROR(ErrorManagement::Exception,
						 "Output signal %i must be scalar.",
						 sigIdx);
			return ok;
			
		}
		
	}
	
	// Mappings
	float **currInput = (float **)&inSignals;
	for (uint32 sigIdx = 0; sigIdx < numOutputSignals; sigIdx++) {
	    currInput[sigIdx] = (float *)GetInputSignalMemory(sigIdx);
	}   
	float **currOutput = (float **)&outSignals;
	for (uint32 sigIdx = 0; sigIdx < numOutputSignals; sigIdx++) {
	    currOutput[sigIdx] = (float *)GetOutputSignalMemory(sigIdx);
	} 
	firstRF_pfTurn = true;
        firstRFTurn = true;
	firstLightTurn = true;
	firstTCTurn = true;
	firstPGTurn = true;
	firstFSBTurn = true;
	firstRF1_Pf = 0;
	firstRF2_Pf = 0;
	firstRF3_Pf = 0;
	firstRF4_Pf = 0;
	firstPLight1 = 0;
	firstPLight2 = 0;
	firstPLight3 = 0;
	firstPLight4 = 0;
	firstPLight5 = 0;
	firstPLight6 = 0;
	firstPLight7 = 0;
	firstPLight8 = 0;

	FSB_TT1_50 = 0;
	FSB_TT2_50 = 0;
	FSB_TT3_50 = 0;
	FSB_TT4_50 = 0;
	FSB_TT5_50 = 0;
	FSB_TT6_50 = 0;
	FSB_TT7_50 = 0;
	FSB_TT8_50 = 0;
	FSW_TT1_50 = 0;
	FSW_TT2_50 = 0;
	FSW_TT3_50 = 0;
	FSW_TT4_50 = 0;
	FSW_TT5_50 = 0;
	FSW_TT6_50 = 0;
	FSW_TT7_50 = 0;
	FSW_TT8_50 = 0;

	firstRF1 = 0;
	firstRF2 = 0;
	firstRF3 = 0;
	firstRF4 = 0;
	
	firstTC_BP21 = 0;	
	firstTC_BP22 = 0;	
	firstTC_BP31 = 0;	
	firstTC_BP32 = 0;	
	firstTC_BP41 = 0;	
	firstTC_BP42 = 0;	
	
	firstPG_TT12 = 0;	
	firstPG_TT13 = 0;	
	firstPG_TT22 = 0;	
	firstPG_TT23 = 0;	
	firstPG_TT32 = 0;	
	firstPG_TT33 = 0;	
	firstPG_TT42 = 0;	
	firstPG_TT43 = 0;	

	currSample = 1; 
	RF1sum = 0;
	RF2sum = 0;
	RF3sum = 0;
	RF4sum = 0;
	RF1_pfsum = 0;
	RF2_pfsum = 0;
	RF3_pfsum = 0;
	RF4_pfsum = 0;
	PLight1sum = 0;
	PLight2sum = 0;
	PLight3sum = 0;
	PLight4sum = 0;
	PLight5sum = 0;
	PLight6sum = 0;
	PLight7sum = 0;
	PLight8sum = 0;
	FSB_TT1sum = 0;
	FSB_TT2sum = 0;
	FSB_TT3sum = 0;
	FSB_TT4sum = 0;
	FSB_TT5sum = 0;
	FSB_TT6sum = 0;
	FSB_TT7sum = 0;
	FSB_TT8sum = 0;
	FSW_TT1sum = 0;
	FSW_TT2sum = 0;
	FSW_TT3sum = 0;
	FSW_TT4sum = 0;
	FSW_TT5sum = 0;
	FSW_TT6sum = 0;
	FSW_TT7sum = 0;
	FSW_TT8sum = 0;
	
	TC_BP21sum = 0;
	TC_BP22sum = 0;
	TC_BP31sum = 0;
	TC_BP32sum = 0;
	TC_BP41sum = 0;
	TC_BP42sum = 0;
	
	PG_TT12sum = 0;
	PG_TT13sum = 0;
	PG_TT22sum = 0;
	PG_TT23sum = 0;
	PG_TT32sum = 0;
	PG_TT33sum = 0;
	PG_TT42sum = 0;
	PG_TT43sum = 0;
	flow_PGsum = 0;
	flow_EGsum = 0;
	flow_GGsum = 0;
	flow_FSLWsum = 0;
	flow_RFsum = 0;
	flow_SCLWsum = 0;
	flow_BPsum = 0;
	flow_DPsum = 0;

	DeltaT_FSLW1sum = 0;
	DeltaT_FSLW2sum = 0;
	DeltaT_RFsum = 0;
	DeltaT_FSBPsum = 0;
	DeltaT_SCLWsum = 0;
	DeltaT_BPsum = 0;
	DeltaT_DPsum = 0;
	DeltaT_PG1sum = 0;
	DeltaT_PG2sum = 0;
	DeltaT_PG3sum = 0;
	DeltaT_PG4sum = 0;
	DeltaT_EG1sum = 0;
	DeltaT_EG2sum = 0;
	DeltaT_EG3sum = 0;
	DeltaT_EG4sum = 0;
	DeltaT_GG1sum = 0;
	DeltaT_GG2sum = 0;
	DeltaT_GG3sum = 0;
	DeltaT_GG4sum = 0;


	
	for (uint32 i = 0; i < MAX_TRENDHISTORY; i++)
	{
	    Tin_PC02_trend[i] = 0;
	    Tin_PC03_trend[i] = 0;
	}
	
	for (uint32 i = 0; i < SMOOTH_HISTORY; i++)
	{
	    TC_BP21hist[i] = 0;
	    TC_BP22hist[i] = 0;
	    TC_BP31hist[i] = 0;
	    TC_BP32hist[i] = 0;
	    TC_BP41hist[i] = 0;
	    TC_BP42hist[i] = 0;
	
	    PG_TT12hist[i] = 0;
	    PG_TT13hist[i] = 0;
	    PG_TT22hist[i] = 0;
	    PG_TT23hist[i] = 0;
	    PG_TT32hist[i] = 0;
	    PG_TT33hist[i] = 0;
	    PG_TT42hist[i] = 0;
	    PG_TT43hist[i] = 0;
	}
	for (uint32 i = 0; i < CAL_HISTORY; i++)
	{
	    Tout_FSLW1_his[i] = 0;
	    Tout_FSLW2_his[i] = 0;
	    Tout_RF_his[i] = 0;
	    Tout_FSBP_his[i] = 0;
	    Tout_SCLW_his[i] = 0;
	    Tout_BP_his[i] = 0;
	    Tout_DP_his[i] = 0;
	    Tout_PG1_his[i] = 0;
	    Tout_PG2_his[i] = 0;
	    Tout_PG3_his[i] = 0;
	    Tout_PG4_his[i] = 0;
	    Tout_EG1_his[i] = 0;
	    Tout_EG2_his[i] = 0;
	    Tout_EG3_his[i] = 0;
	    Tout_EG4_his[i] = 0;
	    Tout_GG1_his[i] = 0;
	    Tout_GG2_his[i] = 0;
	    Tout_GG3_his[i] = 0;
	    Tout_GG4_his[i] = 0;
	}
	trendIdx = 0;
	smoothIdx = 0;
	calIdx = 0;
	totSamples = 0;

	
	return ok;  
}

bool SpiderCalGAM::Execute() {

    *outSignals.VPGbias = *inSignals.VPGbias * 50./5.;
    *outSignals.Vbias = *inSignals.Vbias * 50. / 5.;
    *outSignals.IPG1 = *inSignals.IPG1 * 1/100. * 6000/5.;
    *outSignals.Pv = *inSignals.Pv * 1E5;
    *outSignals.V_EG = *inSignals.V_EG * .5 * 15000/2.5/1000.;
    *outSignals.V_acc = *inSignals.V_acc * 120000/10./1000.;
    *outSignals.I_EG = *inSignals.I_EG * .5*200/2.5*1000;
    *outSignals.I_acc = *inSignals.I_acc * 1000. * 0.184; 
//    *outSignals.I_acc = lowPassFilter(*inSignals.I_acc * 1000. * 0.184); 
    *outSignals.Power_on_EG = - *outSignals.I_EG * *outSignals.V_EG/1000.;
    *outSignals.Peak_temp_blue = *outSignals.Power_on_EG*1.04 + 30.3;
    *outSignals.Peak_temp_red = *outSignals.Power_on_EG*2.05 + 31;
    *outSignals.RF1_Pf = (*inSignals.RF1_Pf - firstRF1_Pf)/1000.* 300000/5. ;
    *outSignals.RF2_Pf = (*inSignals.RF2_Pf - firstRF2_Pf)/1000. * 300000/5. ;
    *outSignals.RF3_Pf = (*inSignals.RF3_Pf - firstRF2_Pf)/1000. * 300000/5. ;
    *outSignals.RF4_Pf = (*inSignals.RF4_Pf - firstRF2_Pf)/1000. * 300000/5. ;
    if(firstRF_pfTurn && *inSignals.RF1_Pf > 0)
    {
      firstRF_pfTurn = false;
      firstRF1_Pf = *inSignals.RF1_Pf;
      firstRF2_Pf = *inSignals.RF2_Pf;
      firstRF3_Pf = *inSignals.RF3_Pf;
      firstRF4_Pf = *inSignals.RF4_Pf;
    }
    *outSignals.PLight1 = (*inSignals.PLight1 - firstPLight1) * 13.7;
    *outSignals.PLight2 = (*inSignals.PLight2 - firstPLight2) * 6.7;
    *outSignals.PLight3 = (*inSignals.PLight3 - firstPLight3) * 10.8;
    *outSignals.PLight4 = (*inSignals.PLight4 - firstPLight4) * 7.3;
    *outSignals.PLight5 = (*inSignals.PLight5 - firstPLight5) * 14.4;
    *outSignals.PLight6 = (*inSignals.PLight6 - firstPLight6) * 12;
    *outSignals.PLight7 = (*inSignals.PLight7 - firstPLight7) * 8.3;
    *outSignals.PLight8 = (*inSignals.PLight8 - firstPLight8) * 9.5;
    
    if(firstLightTurn && *inSignals.PLight1 > 0)
    {
	firstLightTurn = false;
	firstPLight1 = *inSignals.PLight1;
	firstPLight2 = *inSignals.PLight2;
	firstPLight3 = *inSignals.PLight3;
	firstPLight4 = *inSignals.PLight4;
	firstPLight5 = *inSignals.PLight5;
	firstPLight6 = *inSignals.PLight6;
	firstPLight7 = *inSignals.PLight7;
	firstPLight8 = *inSignals.PLight8;
    }
    
    RF1_pfsum += *outSignals.RF1_Pf;
    RF2_pfsum += *outSignals.RF2_Pf;
    RF3_pfsum += *outSignals.RF3_Pf;
    RF4_pfsum += *outSignals.RF4_Pf;
    *outSignals.RF1_pfave = RF1_pfsum/currSample;// /1000.* 300000/5.;
    *outSignals.RF2_pfave = RF2_pfsum/currSample;// /1000.* 300000/5.;
    *outSignals.RF3_pfave = RF3_pfsum/currSample;// /1000.* 300000/5.;
    *outSignals.RF4_pfave = RF4_pfsum/currSample;// /1000.* 300000/5.;

//Display x 10**20
    PLight1sum += (*inSignals.PLight1 - firstPLight1);
    PLight2sum += (*inSignals.PLight2 - firstPLight2);
    PLight3sum += (*inSignals.PLight3 - firstPLight3);
    PLight4sum += (*inSignals.PLight4 - firstPLight4);
    PLight5sum += (*inSignals.PLight5 - firstPLight5);
    PLight6sum += (*inSignals.PLight6 - firstPLight6);
    PLight7sum += (*inSignals.PLight7 - firstPLight7);
    PLight8sum += (*inSignals.PLight8 - firstPLight8);
    
    *outSignals.PLight1ave = PLight1sum/currSample * 137;  //display x10**19
    *outSignals.PLight2ave = PLight2sum/currSample * 67;
    *outSignals.PLight3ave = PLight3sum/currSample * 108;
    *outSignals.PLight4ave = PLight4sum/currSample * 73;
    *outSignals.PLight5ave = PLight5sum/currSample * 144;
    *outSignals.PLight6ave = PLight6sum/currSample * 120;
    *outSignals.PLight7ave = PLight7sum/currSample * 83;
    *outSignals.PLight8ave = PLight8sum/currSample * 95;
    
    *outSignals.FSB_TT1 = calibTCN(*inSignals.FSB_TT1);
    *outSignals.FSB_TT2 = calibTCN(*inSignals.FSB_TT2);
    *outSignals.FSB_TT3 = calibTCN(*inSignals.FSB_TT3);
    *outSignals.FSB_TT4 = calibTCN(*inSignals.FSB_TT4);
    *outSignals.FSB_TT5 = calibTCN(*inSignals.FSB_TT5);
    *outSignals.FSB_TT6 = calibTCN(*inSignals.FSB_TT6);
    *outSignals.FSB_TT7 = calibTCN(*inSignals.FSB_TT7);
    *outSignals.FSB_TT8 = calibTCN(*inSignals.FSB_TT8);
  
    FSB_TT1sum += (*outSignals.FSB_TT1 - FSB_TT1_50);
    FSB_TT2sum += (*outSignals.FSB_TT2 - FSB_TT2_50);
    FSB_TT3sum += (*outSignals.FSB_TT3 - FSB_TT3_50);
    FSB_TT4sum += (*outSignals.FSB_TT4 - FSB_TT4_50);
    FSB_TT5sum += (*outSignals.FSB_TT5 - FSB_TT5_50);
    FSB_TT6sum += (*outSignals.FSB_TT6 - FSB_TT6_50);
    FSB_TT7sum += (*outSignals.FSB_TT7 - FSB_TT7_50);
    FSB_TT8sum += (*outSignals.FSB_TT8 - FSB_TT8_50);
   
    *outSignals.FSB_TT1_mean = FSB_TT1sum/currSample;
    *outSignals.FSB_TT2_mean = FSB_TT2sum/currSample;
    *outSignals.FSB_TT3_mean = FSB_TT3sum/currSample;
    *outSignals.FSB_TT4_mean = FSB_TT4sum/currSample;
    *outSignals.FSB_TT5_mean = FSB_TT5sum/currSample;
    *outSignals.FSB_TT6_mean = FSB_TT6sum/currSample;
    *outSignals.FSB_TT7_mean = FSB_TT7sum/currSample;
    *outSignals.FSB_TT8_mean = FSB_TT8sum/currSample;
    
    *outSignals.FSW_TT1 = calibTCN(*inSignals.FSW_TT1);
    *outSignals.FSW_TT2 = calibTCN(*inSignals.FSW_TT2);
    *outSignals.FSW_TT3 = calibTCN(*inSignals.FSW_TT3);
    *outSignals.FSW_TT4 = calibTCN(*inSignals.FSW_TT4);
    *outSignals.FSW_TT5 = calibTCN(*inSignals.FSW_TT5);
    *outSignals.FSW_TT6 = calibTCN(*inSignals.FSW_TT6);
    *outSignals.FSW_TT7 = calibTCN(*inSignals.FSW_TT7);
    *outSignals.FSW_TT8 = calibTCN(*inSignals.FSW_TT8);
    
    FSW_TT1sum += (*outSignals.FSW_TT1 - FSW_TT1_50);
    FSW_TT2sum += (*outSignals.FSW_TT2 - FSW_TT2_50);
    FSW_TT3sum += (*outSignals.FSW_TT3 - FSW_TT3_50);
    FSW_TT4sum += (*outSignals.FSW_TT4 - FSW_TT4_50);
    FSW_TT5sum += (*outSignals.FSW_TT5 - FSW_TT5_50);
    FSW_TT6sum += (*outSignals.FSW_TT6 - FSW_TT6_50);
    FSW_TT7sum += (*outSignals.FSW_TT7 - FSW_TT7_50);
    FSW_TT8sum += (*outSignals.FSW_TT8 - FSW_TT8_50);
    
    *outSignals.FSW_TT1_mean = FSW_TT1sum/currSample;
    *outSignals.FSW_TT2_mean = FSW_TT2sum/currSample;
    *outSignals.FSW_TT3_mean = FSW_TT3sum/currSample;
    *outSignals.FSW_TT4_mean = FSW_TT4sum/currSample;
    *outSignals.FSW_TT5_mean = FSW_TT5sum/currSample;
    *outSignals.FSW_TT6_mean = FSW_TT6sum/currSample;
    *outSignals.FSW_TT7_mean = FSW_TT7sum/currSample;
    *outSignals.FSW_TT8_mean = FSW_TT8sum/currSample;

   // if(currSample == 50)
    if(firstFSBTurn && *inSignals.FSB_TT1 > 0)
    {
	firstFSBTurn = false;
	FSB_TT1_50 = *outSignals.FSB_TT1;
	FSB_TT2_50 = *outSignals.FSB_TT2;
	FSB_TT3_50 = *outSignals.FSB_TT3;
	FSB_TT4_50 = *outSignals.FSB_TT4;
	FSB_TT5_50 = *outSignals.FSB_TT5;
	FSB_TT6_50 = *outSignals.FSB_TT6;
	FSB_TT7_50 = *outSignals.FSB_TT7;
	FSB_TT8_50 = *outSignals.FSB_TT8;
	FSW_TT1_50 = *outSignals.FSW_TT1;
	FSW_TT2_50 = *outSignals.FSW_TT2;
	FSW_TT3_50 = *outSignals.FSW_TT3;
	FSW_TT4_50 = *outSignals.FSW_TT4;
	FSW_TT5_50 = *outSignals.FSW_TT5;
	FSW_TT6_50 = *outSignals.FSW_TT6;
	FSW_TT7_50 = *outSignals.FSW_TT7;
	FSW_TT8_50 = *outSignals.FSW_TT8;
    }

    
    *outSignals.RF1 = calibTCN(*inSignals.RF1);
    *outSignals.RF2 = calibTCN(*inSignals.RF2);
    *outSignals.RF3 = calibTCN(*inSignals.RF3);
    *outSignals.RF4 = calibTCN(*inSignals.RF4);
    
    if(firstRFTurn && *outSignals.RF1 > 0)
    {
      firstRFTurn = false;
      firstRF1 = *outSignals.RF1;
      firstRF2 = *outSignals.RF2;
      firstRF3 = *outSignals.RF3;
      firstRF4 = *outSignals.RF4;
    }
    RF1sum += (*outSignals.RF1 - firstRF1);
    RF2sum += (*outSignals.RF2 - firstRF2);
    RF3sum += (*outSignals.RF3 - firstRF3);
    RF4sum += (*outSignals.RF4 - firstRF4);
  
    *outSignals.RF1ave = RF1sum/currSample;
    *outSignals.RF2ave = RF2sum/currSample;
    *outSignals.RF3ave = RF3sum/currSample;
    *outSignals.RF4ave = RF4sum/currSample;
    
    *outSignals.TC_BP21 = getSmoothed(calibTCN(*inSignals.TC_BP21), TC_BP21hist);
    *outSignals.TC_BP22 = getSmoothed(calibTCN(*inSignals.TC_BP22), TC_BP22hist);
    *outSignals.TC_BP31 = getSmoothed(calibTCN(*inSignals.TC_BP31), TC_BP31hist);
    *outSignals.TC_BP32 = getSmoothed(calibTCN(*inSignals.TC_BP32), TC_BP32hist);
    *outSignals.TC_BP41 = getSmoothed(calibTCN(*inSignals.TC_BP41), TC_BP41hist);
    *outSignals.TC_BP42 = getSmoothed(calibTCN(*inSignals.TC_BP42), TC_BP42hist);
    
    if(firstTCTurn && *outSignals.TC_BP21 > 0)
    {
	firstTCTurn = false;
	firstTC_BP21 = *outSignals.TC_BP21;
	firstTC_BP22 = *outSignals.TC_BP22;
	firstTC_BP31 = *outSignals.TC_BP31;
	firstTC_BP32 = *outSignals.TC_BP32;
	firstTC_BP41 = *outSignals.TC_BP41;
	firstTC_BP42 = *outSignals.TC_BP42;
    }

    TC_BP21sum += *outSignals.TC_BP21 - firstTC_BP21;
    TC_BP22sum += *outSignals.TC_BP22 - firstTC_BP22;
    TC_BP31sum += *outSignals.TC_BP31 - firstTC_BP31;
    TC_BP32sum += *outSignals.TC_BP32 - firstTC_BP32;
    TC_BP41sum += *outSignals.TC_BP41 - firstTC_BP41;
    TC_BP42sum += *outSignals.TC_BP42 - firstTC_BP42;
    
    *outSignals.BP21heat = TC_BP21sum/currSample;
    *outSignals.BP22heat = TC_BP22sum/currSample;
    *outSignals.BP31heat = TC_BP31sum/currSample;
    *outSignals.BP32heat = TC_BP32sum/currSample;
    *outSignals.BP41heat = TC_BP41sum/currSample;
    *outSignals.BP42heat = TC_BP42sum/currSample;
    
    
    *outSignals.PG_TT12 = getSmoothed(calibTCN(*inSignals.PG_TT12), PG_TT12hist);
    *outSignals.PG_TT13 = getSmoothed(calibTCN(*inSignals.PG_TT13), PG_TT13hist);
    *outSignals.PG_TT22 = getSmoothed(calibTCN(*inSignals.PG_TT22), PG_TT22hist);
    *outSignals.PG_TT23 = getSmoothed(calibTCN(*inSignals.PG_TT23), PG_TT23hist);
    *outSignals.PG_TT32 = getSmoothed(calibTCN(*inSignals.PG_TT32), PG_TT32hist);
    *outSignals.PG_TT33 = getSmoothed(calibTCN(*inSignals.PG_TT33), PG_TT33hist);
    *outSignals.PG_TT42 = getSmoothed(calibTCN(*inSignals.PG_TT42), PG_TT42hist);
    *outSignals.PG_TT43 = getSmoothed(calibTCN(*inSignals.PG_TT43), PG_TT43hist);
    
    if(firstPGTurn &&  *outSignals.PG_TT12 > 0)
    {
	firstPGTurn = false;
	firstPG_TT12 = *outSignals.PG_TT12;
	firstPG_TT13 = *outSignals.PG_TT13;
	firstPG_TT22 = *outSignals.PG_TT22;
	firstPG_TT23 = *outSignals.PG_TT23;
	firstPG_TT32 = *outSignals.PG_TT32;
	firstPG_TT33 = *outSignals.PG_TT33;
	firstPG_TT42 = *outSignals.PG_TT42;
	firstPG_TT43 = *outSignals.PG_TT43;
    }
    PG_TT12sum += *outSignals.PG_TT12 - firstPG_TT12;
    PG_TT13sum += *outSignals.PG_TT13 - firstPG_TT13;
    PG_TT22sum += *outSignals.PG_TT22 - firstPG_TT22;
    PG_TT23sum += *outSignals.PG_TT23 - firstPG_TT23;
    PG_TT32sum += *outSignals.PG_TT32 - firstPG_TT32;
    PG_TT33sum += *outSignals.PG_TT33 - firstPG_TT33;
    PG_TT42sum += *outSignals.PG_TT42 - firstPG_TT42;
    PG_TT43sum += *outSignals.PG_TT43 - firstPG_TT43;

    
    *outSignals.PG12heat = PG_TT12sum/currSample;
    *outSignals.PG13heat = PG_TT13sum/currSample;
    *outSignals.PG22heat = PG_TT22sum/currSample;
    *outSignals.PG23heat = PG_TT23sum/currSample;
    *outSignals.PG32heat = PG_TT32sum/currSample;
    *outSignals.PG33heat = PG_TT33sum/currSample;
    *outSignals.PG42heat = PG_TT42sum/currSample;
    *outSignals.PG43heat = PG_TT43sum/currSample;


    *outSignals.Tout_FSLW1_trend = *inSignals.Tout_FSLW1_trend;
    *outSignals.Tout_FSLW2_trend = *inSignals.Tout_FSLW2_trend;
    *outSignals.Tout_RF_trend = *inSignals.Tout_RF_trend;
    *outSignals.Tout_FSBP_trend = *inSignals.Tout_FSBP_trend;
    *outSignals.Tout_SCLW_trend = *inSignals.Tout_SCLW_trend;
    *outSignals.Tout_BP_trend = *inSignals.Tout_BP_trend;
    *outSignals.Tout_DP_trend = *inSignals.Tout_DP_trend;
    *outSignals.Tout_PG1_trend = *inSignals.Tout_PG1_trend;
    *outSignals.Tout_PG2_trend = *inSignals.Tout_PG2_trend;
    *outSignals.Tout_PG3_trend = *inSignals.Tout_PG3_trend;
    *outSignals.Tout_PG4_trend = *inSignals.Tout_PG4_trend;
    *outSignals.Tout_EG1_trend = *inSignals.Tout_EG1_trend;
    *outSignals.Tout_EG2_trend = *inSignals.Tout_EG2_trend;
    *outSignals.Tout_EG3_trend = *inSignals.Tout_EG3_trend;
    *outSignals.Tout_EG4_trend = *inSignals.Tout_EG4_trend;
    *outSignals.Tout_GG1_trend = *inSignals.Tout_GG1_trend;
    *outSignals.Tout_GG2_trend = *inSignals.Tout_GG2_trend;
    *outSignals.Tout_GG3_trend = *inSignals.Tout_GG3_trend;
    *outSignals.Tout_GG4_trend = *inSignals.Tout_GG4_trend;
    *outSignals.Tin_PC02_trend = *inSignals.Tin_PC02_trend;
    *outSignals.Tin_PC03_trend = *inSignals.Tin_PC03_trend;
    
    Tin_PC02_trend[trendIdx] = *inSignals.Tin_PC02_trend;
    Tin_PC03_trend[trendIdx] = *inSignals.Tin_PC03_trend;

    
    flow_PGsum += *inSignals.flow_PG;
    flow_EGsum += *inSignals.flow_EG;
    flow_GGsum += *inSignals.flow_GG;
    flow_FSLWsum += *inSignals.flow_FSLW;
    flow_RFsum += *inSignals.flow_RF;
    flow_FSBPsum += *inSignals.flow_FSBP;
    flow_SCLWsum += *inSignals.flow_SCLW;
    flow_BPsum += *inSignals.flow_BP;
    flow_DPsum += *inSignals.flow_DP;
	  
    float delay_FSLW=3.32*125/(flow_FSLWsum/totSamples);
    float delay_FSBP=3.84*65/(flow_FSBPsum/totSamples);
    float delay_SCLW=3.98*85/(flow_SCLWsum/totSamples);
    float delay_BP=0.58*375/(flow_BPsum/totSamples);
    float delay_DP=4.65*95/(flow_DPsum/totSamples);
    float delay_PG=0.82*375/(flow_PGsum/totSamples);

//     printf("DELAY FSLW : %f\n", delay_FSLW);
//     printf("DELAY FSBP : %f\n", delay_FSBP);
//     printf("DELAY SCLW : %f\n", delay_SCLW);
//     printf("DELAY BP : %f\n", delay_BP);
//     printf("DELAY DP : %f\n", delay_DP);
//     printf("DELAY PG : %f\n", delay_PG);
// 
    
    
    uint32 delay_FSLWint = 0.5 + delay_FSLW / period;
    uint32 delay_FSBPint = 0.5 + delay_FSBP / period;
    uint32 delay_SCLWint = 0.5 + delay_SCLW / period;
    uint32 delay_BPint = 0.5 + delay_BP / period;
    uint32 delay_DPint = 0.5 + delay_DP / period;
    uint32 delay_PGint = 0.5 + delay_PG / period;
  
    uint32 delay_RFint =0;
    uint32 delay_EGint =0;
    uint32 delay_GGint =0;
    
    uint32 delay;
    
    
    delay = trendIdx - delay_FSLWint;
    if(delay < 0) delay += MAX_TRENDHISTORY; 
    if(delay < 0) delay = 0;
    if(delay>= MAX_TRENDHISTORY) delay = MAX_TRENDHISTORY - 1;
    
    
    *outSignals.DT_FSLW1 = (totSamples <= delay_FSLWint)?0:*inSignals.Tout_FSLW1_trend - Tin_PC03_trend[delay];
   // - getCalAveraged(*inSignals.Tout_FSLW1_trend, Tout_FSLW1_his) + getCalPGAveraged(Tin_PC03_trend, delay_FSLWint);
    
    delay = trendIdx - delay_FSLWint;
    if(delay < 0) delay += MAX_TRENDHISTORY; 
    if(delay < 0) delay = 0;
    if(delay>= MAX_TRENDHISTORY) delay = MAX_TRENDHISTORY - 1;
    *outSignals.DT_FSLW2 = (totSamples <= delay_FSLWint)?0:*inSignals.Tout_FSLW2_trend - Tin_PC03_trend[delay];
    //- getCalAveraged(*inSignals.Tout_FSLW1_trend, Tout_FSLW2_his) + getCalPGAveraged(Tin_PC03_trend, delay_FSLWint);
    
    delay = trendIdx - delay_RFint;
    if(delay < 0) delay += MAX_TRENDHISTORY; 
     if(delay < 0) delay = 0;
    if(delay>= MAX_TRENDHISTORY) delay = MAX_TRENDHISTORY - 1;
    *outSignals.DT_RF = (totSamples <= delay_RFint)?0:*inSignals.Tout_RF_trend - Tin_PC02_trend[delay];
    //- getCalAveraged(*inSignals.Tout_RF_trend, Tout_RF_his) + getCalPGAveraged(Tin_PC02_trend, delay_RFint);

     if(delay < 0) delay = 0;
    if(delay>= MAX_TRENDHISTORY) delay = MAX_TRENDHISTORY - 1;
    delay = trendIdx - delay_FSBPint;
    if(delay < 0) delay += MAX_TRENDHISTORY; 
    if(delay < 0) delay = 0;
    if(delay>= MAX_TRENDHISTORY) delay = MAX_TRENDHISTORY - 1;
    *outSignals.DT_FSBP = (totSamples <= delay_FSBPint)?0:*inSignals.Tout_FSBP_trend - Tin_PC03_trend[delay];
   // - getCalAveraged(*inSignals.Tout_FSBP_trend, Tout_FSBP_his) + getCalPGAveraged(Tin_PC03_trend, delay_FSBPint);
    
    delay = trendIdx - delay_SCLWint;
    if(delay < 0) delay += MAX_TRENDHISTORY; 
    if(delay < 0) delay = 0;
    if(delay>= MAX_TRENDHISTORY) delay = MAX_TRENDHISTORY - 1;
    *outSignals.DT_SCLW = (totSamples <= delay_SCLWint)?0:*inSignals.Tout_SCLW_trend - Tin_PC03_trend[delay];
   // - getCalAveraged(*inSignals.Tout_SCLW_trend, Tout_SCLW_his) + getCalPGAveraged(Tin_PC03_trend, delay_SCLWint);
    
    delay = trendIdx - delay_BPint;
    if(delay < 0) delay += MAX_TRENDHISTORY; 
    if(delay < 0) delay = 0;
    if(delay>= MAX_TRENDHISTORY) delay = MAX_TRENDHISTORY - 1;
    *outSignals.DT_BP = (totSamples <= delay_BPint)?0:*inSignals.Tout_BP_trend - Tin_PC03_trend[delay];
   // - getCalAveraged(*inSignals.Tout_BP_trend, Tout_BP_his) + getCalPGAveraged(Tin_PC03_trend, delay_BPint);
    
    delay = trendIdx - delay_DPint;
    if(delay < 0) delay += MAX_TRENDHISTORY; 
    if(delay < 0) delay = 0;
    if(delay>= MAX_TRENDHISTORY) delay = MAX_TRENDHISTORY - 1;
    *outSignals.DT_DP = (totSamples <= delay_DPint)?0:*inSignals.Tout_DP_trend - Tin_PC03_trend[delay];
   // - getCalAveraged(*inSignals.Tout_DP_trend, Tout_DP_his) + getCalPGAveraged(Tin_PC03_trend, delay_DPint);
    
    delay = trendIdx - delay_PGint;
    if(delay < 0) delay += MAX_TRENDHISTORY; 
    if(delay < 0) delay = 0;
    if(delay>= MAX_TRENDHISTORY) delay = MAX_TRENDHISTORY - 1;
    *outSignals.DT_PG1 = (totSamples <= delay_PGint)?0:*inSignals.Tout_PG1_trend - Tin_PC03_trend[delay];
    //- getCalAveraged(*inSignals.Tout_PG1_trend, Tout_PG1_his) + getCalPGAveraged(Tin_PC03_trend, delay_PGint);
    
    delay = trendIdx - delay_PGint;
    if(delay < 0) delay += MAX_TRENDHISTORY; 
    if(delay < 0) delay = 0;
    if(delay>= MAX_TRENDHISTORY) delay = MAX_TRENDHISTORY - 1;
    *outSignals.DT_PG2 = (totSamples <= delay_PGint)?0:*inSignals.Tout_PG2_trend - Tin_PC03_trend[delay];
    //- getCalAveraged(*inSignals.Tout_PG2_trend, Tout_PG2_his) + getCalPGAveraged(Tin_PC03_trend, delay_PGint);
    
    delay = trendIdx - delay_PGint;
    if(delay < 0) delay += MAX_TRENDHISTORY; 
    if(delay < 0) delay = 0;
    if(delay>= MAX_TRENDHISTORY) delay = MAX_TRENDHISTORY - 1;
    *outSignals.DT_PG3 = (totSamples <= delay_PGint)?0:*inSignals.Tout_PG3_trend - Tin_PC03_trend[delay];
    //- getCalAveraged(*inSignals.Tout_PG3_trend, Tout_PG3_his) + getCalPGAveraged(Tin_PC03_trend, delay_PGint);
    
    delay = trendIdx - delay_PGint;
    if(delay < 0) delay += MAX_TRENDHISTORY; 
    if(delay < 0) delay = 0;
    if(delay>= MAX_TRENDHISTORY) delay = MAX_TRENDHISTORY - 1;
    *outSignals.DT_PG4 = (totSamples <= delay_PGint)?0:*inSignals.Tout_PG4_trend - Tin_PC03_trend[delay];
   // - getCalAveraged(*inSignals.Tout_PG4_trend, Tout_PG4_his) + getCalPGAveraged(Tin_PC03_trend, delay_PGint);
    
    delay = trendIdx - delay_EGint;
    if(delay < 0) delay += MAX_TRENDHISTORY; 
    if(delay < 0) delay = 0;
    if(delay>= MAX_TRENDHISTORY) delay = MAX_TRENDHISTORY - 1;
    *outSignals.DT_EG1 = (totSamples <= delay_EGint)?0:*inSignals.Tout_EG1_trend - Tin_PC02_trend[delay];
   // - getCalAveraged(*inSignals.Tout_EG1_trend, Tout_EG1_his) + getCalPGAveraged(Tin_PC02_trend, delay_EGint);
    
    delay = trendIdx - delay_EGint;
    if(delay < 0) delay += MAX_TRENDHISTORY; 
    if(delay < 0) delay = 0;
    if(delay>= MAX_TRENDHISTORY) delay = MAX_TRENDHISTORY - 1;
    *outSignals.DT_EG2 = (totSamples <= delay_EGint)?0:*inSignals.Tout_EG2_trend - Tin_PC02_trend[delay];
    //- getCalAveraged(*inSignals.Tout_EG2_trend, Tout_EG2_his) + getCalPGAveraged(Tin_PC02_trend, delay_EGint);
    
    delay = trendIdx - delay_EGint;
    if(delay < 0) delay += MAX_TRENDHISTORY; 
    if(delay < 0) delay = 0;
    if(delay>= MAX_TRENDHISTORY) delay = MAX_TRENDHISTORY - 1;
    *outSignals.DT_EG3 = (totSamples <= delay_EGint)?0:*inSignals.Tout_EG3_trend - Tin_PC02_trend[delay];
   // - getCalAveraged(*inSignals.Tout_EG3_trend, Tout_EG3_his) + getCalPGAveraged(Tin_PC02_trend, delay_EGint);
    
    delay = trendIdx - delay_EGint;
    if(delay < 0) delay += MAX_TRENDHISTORY; 
    if(delay < 0) delay = 0;
    if(delay>= MAX_TRENDHISTORY) delay = MAX_TRENDHISTORY - 1;
    *outSignals.DT_EG4 = (totSamples <= delay_EGint)?0:*inSignals.Tout_EG4_trend - Tin_PC02_trend[delay];
   // - getCalAveraged(*inSignals.Tout_EG4_trend, Tout_EG4_his) + getCalPGAveraged(Tin_PC02_trend, delay_EGint);
    
    delay = trendIdx - delay_GGint;
    if(delay < 0) delay += MAX_TRENDHISTORY; 
    if(delay < 0) delay = 0;
    if(delay>= MAX_TRENDHISTORY) delay = MAX_TRENDHISTORY - 1;
    *outSignals.DT_GG1 = (totSamples <= delay_GGint)?0:*inSignals.Tout_GG1_trend - Tin_PC02_trend[delay];
   // - getCalAveraged(*inSignals.Tout_GG1_trend, Tout_GG1_his) + getCalPGAveraged(Tin_PC02_trend, delay_GGint);
    
    delay = trendIdx - delay_GGint;
    if(delay < 0) delay += MAX_TRENDHISTORY; 
    if(delay < 0) delay = 0;
    if(delay>= MAX_TRENDHISTORY) delay = MAX_TRENDHISTORY - 1;
    *outSignals.DT_GG2 = (totSamples <= delay_GGint)?0:*inSignals.Tout_GG2_trend - Tin_PC02_trend[delay];
   // - getCalAveraged(*inSignals.Tout_GG2_trend, Tout_GG2_his) + getCalPGAveraged(Tin_PC02_trend, delay_GGint);
    
    delay = trendIdx - delay_GGint;
    if(delay < 0) delay += MAX_TRENDHISTORY; 
    if(delay < 0) delay = 0;
    if(delay>= MAX_TRENDHISTORY) delay = MAX_TRENDHISTORY - 1;
    *outSignals.DT_GG3 = (totSamples <= delay_GGint)?0:*inSignals.Tout_GG3_trend - Tin_PC02_trend[delay];
   // - getCalAveraged(*inSignals.Tout_GG3_trend, Tout_GG3_his) + getCalPGAveraged(Tin_PC02_trend, delay_GGint);
    
    delay = trendIdx - delay_GGint;
    if(delay < 0) delay += MAX_TRENDHISTORY; 
    if(delay < 0) delay = 0;
    if(delay>= MAX_TRENDHISTORY) delay = MAX_TRENDHISTORY - 1;
    *outSignals.DT_GG4 = (totSamples <= delay_GGint)?0:*inSignals.Tout_GG4_trend - Tin_PC02_trend[delay];
    //- getCalAveraged(*inSignals.Tout_GG4_trend, Tout_GG4_his) + getCalPGAveraged(Tin_PC02_trend, delay_GGint);
    DeltaT_FSLW1sum += *outSignals.DT_FSLW1;
    DeltaT_FSLW2sum += *outSignals.DT_FSLW2;
    DeltaT_RFsum += *outSignals.DT_RF;
    DeltaT_FSBPsum += *outSignals.DT_FSBP;
    DeltaT_SCLWsum += *outSignals.DT_SCLW;
    DeltaT_BPsum += *outSignals.DT_BP;
    DeltaT_DPsum += *outSignals.DT_DP;
    DeltaT_PG1sum += *outSignals.DT_PG1;
    DeltaT_PG2sum += *outSignals.DT_PG2;
    DeltaT_PG3sum += *outSignals.DT_PG3;
    DeltaT_PG4sum += *outSignals.DT_PG4;
    DeltaT_EG1sum += *outSignals.DT_EG1;
    DeltaT_EG2sum += *outSignals.DT_EG2;
    DeltaT_EG3sum += *outSignals.DT_EG3;
    DeltaT_EG4sum += *outSignals.DT_EG4;
    DeltaT_GG1sum += *outSignals.DT_GG1;
    DeltaT_GG2sum += *outSignals.DT_GG2;
    DeltaT_GG3sum += *outSignals.DT_GG3;
    DeltaT_GG4sum += *outSignals.DT_GG4;

    double RF1_heat = RF1_pfsum * 1000 * period;
    double RF2_heat = RF2_pfsum * 1000 * period;
    double RF3_heat = RF3_pfsum * 1000 * period;
    double RF4_heat = RF4_pfsum * 1000 * period;
    double rfTot_heat = RF1_heat + RF2_heat + RF3_heat + RF4_heat;
    
    //TACON
    //rfTot_heat = 1;
    
    
    
    if( rfTot_heat== 0)
    {
      *outSignals.DT_FSLW1_heat = 0;
      *outSignals.DT_FSLW2_heat = 0;
      *outSignals.DT_RF_heat = 0;
      *outSignals.DT_FSBP_heat = 0;
      *outSignals.DT_SCLW_heat = 0;
      *outSignals.DT_BP_heat = 0;
      *outSignals.DT_DP_heat = 0;
      *outSignals.DT_PG1_heat =  0;
      *outSignals.DT_PG2_heat =  0;
      *outSignals.DT_PG3_heat =  0;
      *outSignals.DT_PG4_heat =  0;
      *outSignals.DT_EG1_heat =  0;
      *outSignals.DT_EG2_heat =  0;
      *outSignals.DT_EG3_heat =  0;
      *outSignals.DT_EG4_heat =  0;
      *outSignals.DT_GG1_heat =  0;
      *outSignals.DT_GG2_heat =  0;
      *outSignals.DT_GG3_heat =  0;
      *outSignals.DT_GG4_heat =  0;
      *outSignals.DT_Tot_heat =  0;
    }
    else
    {
      uint32 startDispSample = 400;
      *outSignals.DT_FSLW1_heat = (currSample <= startDispSample)?0:((DeltaT_FSLW1sum * period)*(flow_FSLWsum/totSamples) /2/3.6*4183) / rfTot_heat;
      *outSignals.DT_FSLW2_heat = (currSample <= startDispSample)?0:DeltaT_FSLW2sum * flow_FSLWsum/totSamples /2/3.6*4183*period / rfTot_heat;
      *outSignals.DT_RF_heat = (currSample <= startDispSample)?0:DeltaT_RFsum * flow_RFsum/totSamples /3.6*4183*period / rfTot_heat;
      *outSignals.DT_FSBP_heat = (currSample <= startDispSample)?0:DeltaT_FSBPsum * flow_FSBPsum/totSamples /3.6*4183*period / rfTot_heat;
      *outSignals.DT_SCLW_heat = (currSample <= startDispSample)?0:DeltaT_SCLWsum * flow_SCLWsum/totSamples /3.6*4183*period / rfTot_heat;
      *outSignals.DT_BP_heat = (currSample <= startDispSample)?0:DeltaT_BPsum * flow_BPsum/totSamples /3.6*4183*period / rfTot_heat;
      *outSignals.DT_DP_heat = (currSample <= startDispSample)?0:DeltaT_DPsum * flow_DPsum/totSamples /3.6*4183*period / rfTot_heat;
      *outSignals.DT_PG1_heat = (currSample <= startDispSample)?0:DeltaT_PG1sum * flow_PGsum/totSamples /4/3.6*4183*period / rfTot_heat;
      *outSignals.DT_PG2_heat = (currSample <= startDispSample)?0:DeltaT_PG2sum * flow_PGsum/totSamples /4/3.6*4183*period / rfTot_heat;
      *outSignals.DT_PG3_heat = (currSample <= startDispSample)?0:DeltaT_PG3sum * flow_PGsum/totSamples /4/3.6*4183*period / rfTot_heat;
      *outSignals.DT_PG4_heat = (currSample <= startDispSample)?0:DeltaT_PG4sum * flow_PGsum/totSamples /4/3.6*4183*period / rfTot_heat;
      *outSignals.DT_EG1_heat = (currSample <= startDispSample)?0:DeltaT_EG1sum * flow_EGsum/totSamples /4/3.6*4183*period / rfTot_heat;
      *outSignals.DT_EG2_heat = (currSample <= startDispSample)?0:DeltaT_EG2sum * flow_EGsum/totSamples /4/3.6*4183*period / rfTot_heat;
      *outSignals.DT_EG3_heat = (currSample <= startDispSample)?0:DeltaT_EG3sum * flow_EGsum/totSamples /4/3.6*4183*period / rfTot_heat;
      *outSignals.DT_EG4_heat = (currSample <= startDispSample)?0:DeltaT_EG4sum * flow_EGsum/totSamples /4/3.6*4183*period / rfTot_heat;
      *outSignals.DT_GG1_heat = (currSample <= startDispSample)?0:DeltaT_GG1sum * flow_GGsum/totSamples /4/3.6*4183*period / rfTot_heat;
      *outSignals.DT_GG2_heat = (currSample <= startDispSample)?0:DeltaT_GG2sum * flow_GGsum/totSamples /4/3.6*4183*period / rfTot_heat;
      *outSignals.DT_GG3_heat = (currSample <= startDispSample)?0:DeltaT_GG3sum * flow_GGsum/totSamples /4/3.6*4183*period / rfTot_heat;
      *outSignals.DT_GG4_heat = (currSample <= startDispSample)?0:DeltaT_GG4sum * flow_GGsum/totSamples /4/3.6*4183*period / rfTot_heat;
      *outSignals.DT_Tot_heat = (*outSignals.DT_FSLW1_heat+ *outSignals.DT_FSLW2_heat + *outSignals.DT_RF_heat +
      *outSignals.DT_FSBP_heat + *outSignals.DT_SCLW_heat + *outSignals.DT_BP_heat + 
      *outSignals.DT_DP_heat + *outSignals.DT_PG1_heat + *outSignals.DT_PG2_heat + 
      *outSignals.DT_PG3_heat + *outSignals.DT_PG4_heat);
    }
    if(*inSignals.RF1_Pf > 0 || currSample > 1)
	currSample++;
    trendIdx++;
    if(trendIdx >= MAX_TRENDHISTORY)
      trendIdx = 0;
    smoothIdx++;
    if(smoothIdx >= SMOOTH_HISTORY)
      smoothIdx = 0;
    calIdx++;
    if(calIdx >= CAL_HISTORY)
      calIdx = 0;
    totSamples++;
    return true; 
}
	  
CLASS_REGISTER(SpiderCalGAM, "1.0")

}