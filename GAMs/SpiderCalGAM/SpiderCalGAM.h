/**
 * @file SpiderCalGAM.h
 * @brief Header file for class SpiderCalyGAM
 * @date June 16 , 2020
 * @author nn
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

 * @details This header file contains the declaration of the class MathExpressionGAM
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
 */

#ifndef SPIDERCALGAM_H_
#define SPIDERCALGAM_H_

/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/
//#pragma pack(1)

#include "GAM.h"
#include "StructuredDataI.h"
#include "MessageI.h"

#define MAX_TRENDHISTORY 10000
#define SMOOTH_HISTORY 5
#define CAL_HISTORY 100
/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/

namespace MARTe {

/**
 * @brief A GAM that computes the required values for SPIDER Calorimetry visualization 
 * for run-time expression evaluation.
 * @details This GAM performs the required computation for all caloriletry signals of interest. 
 * 
  * 
 * @todo Add support for constants specified in configuration file.
 */




class SpiderCalGAM : public GAM {
public:
	CLASS_REGISTER_DECLARATION()
	
	SpiderCalGAM();
	
	virtual ~SpiderCalGAM();
	
	/**
	 * @brief Initializes and retrieves informations from configuration file. 
	 * @details During the initialization phase number of inputs and outputs are
	 * read from the configuration file and expression are stored.
	 * @param[in] data the GAM configuration specified in the configuration file.
	 * @return true on succeed.
	 * @pre Each output signal must have an "Expression" parameter.
	 */
	virtual bool Initialise(StructuredDataI & data);
	
	/**
	 * @brief Checks parameters and compile exprtk expressions. 
	 * @details This method:
	 * 1. checks if types and dimensions retrieved from the configuration file are correct,
	 * 2. allocates a local float64 recast of GAM memory,
	 * 3. uses exprtk library to parse and compile expressions.
	 * 
	 * Expressions are compiled here since one of strenghts of exprtk is that
	 * expressions do not need to be recompiled each time they are evaluated,
	 * even if variables in the expression change their value.
	 * @return true on succeed.
	 * @pre Initialise().
	 */
	virtual bool Setup();
	
	/**
	 * @brief Evaluates expressions from configuration file and outputs them. 
	 * @details During ecxecution, expressions for each output signal are
	 * evaluated with data from input signals and results are copied in output
	 * memory.
	 * @return true
	 * @pre
	 *	Initialise() &&
	 *	Setup()
	 */
	virtual bool Execute();
	
private:
	/**
	 * @name Signal data
	 * Informations on inputs and outputs in this GAM are stored here.
	 */
	//@{
	float lowPassFilter(float x);
	bool GetSignalNames(const SignalDirection direction,StreamString* names);
	float getSmoothed(float val, float *history);
	float getCalAveraged(float val, float *history);
	float getCalPGAveraged(float *history, int delay);
	uint32 numOutputSignals;
	uint32 numInputSignals;
	
	bool firstRF_pfTurn;
	bool firstRFTurn;
	bool firstLightTurn;
 	bool firstTCTurn;
 	bool firstPGTurn;
	bool firstFSBTurn;
        float firstRF1_Pf;
        float firstRF2_Pf;
        float firstRF3_Pf;
        float firstRF4_Pf;
	float firstPLight1;
	float firstPLight2;
	float firstPLight3;
	float firstPLight4;
	float firstPLight5;
	float firstPLight6;
	float firstPLight7;
	float firstPLight8;
	
	float FSB_TT1_50;
	float FSB_TT2_50;
	float FSB_TT3_50;
	float FSB_TT4_50;
	float FSB_TT5_50;
	float FSB_TT6_50;
	float FSB_TT7_50;
	float FSB_TT8_50;
	
	float FSW_TT1_50;
	float FSW_TT2_50;
	float FSW_TT3_50;
	float FSW_TT4_50;
	float FSW_TT5_50;
	float FSW_TT6_50;
	float FSW_TT7_50;
	float FSW_TT8_50;

	float firstRF1;
        float firstRF2;
        float firstRF3;
        float firstRF4;

	float firstTC_BP21;
	float firstTC_BP22;
	float firstTC_BP31;
	float firstTC_BP32;
	float firstTC_BP41;
	float firstTC_BP42;
	
	float firstPG_TT12;
	float firstPG_TT13;
	float firstPG_TT22;
	float firstPG_TT23;
	float firstPG_TT32;
	float firstPG_TT33;
	float firstPG_TT42;
	float firstPG_TT43;
	
	
	uint32 currSample; 
	uint32 totSamples; 
	double RF1_pfsum;
	double RF2_pfsum;
	double RF3_pfsum;
	double RF4_pfsum;
	
	double PLight1sum;
	double PLight2sum;
	double PLight3sum;
	double PLight4sum;
	double PLight5sum;
	double PLight6sum;
	double PLight7sum;
	double PLight8sum;
	
	double FSB_TT1sum;
	double FSB_TT2sum;
	double FSB_TT3sum;
	double FSB_TT4sum;
	double FSB_TT5sum;
	double FSB_TT6sum;
	double FSB_TT7sum;
	double FSB_TT8sum;
	
	double FSW_TT1sum;
	double FSW_TT2sum;
	double FSW_TT3sum;
	double FSW_TT4sum;
	double FSW_TT5sum;
	double FSW_TT6sum;
	double FSW_TT7sum;
	double FSW_TT8sum;
	
	double RF1sum;
	double RF2sum;
	double RF3sum;
	double RF4sum;
	
	double TC_BP21sum;
	double TC_BP22sum;
	double TC_BP31sum;
	double TC_BP32sum;
	double TC_BP41sum;
	double TC_BP42sum;
	
	double PG_TT12sum;
	double PG_TT13sum;
	double PG_TT22sum;
	double PG_TT23sum;
	double PG_TT32sum;
	double PG_TT33sum;
	double PG_TT42sum;
	double PG_TT43sum;

	float TC_BP21hist[SMOOTH_HISTORY];
	float TC_BP22hist[SMOOTH_HISTORY];
	float TC_BP31hist[SMOOTH_HISTORY];
	float TC_BP32hist[SMOOTH_HISTORY];
	float TC_BP41hist[SMOOTH_HISTORY];
	float TC_BP42hist[SMOOTH_HISTORY];
	
	float PG_TT12hist[SMOOTH_HISTORY];
	float PG_TT13hist[SMOOTH_HISTORY];
	float PG_TT22hist[SMOOTH_HISTORY];
	float PG_TT23hist[SMOOTH_HISTORY];
	float PG_TT32hist[SMOOTH_HISTORY];
	float PG_TT33hist[SMOOTH_HISTORY];
	float PG_TT42hist[SMOOTH_HISTORY];
	float PG_TT43hist[SMOOTH_HISTORY];

	double flow_PGsum;
	double flow_EGsum;
	double flow_GGsum;
	double flow_FSLWsum;
	double flow_RFsum;
	double flow_FSBPsum;
	double flow_SCLWsum;
	double flow_BPsum;
	double flow_DPsum;

	double DeltaT_FSLW1sum;
	double DeltaT_FSLW2sum;
	double DeltaT_RFsum;
	double DeltaT_FSBPsum;
	double DeltaT_SCLWsum;
	double DeltaT_BPsum;
	double DeltaT_DPsum;
	double DeltaT_PG1sum;
	double DeltaT_PG2sum;
	double DeltaT_PG3sum;
	double DeltaT_PG4sum;
	double DeltaT_EG1sum;
	double DeltaT_EG2sum;
	double DeltaT_EG3sum;
	double DeltaT_EG4sum;
	double DeltaT_GG1sum;
	double DeltaT_GG2sum;
	double DeltaT_GG3sum;
	double DeltaT_GG4sum;


	float Tin_PC02_trend[MAX_TRENDHISTORY];
	float Tin_PC03_trend[MAX_TRENDHISTORY];
	
	float Tout_FSLW1_his[CAL_HISTORY];
	float Tout_FSLW2_his[CAL_HISTORY];
	float Tout_RF_his[CAL_HISTORY];
	float Tout_FSBP_his[CAL_HISTORY];
	float Tout_SCLW_his[CAL_HISTORY];
	float Tout_BP_his[CAL_HISTORY];
	float Tout_DP_his[CAL_HISTORY];
	float Tout_PG1_his[CAL_HISTORY];
	float Tout_PG2_his[CAL_HISTORY];
	float Tout_PG3_his[CAL_HISTORY];
	float Tout_PG4_his[CAL_HISTORY];
	float Tout_EG1_his[CAL_HISTORY];
	float Tout_EG2_his[CAL_HISTORY];
	float Tout_EG3_his[CAL_HISTORY];
	float Tout_EG4_his[CAL_HISTORY];
	float Tout_GG1_his[CAL_HISTORY];
	float Tout_GG2_his[CAL_HISTORY];
	float Tout_GG3_his[CAL_HISTORY];
	float Tout_GG4_his[CAL_HISTORY];

	uint32 trendIdx;

	
	struct {
	  float *VPGbias;  
	  float *Vbias;
	  float *IPG1;
	  float *Pv;
	  float *V_EG;
	  float *V_acc;
	  float *I_EG;
	  float *I_acc;
	  float *RF1_Pf;
	  float *RF2_Pf;
	  float *RF3_Pf;
	  float *RF4_Pf;
	  float *PLight1;
	  float *PLight2;
	  float *PLight3;
	  float *PLight4;
	  float *PLight5;
	  float *PLight6;
	  float *PLight7;
	  float *PLight8;
	  float *FSB_TT1;
	  float *FSB_TT2;
	  float *FSB_TT3;
	  float *FSB_TT4;
	  float *FSB_TT5;
	  float *FSB_TT6;
	  float *FSB_TT7;
	  float *FSB_TT8;
	  float *FSW_TT1;
	  float *FSW_TT2;
	  float *FSW_TT3;
	  float *FSW_TT4;
	  float *FSW_TT5;
	  float *FSW_TT6;
	  float *FSW_TT7;
	  float *FSW_TT8;
	  float *RF1;
	  float *RF2;
	  float *RF3;
	  float *RF4;
	  float *TC_BP21;
	  float *TC_BP22;
	  float *TC_BP31;
	  float *TC_BP32;
	  float *TC_BP41;
	  float *TC_BP42;
	  float *PG_TT12;
	  float *PG_TT13;
	  float *PG_TT22;
	  float *PG_TT23;
	  float *PG_TT32;
	  float *PG_TT33;
	  float *PG_TT42;
	  float *PG_TT43;
	  float *Tout_FSLW1_trend;
	  float *Tout_FSLW2_trend;
	  float *Tout_RF_trend;
	  float *Tout_FSBP_trend;
	  float *Tout_SCLW_trend;
	  float *Tout_BP_trend;
	  float *Tout_DP_trend;
	  float *Tout_PG1_trend;
	  float *Tout_PG2_trend;
	  float *Tout_PG3_trend;
	  float *Tout_PG4_trend;
	  float *Tout_EG1_trend;
	  float *Tout_EG2_trend;
	  float *Tout_EG3_trend;
	  float *Tout_EG4_trend;
	  float *Tout_GG1_trend;
	  float *Tout_GG2_trend;
	  float *Tout_GG3_trend;
	  float *Tout_GG4_trend;
	  float *Tin_PC02_trend;
	  float *Tin_PC03_trend;
	  float *flow_PG;
	  float *flow_EG;
	  float *flow_GG;
	  float *flow_FSLW;
	  float *flow_RF;
	  float *flow_FSBP;
	  float *flow_SCLW;
	  float *flow_BP;
	  float *flow_DP;
	} inSignals;
	
	struct  {
	  float *VPGbias;  
	  float *Vbias;
	  float *IPG1;
	  float *Pv;
	  float *V_EG;
	  float *V_acc;
	  float *I_EG;
	  float *I_acc;
	  float *Power_on_EG;
	  float *Peak_temp_blue;
	  float *Peak_temp_red;
	  float *RF1_Pf;
	  float *RF2_Pf;
	  float *RF3_Pf;
	  float *RF4_Pf;
	  float *PLight1;
	  float *PLight2;
	  float *PLight3;
	  float *PLight4;
	  float *PLight5;
	  float *PLight6;
	  float *PLight7;
	  float *PLight8;
	  float *RF1_pfave;
	  float *RF2_pfave;
	  float *RF3_pfave;
	  float *RF4_pfave;
	  float *PLight1ave;
	  float *PLight2ave;
	  float *PLight3ave;
	  float *PLight4ave;
	  float *PLight5ave;
	  float *PLight6ave;
	  float *PLight7ave;
	  float *PLight8ave;
	  float *FSB_TT1;
	  float *FSB_TT2;
	  float *FSB_TT3;
	  float *FSB_TT4;
	  float *FSB_TT5;
	  float *FSB_TT6;
	  float *FSB_TT7;
	  float *FSB_TT8;
	  float *FSB_TT1_mean;
	  float *FSB_TT2_mean;
	  float *FSB_TT3_mean;
	  float *FSB_TT4_mean;
	  float *FSB_TT5_mean;
	  float *FSB_TT6_mean;
	  float *FSB_TT7_mean;
	  float *FSB_TT8_mean;
	  float *FSW_TT1;
	  float *FSW_TT2;
	  float *FSW_TT3;
	  float *FSW_TT4;
	  float *FSW_TT5;
	  float *FSW_TT6;
	  float *FSW_TT7;
	  float *FSW_TT8;
	  float *FSW_TT1_mean;
	  float *FSW_TT2_mean;
	  float *FSW_TT3_mean;
	  float *FSW_TT4_mean;
	  float *FSW_TT5_mean;
	  float *FSW_TT6_mean;
	  float *FSW_TT7_mean;
	  float *FSW_TT8_mean;
	  float *RF1;
	  float *RF2;
	  float *RF3;
	  float *RF4;
	  float *RF1ave;
	  float *RF2ave;
	  float *RF3ave;
	  float *RF4ave;
	  float *TC_BP21;
	  float *TC_BP22;
	  float *TC_BP31;
	  float *TC_BP32;
	  float *TC_BP41;
	  float *TC_BP42;
	  float *BP21heat;
	  float *BP22heat;
	  float *BP31heat;
	  float *BP32heat;
	  float *BP41heat;
	  float *BP42heat;
	  float *PG_TT12;
	  float *PG_TT13;
	  float *PG_TT22;
	  float *PG_TT23;
	  float *PG_TT32;
	  float *PG_TT33;
	  float *PG_TT42;
	  float *PG_TT43;
	  float *PG_TT12heat;
	  float *PG_TT13heat;
	  float *PG_TT22heat;
	  float *PG_TT23heat;
	  float *PG_TT32heat;
	  float *PG_TT33heat;
	  float *PG_TT42heat;
	  float *PG_TT43heat;
	  float *PG12heat;
	  float *PG13heat;
	  float *PG22heat;
	  float *PG23heat;
	  float *PG32heat;
	  float *PG33heat;
	  float *PG42heat;
	  float *PG43heat;
	  float *Tout_FSLW1_trend;
	  float *Tout_FSLW2_trend;
	  float *Tout_RF_trend;
	  float *Tout_FSBP_trend;
	  float *Tout_SCLW_trend;
	  float *Tout_BP_trend;
	  float *Tout_DP_trend;
	  float *Tout_PG1_trend;
	  float *Tout_PG2_trend;
	  float *Tout_PG3_trend;
	  float *Tout_PG4_trend;
	  float *Tout_EG1_trend;
	  float *Tout_EG2_trend;
	  float *Tout_EG3_trend;
	  float *Tout_EG4_trend;
	  float *Tout_GG1_trend;
	  float *Tout_GG2_trend;
	  float *Tout_GG3_trend;
	  float *Tout_GG4_trend;
	  float *Tin_PC02_trend;
	  float *Tin_PC03_trend;
	  float *DT_FSLW1;
	  float *DT_FSLW2;
	  float *DT_RF;
	  float *DT_FSBP;
	  float *DT_SCLW;
	  float *DT_BP;
	  float *DT_DP;
	  float *DT_PG1;
	  float *DT_PG2;
	  float *DT_PG3;
	  float *DT_PG4;
	  float *DT_EG1;
	  float *DT_EG2;
	  float *DT_EG3;
	  float *DT_EG4;
	  float *DT_GG1;
	  float *DT_GG2;
	  float *DT_GG3;
	  float *DT_GG4;
	  float *DT_FSLW1_heat;
	  float *DT_FSLW2_heat;
	  float *DT_RF_heat;
	  float *DT_FSBP_heat;
	  float *DT_SCLW_heat;
	  float *DT_BP_heat;
	  float *DT_DP_heat;
	  float *DT_PG1_heat;
	  float *DT_PG2_heat;
	  float *DT_PG3_heat;
	  float *DT_PG4_heat;
	  float *DT_EG1_heat;
	  float *DT_EG2_heat;
	  float *DT_EG3_heat;
	  float *DT_EG4_heat;
	  float *DT_GG1_heat;
	  float *DT_GG2_heat;
	  float *DT_GG3_heat;
	  float *DT_GG4_heat;
	  float *DT_Tot_heat;
	}outSignals;
	
//20 Hz Low pass filter (10kHz sampling speed) history	
	float lowPassPrevX, lowPassPrevY;
	double calibTCN(float inX);
	float period;
	int smoothIdx;
	int calIdx;
};

} /* Namespace MARTe */

/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif 
	
