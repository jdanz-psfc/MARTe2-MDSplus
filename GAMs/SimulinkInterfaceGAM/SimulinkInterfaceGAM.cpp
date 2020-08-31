/**
 * @file SimulinkInterfaceGAM.cpp
 * @brief Source file for class SimulinkInterfaceGAM
 * @date May 24, 2019
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

 * @details This source file contains the definition of all the methods for the
 * class SimulinkInterfaceGAM (public, protected, and private). Be aware that some
 * methods, such as those inline could be defined on the header file, instead.
 */

#define DLL_API

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

#include <iostream>

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/

#include "SimulinkInterfaceGAM.h"
#include "AdvancedErrorManagement.h"

/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

namespace MARTe {

SimulinkInterfaceGAM::SimulinkInterfaceGAM() : GAM() {
}

SimulinkInterfaceGAM::~SimulinkInterfaceGAM() {
}

bool SimulinkInterfaceGAM::Initialise(StructuredDataI & data) {
	
	bool ok = GAM::Initialise(data);
	
	// Debug
	REPORT_ERROR(ErrorManagement::Debug, "INITIALIZE");
	isFirstRun = true;
	ok = data.Read("SleepTime", sleepTime);
	if (!ok) {
		
		REPORT_ERROR(ErrorManagement::Debug,
					 "Sleep time not set. By default it will be zeroed.");
		
		sleepTime = 0;
		
	}
	
	/***********************************************************************//**
	* The GAM reads the name of the model as declared in the configuration file,
	* and uses it to retrieve pointers to essential functions provided by
	* Simulink(R) C-APIs.
	***************************************************************************/
	
	ok = data.Read("ModelName", modelName);
	if (!ok) {
		
		REPORT_ERROR(ErrorManagement::ParametersError,
					 "Error while reading ModelName from configuration file. Was it defined?");
		return ok;
		
	}
	
	ok = data.Read("FileName", fileName);
	if (!ok) {
		
		REPORT_ERROR(ErrorManagement::Information,
					 "FileName not declared. Assuming it is the same as ModelName.");
		
		fileName = modelName;
		
	}
	
	// Provisional storage of symbol names.
	char8* libraryName;
	char8* initFuncName;
	char8* stepFuncName;
	char8* termFuncName;
	
	// Library opening.
	libraryName = fileName.BufferReference();
	ok = StringHelper::Concatenate(libraryName, ".so");
	if (!ok) {
		REPORT_ERROR(ErrorManagement::InitialisationError,
					 "Symbol name generation failed.");
		return ok;
	}
	
	ok = modelLibrary.Open(libraryName);
	if (!ok) {
		REPORT_ERROR(ErrorManagement::InitialisationError,
					 "Failed to load model library %s", libraryName);
		return ok;
	}
	
	// Initialization function pointer retrieval.
	initFuncName = modelName.BufferReference();
	ok = StringHelper::Concatenate(initFuncName, "_initialize");
	if (!ok) {
		REPORT_ERROR(ErrorManagement::InitialisationError,
					 "Symbol name generation failed.");
		return ok;
	}
	
	modelFuncPtr_Init = reinterpret_cast<void(*)(void)>(modelLibrary.Function(initFuncName));	// void(*)(void) is a pointer to a void function that takes no arguments
	ok = modelFuncPtr_Init != static_cast<void(*)(void)>(NULL);
	if (!ok) {
		REPORT_ERROR(ErrorManagement::InitialisationError,
					 "Failed to load model initialize function.");
		return ok;
	}
	
	// Step function pointer retrieval.
	stepFuncName = modelName.BufferReference();
	ok = StringHelper::Concatenate(stepFuncName, "_step");
	if (!ok) {
		REPORT_ERROR(ErrorManagement::InitialisationError,
					 "Symbol name generation failed.");
		return ok;
	}
	
	modelFuncPtr_Step = reinterpret_cast<void(*)(void)>(modelLibrary.Function(stepFuncName));
	ok = modelFuncPtr_Step != static_cast<void(*)(void)>(NULL);
	if (!ok) {
		REPORT_ERROR(ErrorManagement::InitialisationError,
					 "Failed to load model step function.");
		return ok;
	}
	
	// Termination function pointer retrieval.
	termFuncName = modelName.BufferReference();
	ok = StringHelper::Concatenate(stepFuncName, "_terminate");
	if (!ok) {
		REPORT_ERROR(ErrorManagement::InitialisationError,
					 "Symbol name generation failed.");
		return ok;
	}
	
	modelFuncPtr_Term = reinterpret_cast<void(*)(void)>(modelLibrary.Function(termFuncName));
	ok = modelFuncPtr_Term != static_cast<void(*)(void)>(NULL);
	if (!ok) {
		REPORT_ERROR(ErrorManagement::InitialisationError,
					 "Failed to load model terminate function.");
		return ok;
	}
	
	// Finally we need a pointer to the ModelMappingInfo struct, that is where
	// Simulink(R) stores information about each single model.
	// Unfortunately the Coder does not provide an API to retrieve this, so it
	// has to be added manually:
	// In Configuration Parameters > Code Generator > Custom Code
	// add these lines to Source File:
	//
	// void* GetMmiPtr(void)
	// {
	// 	rtwCAPI_ModelMappingInfo* mmiPtr = &(rtmGetDataMapInfo(<ModelName>_M).mmi);
	// 
	// 	return (void*) mmiPtr;
	// }
	getMmiPtr = reinterpret_cast<void*(*)(void)>(modelLibrary.Function("GetMmiPtr"));
	ok = getMmiPtr != static_cast<void*(*)(void)>(NULL);
	
	if (!ok) {
		
		REPORT_ERROR(ErrorManagement::InitialisationError,
					 "Failed to load GetMmiPtr function.");
		return ok;
	}
	
	/***********************************************************************//**
	 * Model is initialized by calling the model initialize function.
	 **************************************************************************/
	
	// Model initialization function call
	modelFuncPtr_Init();
	
	// mmi stores general infos and maps storing more infos
	//RT_MODEL_proveVarieDue_T* mmiPtr = (RT_MODEL_proveVarieDue_T*) getMmiPtr(); 
	//rtwCAPI_ModelMappingInfo* mmi = &(rtmGetDataMapInfo(mmiPtr).mmi);
	rtwCAPI_ModelMappingInfo* mmi = (rtwCAPI_ModelMappingInfo*) getMmiPtr();
	
	/***********************************************************************//**
	 * Built-in C API macros are then used to extract information on the model
	 * from mmi, and store them somewhere to be used later.
	 **************************************************************************/
	
	// How many signals and parameters are used
	numRootInputs  = rtwCAPI_GetNumRootInputs(mmi);
	numRootOutputs = rtwCAPI_GetNumRootOutputs(mmi);
	numParameters  = rtwCAPI_GetNumModelParameters(mmi);
	
	// C API root input/output structures (pointer to)
	modelRootInputs  = rtwCAPI_GetRootInputs(mmi);
	modelRootOutputs = rtwCAPI_GetRootOutputs(mmi);
	
	// C API parameter structure (pointer to)
	modelParameters = rtwCAPI_GetModelParameters(mmi);
	
	// Signal memory addresses
	dataAddrMap = rtwCAPI_GetDataAddressMap(mmi);
	
	// Signal dimension structure (pointer to)
	dimensionMap = rtwCAPI_GetDimensionMap(mmi);
	
	// Signal dimension array of value (pointer to)
	dimensionArray = rtwCAPI_GetDimensionArray(mmi);
	
	// Signal type structure (pointer to)
	dataTypeMap = rtwCAPI_GetDataTypeMap(mmi);
	
	// Allocation of info storage structures
	rootInputSignalInfo  = new modelSignalInfoStruct[numRootInputs];
	rootOutputSignalInfo = new modelSignalInfoStruct[numRootOutputs];
	parameterInfo        = new modelParametersInfoStruct[numParameters];
	
	/***********************************************************************//**
	 * Functions to retrieve info on each signal and parameter are called.
	 **************************************************************************/
	
	ok = GetModelSignalData(InputSignals, numRootInputs, modelRootInputs, rootInputSignalInfo);
	
	ok = GetModelSignalData(OutputSignals, numRootOutputs, modelRootOutputs, rootOutputSignalInfo);
	
	ok = GetModelParameterData(numParameters, modelParameters, parameterInfo);
	
	/***********************************************************************//**
	 * Parameter values are eventually read from the configuration file and
	 * their value in the model is updated accordingly. If Simulink(R) model
	 * has a stored value for that parameter, that value is overwritten with
	 * the value found in the configuration file.
	 **************************************************************************/
	
	// This will store some information on the parameter value 
	AnyType arrayDescription;
	
	// Dimensions of the parameter
	uint32 numberOfDimensions;
	uint32 numberOfRows;
	uint32 numberOfCols;
	
	ok = data.MoveRelative("Parameters");
	if(!ok){
		REPORT_ERROR(ErrorManagement::ParametersError,
					 "Cannot find Parameters node in configuration file.");
		return ok;
	}
	
	for (uint32 i = 0; i < numParameters; i++ ) {
		
		// Before we store the parameter, we must know how much memory to allocate
		AnyType arrayDescription = data.GetType(parameterInfo[i].paramName);
		ok = arrayDescription.GetDataPointer() != NULL_PTR(void *);
		if (!ok) {
			
			REPORT_ERROR(ErrorManagement::ParametersError,
						 "Error while reading param. %s from configuration file. GetDataPointer() == NULL_PTR(void *). Was the parameter defined?",
						 parameterInfo[i].paramName);
			return ok;
			
		}
		
		/// Since we are at it, various coherence checks between model and configuration file are performed
		numberOfDimensions = arrayDescription.GetNumberOfDimensions();
		
		ok = (numberOfDimensions <= 2u);	// More than 2 dimensions is not supported
		if (!ok) {
			
			REPORT_ERROR(ErrorManagement::ParametersError,
						 "Param. %s has %i dimension(s). Dimensions must be less or equal to 2",
						 parameterInfo[i].paramName, numberOfDimensions);
			return ok;
			
		}
		
		ok = (numberOfDimensions == parameterInfo[i].modelNumDimensions);
		if (!ok) {
			
			REPORT_ERROR(ErrorManagement::ParametersError,
						 "Param. %s dimension mismatch. Model: %i, configuration file: %i",
						 parameterInfo[i].paramName,
						 parameterInfo[i].modelNumDimensions,
						 numberOfDimensions);
			return ok;
			
		}
		
		numberOfRows = arrayDescription.GetNumberOfElements(1u);
		numberOfCols = arrayDescription.GetNumberOfElements(0u);
		
		ok = (numberOfRows == parameterInfo[i].modelDimensions[0]);
		if (!ok) {
			
			REPORT_ERROR(ErrorManagement::ParametersError,
						 "Param. %s nOfRows mismatch. Model: %i, configuration file: %i",
						 parameterInfo[i].paramName,
						 parameterInfo[i].modelDimensions[0],
						 numberOfRows);
			return ok;
			
		}
		
		ok = (numberOfCols == parameterInfo[i].modelDimensions[1]);
		if (!ok) {
			
			REPORT_ERROR(ErrorManagement::ParametersError,
						 "Param. %s nOfCols mismatch. Model: %i, configuration file: %i",
						 parameterInfo[i].paramName,
						 parameterInfo[i].modelDimensions[1],
						 numberOfCols);
			return ok;
			
		}
		
		// Now parameter values are read from the configuration file.
		// Then, parameters are updated accordingly.
		// (This part does not need to be templated since parameters are read
		// only from config. file and can then be safely assumed to be all float64)
		switch (numberOfDimensions) {
			
			case 0: {
				
				float64 configValue;
				ok = data.Read(parameterInfo[i].paramName, configValue);
				
				if (!ok) {
					
					REPORT_ERROR(ErrorManagement::ParametersError,
								 "Error while reading param. %s from configuration file. Was it defined?",
								 parameterInfo[i].paramName);
					return ok;
					
				}
				
				*((float64*)parameterInfo[i].paramAddr) = configValue;
				
				break;
				
			}
			
			case 1: {
				
				Vector<float64> configVector(numberOfRows*numberOfCols);
				ok = data.Read(parameterInfo[i].paramName, configVector);
				
				if (!ok) {
					
					REPORT_ERROR(ErrorManagement::ParametersError,
								 "Error while reading param. %s from configuration file. Was it defined?",
								 parameterInfo[i].paramName);
					return ok;
					
				}
				
				for (uint32 j = 0; j < numberOfRows*numberOfCols; j++) {
					
					*((float64*)parameterInfo[i].paramAddr + j) = configVector[j];
					
					//std::cout << configVector[j] << std::endl;
					
				}
				
				break;
				
			}
			
			case 2: {
				
				Matrix<float64> configMatrix(numberOfRows, numberOfCols);
				ok = data.Read(parameterInfo[i].paramName, configMatrix);
				
				if (!ok) {
					
					REPORT_ERROR(ErrorManagement::ParametersError,
								 "Error while reading param. %s from configuration file. Was it defined?",
								 parameterInfo[i].paramName);
					return ok;
				}
				
				for (uint32 j = 0; j < numberOfRows; j++) {
					
					for (uint32 k = 0; k < numberOfCols; k++) {
						
						// Here indices are inverted because in the model matrices are stored in col-major format
						*((float64*)parameterInfo[i].paramAddr + numberOfRows*k + j) = configMatrix[j][k];
						
						//std::cout << configMatrix[j][k] << std::endl;
						
					}
					
				}
				
				break;
				
			}
			
			default:
				REPORT_ERROR(ErrorManagement::ParametersError,
							 "Param. %s has %i dimension(s). Dimensions must be less or equal to 2",
							 parameterInfo[i].paramName, numberOfDimensions);
				return ok;
				break;
			
		}
			
	}
	
	ok = data.MoveToAncestor(1);
	if(!ok){
		REPORT_ERROR(ErrorManagement::InitialisationError,
					 "Failed MoveToAncestor() from %s", data.GetName());
		return ok;
	}
	
	return ok;
}


bool SimulinkInterfaceGAM::Setup() {

	// Debug
	REPORT_ERROR(ErrorManagement::Debug, "SETUP");
	
	/***********************************************************************//**
	 * @pre Number of inputs and outputs declared in the configuration file
	 * must be the same as in the model.
	 **************************************************************************/
	bool ok = (GetNumberOfInputSignals() == numRootInputs);
	if (!ok) {
		
		REPORT_ERROR(ErrorManagement::ParametersError,
					 "Number of inputs mismatch. Model: (%i), GAM: (%i)",
					 numRootInputs, GetNumberOfInputSignals());
		return ok;
		
	}
	
	ok = (GetNumberOfOutputSignals() == numRootOutputs);
	if (!ok) {
		
		REPORT_ERROR(ErrorManagement::ParametersError,
					 "Number of outputs mismatch. Model: (%i), GAM: (%i)",
					 numRootOutputs, GetNumberOfOutputSignals());
		return ok;
		
	}
	
	/***********************************************************************//**
	 * @pre Each input and output must be of the same type and have the same
	 * dimensions as in the model.
	 **************************************************************************/
	
	// At the moment, we assume that nOfSamples in MARTe2 sense is 1
	TypeDescriptor gamType, modelType;
	uint32 gamNumOfDims, modelNumOfDims;
	uint32 gamNumOfElems, modelNumOfElems;
	
	for (uint32 i = 0; i < numRootInputs; i++) {
		
		// Type check
		gamType = GetSignalType(InputSignals, i);
		modelType = rootInputSignalInfo[i].MARTe2DataType;
		
		if (gamType != modelType) {
			
			REPORT_ERROR(ErrorManagement::ParametersError,
						 "Input no. (%i): data type mismatch. Model: (%s), GAM: (%s)",
						 i+1, TypeDescriptor::GetTypeNameFromTypeDescriptor(modelType),
						 TypeDescriptor::GetTypeNameFromTypeDescriptor(gamType));
			return false;
			
		}
		
		// Dimension check
		GetSignalNumberOfDimensions(InputSignals, i, gamNumOfDims);
		modelNumOfDims = rootInputSignalInfo[i].modelNumDimensions;
		
		if (gamNumOfDims != modelNumOfDims) {
			
			REPORT_ERROR(ErrorManagement::ParametersError,
						 "Input no. (%i): dimension mismatch. Model: (%i), GAM: (%i)",
						 i+1, modelNumOfDims, gamNumOfDims);
			//return false;
			
		}
		
		// Number of element check
		GetSignalNumberOfElements(InputSignals, i, gamNumOfElems);
		modelNumOfElems = rootInputSignalInfo[i].modelNumElements;
		
		if (gamNumOfElems != modelNumOfElems) {
			
			REPORT_ERROR(ErrorManagement::ParametersError,
						 "Input no. (%i): number of elements mismatch. Model: (%i), GAM: (%i)",
						 i+1, modelNumOfElems, gamNumOfElems);
			return false;
			
		}
		
	}
	
	for (uint32 i = 0; i < numRootOutputs; i++) {
		
		// Type check
		gamType = GetSignalType(OutputSignals, i);
		modelType = rootOutputSignalInfo[i].MARTe2DataType;
		
		if (gamType != modelType) {
			
			REPORT_ERROR(ErrorManagement::ParametersError,
						 "Output no. (%i): data type mismatch. Model: (%s), GAM: (%s)",
						 i+1, TypeDescriptor::GetTypeNameFromTypeDescriptor(modelType),
						 TypeDescriptor::GetTypeNameFromTypeDescriptor(gamType));
			return false;
			
		}
		
		// Dimension check
		GetSignalNumberOfDimensions(OutputSignals, i, gamNumOfDims);
		modelNumOfDims = rootOutputSignalInfo[i].modelNumDimensions;
		
		if (gamNumOfDims != modelNumOfDims) {
			
			REPORT_ERROR(ErrorManagement::ParametersError,
						 "Output no. (%i): dimension mismatch. Model: (%i), GAM: (%i)",
						 i+1, modelNumOfDims, gamNumOfDims);
			return false;
			
		}
		
		// Number of element check
		GetSignalNumberOfElements(OutputSignals, i, gamNumOfElems);
		modelNumOfElems = rootOutputSignalInfo[i].modelNumElements;
		
		if (gamNumOfElems != modelNumOfElems) {
			
			REPORT_ERROR(ErrorManagement::ParametersError,
						 "Output no. (%i): number of elements mismatch. Model: (%i), GAM: (%i)",
						 i+1, modelNumOfElems, gamNumOfElems);
			return false;
			
		}
		
	}
	
    return ok;
}


bool SimulinkInterfaceGAM::Execute() {
	
	// Debug
	if (isFirstRun) {
		
		REPORT_ERROR(ErrorManagement::Debug, "RUNNING");
		
		isFirstRun = false;
		
	}
	
	/***********************************************************************//**
	 * Inputs from GAM memory are copied into model memory to be elaborated,
	 * with row-major > col-major conversion.
	 **************************************************************************/
	
	// Cycle over all signals
	for (uint32 i = 0; i < numRootInputs; i++) {
		
		// Raw data are copied from GAM memory to model memory for elaboration
		ExchangeData(InputSignals, i, rootInputSignalInfo[i].MARTe2DataType);
		
	}
	
	/***********************************************************************//**
	 * Data are then elaborated by calling the step function of the model.
	 **************************************************************************/
	
	modelFuncPtr_Step();
	
	/***********************************************************************//**
	 * Data are eventually copied from model memory to GAM memory,
	 * with col-major > row-major conversion.
	 **************************************************************************/
	
	// Cycle over all signals
	for (uint32 i = 0; i < numRootOutputs; i++) {
		
		// Elaborated data are copied back from model memory to GAM memory
		ExchangeData(OutputSignals, i, rootOutputSignalInfo[i].MARTe2DataType);
		
	}
	
	//--------------------------------------------------------------------------
	
	// Output for debugging
	//PrintSignalForDebugging<float64>(0);
	//PrintSignalForDebugging<float64>(1);
	//PrintSignalForDebugging<float64>(2);
	//PrintSignalForDebugging<int32>(3);
	
	REPORT_ERROR(ErrorManagement::Debug, "-------------------------------------------------------------------");
	
	// Sleeps for debugging
	if (sleepTime!=0) {
		
		MARTe::Sleep::Sec(sleepTime);
		
	}
	
	return true;
}

CLASS_REGISTER(SimulinkInterfaceGAM, "1.0")

} /* namespace MARTe */

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/

// Memory transfer from model to GAM and viceversa
void MARTe::SimulinkInterfaceGAM::ExchangeData(const SignalDirection direction,
											   const uint32 signalIndex,
											   const TypeDescriptor MARTe2SignalDataType
											   ) {
	
	/***********************************************************************//**
	 * Template functions are a compile-time feature,
	 * so a specific function call for every type is required.
	 **************************************************************************/
	if (MARTe2SignalDataType==UnsignedInteger8Bit) {
		
		CopyRealtimeSignalDataFromAndToModel<uint8>(direction, signalIndex);
		
	} else if (MARTe2SignalDataType==UnsignedInteger16Bit) {
		
		CopyRealtimeSignalDataFromAndToModel<uint16>(direction, signalIndex);
		
	} else if (MARTe2SignalDataType==UnsignedInteger32Bit) {
		
		CopyRealtimeSignalDataFromAndToModel<uint32>(direction, signalIndex);
		
	} else if (MARTe2SignalDataType==UnsignedInteger64Bit) {
		
		CopyRealtimeSignalDataFromAndToModel<uint64>(direction, signalIndex);
		
	} else if (MARTe2SignalDataType==SignedInteger8Bit) {
		
		CopyRealtimeSignalDataFromAndToModel<int8>(direction, signalIndex);
		
	} else if (MARTe2SignalDataType==SignedInteger16Bit) {
		
		CopyRealtimeSignalDataFromAndToModel<int16>(direction, signalIndex);
		
	} else if (MARTe2SignalDataType==SignedInteger32Bit) {
		
		CopyRealtimeSignalDataFromAndToModel<int32>(direction, signalIndex);
		
	} else if (MARTe2SignalDataType==SignedInteger64Bit) {
		
		CopyRealtimeSignalDataFromAndToModel<int64>(direction, signalIndex);
		
	} else if (MARTe2SignalDataType==Float32Bit) {
		
		CopyRealtimeSignalDataFromAndToModel<float32>(direction, signalIndex);
		
	} else if (MARTe2SignalDataType==Float64Bit) {
		
		CopyRealtimeSignalDataFromAndToModel<float64>(direction, signalIndex);
		
	} else if (MARTe2SignalDataType==Character8Bit) {
		
		CopyRealtimeSignalDataFromAndToModel<char8>(direction, signalIndex);
		
	} else {
		
		REPORT_ERROR(ErrorManagement::Exception,
					 "Error while copying data between model and GAM. Signal no. %i: unsupported type.",
					 signalIndex);
		
	}
	
}

// Actual template function that performs the memory copy from model to GAM and viceversa (with row-major/col-major conversion)
template <typename T>
void MARTe::SimulinkInterfaceGAM::CopyRealtimeSignalDataFromAndToModel(const SignalDirection direction,
																	   const uint32 signalIndex
																	   ) {
	
	T* signalAddrInModel;
	T* signalAddrInGAM;
	
	uint32 numOfRows, numOfCols;
	
	switch (direction) {
		
		case InputSignals:
			
			signalAddrInModel = (T*)rootInputSignalInfo[signalIndex].signAddr;
			signalAddrInGAM   = (T*)GetInputSignalMemory(signalIndex);
			
			numOfRows = rootInputSignalInfo[signalIndex].modelDimensions[0];
			numOfCols = rootInputSignalInfo[signalIndex].modelDimensions[1];
			
			// Row-major to col-major conversion is required since the model works in col-major format, while MARTe2 works in row-major
			for (uint32 j = 0; j < rootInputSignalInfo[signalIndex].modelDimensions[0]; j++) {
				
				for (uint32 k = 0; k < rootInputSignalInfo[signalIndex].modelDimensions[1]; k++) {
					
					// Notice that indices are reversed to perform the conversion
					*(signalAddrInModel + numOfRows*k + j) = *(signalAddrInGAM + numOfCols*j + k);
					
				}
				
			}
			break;
			
			
		case OutputSignals:
			
			signalAddrInModel = (T*)rootOutputSignalInfo[signalIndex].signAddr;
			signalAddrInGAM   = (T*)GetOutputSignalMemory(signalIndex);
			
			numOfRows = rootOutputSignalInfo[signalIndex].modelDimensions[0];
			numOfCols = rootOutputSignalInfo[signalIndex].modelDimensions[1];
			
			// Col-major to row-major conversion is required since the model works in col-major format, while MARTe2 works in row-major
			for (uint32 j = 0; j < rootOutputSignalInfo[signalIndex].modelDimensions[0]; j++) {
				
				for (uint32 k = 0; k < rootOutputSignalInfo[signalIndex].modelDimensions[1]; k++) {
					
					// Notice that indices are reversed to perform the conversion
					*(signalAddrInGAM + numOfCols*j + k) = *(signalAddrInModel + numOfRows*k + j);
					
				}
				
			}
			break;
			
		default:
			
			REPORT_ERROR(ErrorManagement::Exception,
						 "Error while copying data between model and GAM. Signal no. %i: direction not specified.",
						 signalIndex);
			
	}
	
}

// This method extracts all signal data from the model
bool MARTe::SimulinkInterfaceGAM::GetModelSignalData(const SignalDirection direction,
													 const uint32 numOfSignals,
													 const rtwCAPI_Signals* modelRootSignals,
													 modelSignalInfoStruct* rootSignalInfo
													 ) {
	
	bool ok = true;
	
	const char8* signDir;
	
	if (direction==InputSignals) {
		
		signDir = "Input ";
		
	} else if (direction==OutputSignals) {
		
		signDir = "Output";
		
	}
	
	for (uint32 i = 0; i < numOfSignals; i++) {
		
		/*******************************************************************//**
		 * Indices of where data are stored are retrieved using Simulink® C APIs.
		 **********************************************************************/
		rootSignalInfo[i].addrIdx  = rtwCAPI_GetSignalAddrIdx(modelRootSignals, i);			// Address index
		rootSignalInfo[i].dimIdx   = rtwCAPI_GetSignalDimensionIdx(modelRootSignals, i);	// Dimension index
		rootSignalInfo[i].dTypeIdx = rtwCAPI_GetSignalDataTypeIdx(modelRootSignals, i);		// Type index
		
		/*******************************************************************//**
		 * Using these indices, data are retrieved from
		 * the respective structure and stored in a modelSignalInfoStruct:
		 **********************************************************************/
		
		/// 1. memory addresses are retrieved from dataAddressMap
		rootSignalInfo[i].signAddr = rtwCAPI_GetDataAddress(dataAddrMap, rootSignalInfo[i].addrIdx);
		
		/// 2. informations on number of elements are retrieved from dimensionMap
		rootSignalInfo[i].dimArrayIdx = rtwCAPI_GetDimArrayIndex(dimensionMap, rootSignalInfo[i].dimIdx);	// Starting position in the dimensionArray
		rootSignalInfo[i].numDims     = rtwCAPI_GetNumDims(dimensionMap, rootSignalInfo[i].dimIdx);			// Number of elements in the dimensionArray referring to this signal
		
		if (rootSignalInfo[i].numDims > 2) {
			
			ok = false;
			REPORT_ERROR(ErrorManagement::ParametersError,
						 "%s no. %i of the model has more than 2 dimensions. Unsupported.",
						 signDir, i);
			
		}
		
		rootSignalInfo[i].modelNumElements = 1;
		
		for (uint32 k = 0; k < rootSignalInfo[i].numDims; k++) {	// Reading all elements in the dimensionArray referring to this signal
			
			rootSignalInfo[i].modelDimensions[k] = dimensionArray[rootSignalInfo[i].dimArrayIdx + k];		// Apparently there is no built-in macro to access this
			rootSignalInfo[i].modelNumElements = rootSignalInfo[i].modelNumElements * rootSignalInfo[i].modelDimensions[k];
			
		}
		
		/// 3. informations on number of dimensions are retrieved from dimensionMap
		rootSignalInfo[i].slDataOrientation = rtwCAPI_GetOrientation(dimensionMap, rootSignalInfo[i].dimIdx);
		
		switch (rootSignalInfo[i].slDataOrientation) {
			
			case (rtwCAPI_SCALAR): {
				
				rootSignalInfo[i].modelNumDimensions = 0;
				break;
				
			}
			
			case (rtwCAPI_VECTOR): {
				
				rootSignalInfo[i].modelNumDimensions = 1;
				break;
				
			}
			
			case (rtwCAPI_MATRIX_COL_MAJOR): {
				
				rootSignalInfo[i].modelNumDimensions = 2;
				break;
				
			}
			
			default:
				ok = false;
				REPORT_ERROR(ErrorManagement::ParametersError,
							 "%s no. (%i): unsupported data orientation",
							 signDir, i+1);
				break;
			
		}
		
		/// 4. type is retrieved from dataTypeMap and stored both as a string and as MARTe2 data type
		rootSignalInfo[i].slDataType = rtwCAPI_GetDataTypeSLId(dataTypeMap, rootSignalInfo[i].dTypeIdx);
		rootSignalInfo[i].cDataType = rtwCAPI_GetDataTypeCName(dataTypeMap, rootSignalInfo[i].dTypeIdx);
		
		switch (rootSignalInfo[i].slDataType) {
			
			case SS_DOUBLE: {
				rootSignalInfo[i].MARTe2DataType = Float64Bit;
				break;
			}
			
			case SS_SINGLE: {
				rootSignalInfo[i].MARTe2DataType = Float32Bit;
				break;
			}
			
			case SS_INT8: {
				rootSignalInfo[i].MARTe2DataType = SignedInteger8Bit;
				break;
			}
			
			case SS_UINT8: {
				rootSignalInfo[i].MARTe2DataType = UnsignedInteger8Bit;
				break;
			}
			
			case SS_INT16: {
				rootSignalInfo[i].MARTe2DataType = SignedInteger16Bit;
				break;
			}
			
			case SS_UINT16: {
				rootSignalInfo[i].MARTe2DataType = UnsignedInteger16Bit;
				break;
			}
			
			case SS_INT32: {
				rootSignalInfo[i].MARTe2DataType = SignedInteger32Bit;
				break;
			}
			
			case SS_UINT32: {
				rootSignalInfo[i].MARTe2DataType = UnsignedInteger32Bit;
				break;
			}
			
			case SS_BOOLEAN: {
				rootSignalInfo[i].MARTe2DataType = UnsignedInteger8Bit; // Is boolean supported by TypeDescriptor?
				break;
			}
			default:
				ok = false;
				REPORT_ERROR(ErrorManagement::ParametersError, 
							 "%s no. (%i): unsupported model data type", 
							 signDir, i+1);
				break;
			
		}
		
		/// 5. information retrieved from the model are printed on screen
		REPORT_ERROR(ErrorManagement::Information,
					 "%s no. %i, ID no. %i, dims. %i, rows %i, cols %i, elems. %i, cType %s, MARTe2 type %s",
					 signDir, i, rootSignalInfo[i].addrIdx, rootSignalInfo[i].modelNumDimensions,
					 rootSignalInfo[i].modelDimensions[0], rootSignalInfo[i].modelDimensions[1],
					 rootSignalInfo[i].modelNumElements, rootSignalInfo[i].cDataType,
					 TypeDescriptor::GetTypeNameFromTypeDescriptor(rootSignalInfo[i].MARTe2DataType)
					 );
		
	}
	
	return ok;
	
}

// This method extracts all parameter data from the model
bool MARTe::SimulinkInterfaceGAM::GetModelParameterData(const uint32 numOfParams,
														const rtwCAPI_ModelParameters* modelParameters, 
														modelParametersInfoStruct* parameterInfo
														) {
	
	bool ok = true;
	
	for (uint32 i = 0; i < numOfParams; i++) {
		
		/*******************************************************************//**
		* Parameter name is retrieved.
		**********************************************************************/
		
		parameterInfo[i].paramName = rtwCAPI_GetModelParameterName(modelParameters, i);
		
		/*******************************************************************//**
		 * Indices of where data are stored are retrieved using Simulink® C APIs.
		 **********************************************************************/
		 
		parameterInfo[i].addrIdx  = rtwCAPI_GetModelParameterAddrIdx(modelParameters, i);		// Address index
		parameterInfo[i].dimIdx   = rtwCAPI_GetModelParameterDimensionIdx(modelParameters, i);	// Dimension index
		parameterInfo[i].dTypeIdx = rtwCAPI_GetModelParameterDataTypeIdx(modelParameters, i);	// Type index
		
		/*******************************************************************//**
		 * Using these indices, data are retrieved from
		 * the respective structure and stored in a modelParametersInfoStruct:
		 **********************************************************************/
		
		/// 1. memory addresses are retrieved from dataAddressMap
		parameterInfo[i].paramAddr = rtwCAPI_GetDataAddress(dataAddrMap, parameterInfo[i].addrIdx);
		
		/// 2. informations on number of elements are retrieved from dimensionMap
		parameterInfo[i].dimArrayIdx = rtwCAPI_GetDimArrayIndex(dimensionMap, parameterInfo[i].dimIdx);	// Starting position in the dimensionArray
		parameterInfo[i].numDims     = rtwCAPI_GetNumDims(dimensionMap, parameterInfo[i].dimIdx);		// Number of elements in the dimensionArray referring to this signal
		
		if (parameterInfo[i].numDims > 2) {
			
			ok = false;
			REPORT_ERROR(ErrorManagement::ParametersError,
						 "Parameter no. %i (%s) of the model has more than 2 dimensions. Unsupported.",
						 i, parameterInfo[i].paramName);
			
		}
		
		parameterInfo[i].modelNumElements = 1;
		
		for (uint32 k = 0; k < parameterInfo[i].numDims; k++) {	// Reading all elements in the dimensionArray referring to this parameter
			
			parameterInfo[i].modelDimensions[k] = dimensionArray[parameterInfo[i].dimArrayIdx + k];		// Apparently there is no built-in macro to access this
			parameterInfo[i].modelNumElements   = parameterInfo[i].modelNumElements * parameterInfo[i].modelDimensions[k];
			
		}
		
		/// 3. informations on number of dimensions are retrieved from dimensionMap
		parameterInfo[i].slDataOrientation = rtwCAPI_GetOrientation(dimensionMap, parameterInfo[i].dimIdx);
		
		switch (parameterInfo[i].slDataOrientation) {
			
			case (rtwCAPI_SCALAR): {
				
				parameterInfo[i].modelNumDimensions = 0;
				break;
				
			}
			
			case (rtwCAPI_VECTOR): {
				
				// MARTe2 treats column vectors as matrices, so a vector with more than one row must get NumDimensions = 2
				if (parameterInfo[i].modelDimensions[0] > 1) {
					
					parameterInfo[i].modelNumDimensions = 2;
					
				} else {
					
					parameterInfo[i].modelNumDimensions = 1;
				
				}
				
				break;
				
			}
			
			case (rtwCAPI_MATRIX_COL_MAJOR): {
				
				parameterInfo[i].modelNumDimensions = 2;
				break;
				
			}
			
			default:
				
				ok = false;
				REPORT_ERROR(ErrorManagement::ParametersError, "Parameter no. %i (%s): unsupported data orientation",
							 i, parameterInfo[i].paramName);
				break;
			
		}
		
		/// 4. type is retrieved and stored as a string and as MARTe2 data type
		parameterInfo[i].slDataType = rtwCAPI_GetDataTypeSLId(dataTypeMap, parameterInfo[i].dTypeIdx);
		parameterInfo[i].cDataType  = rtwCAPI_GetDataTypeCName(dataTypeMap, parameterInfo[i].dTypeIdx);
		
		switch (parameterInfo[i].slDataType) {
			
			case SS_DOUBLE: {
				parameterInfo[i].MARTe2DataType = Float64Bit;
				break;
			}
			
			case SS_SINGLE: {
				parameterInfo[i].MARTe2DataType = Float32Bit;
				break;
			}
			
			case SS_INT8: {
				parameterInfo[i].MARTe2DataType = SignedInteger8Bit;
				break;
			}
			
			case SS_UINT8: {
				parameterInfo[i].MARTe2DataType = UnsignedInteger8Bit;
				break;
			}
			
			case SS_INT16: {
				parameterInfo[i].MARTe2DataType = SignedInteger16Bit;
				break;
			}
			
			case SS_UINT16: {
				parameterInfo[i].MARTe2DataType = UnsignedInteger16Bit;
				break;
			}
			
			case SS_INT32: {
				parameterInfo[i].MARTe2DataType = SignedInteger32Bit;
				break;
			}
			
			case SS_UINT32: {
				parameterInfo[i].MARTe2DataType = UnsignedInteger32Bit;
				break;
			}
			
			case SS_BOOLEAN: {
				parameterInfo[i].MARTe2DataType = UnsignedInteger8Bit; // Is boolean supported by TypeDescriptor?
				break;
			}
			
			default:
				
				ok = false;
				REPORT_ERROR(ErrorManagement::ParametersError,
							 "Parameter no. %i (%s): unsupported model data type",
							 i, parameterInfo[i].paramName);
				break;
			
		}
		
		/// 5. information retrieved from the model are printed on screen
		REPORT_ERROR(ErrorManagement::Information, "Param  no. %i, ID no. %i, dims. %i, rows %i, cols %i, elems. %i, cType %s, MARTe2 type %s (%s)",
					 i, parameterInfo[i].addrIdx, parameterInfo[i].modelNumDimensions, parameterInfo[i].modelDimensions[0],
					 parameterInfo[i].modelDimensions[1], parameterInfo[i].modelNumElements, parameterInfo[i].cDataType,
					 TypeDescriptor::GetTypeNameFromTypeDescriptor(parameterInfo[i].MARTe2DataType), parameterInfo[i].paramName);
		
	}
	
	return ok;
	
}

// Method to output signal values (only for debugging purpose)
template <typename T>
void MARTe::SimulinkInterfaceGAM::PrintSignalForDebugging(uint32 signalIndex) {
	
	bool ok = false;
	
	uint32 elem;
	
	REPORT_ERROR(ErrorManagement::Information, "Signal no. %i", signalIndex);
	
	/// Based on how many dimensions the signal has, the print function behaves differently.
	switch (rootOutputSignalInfo[signalIndex].modelNumDimensions) {
		
		/// In each case a consistency control between model and GAM signal values is implemented.
		
		case 0:
			
			// Consistency check between signal value in model and in GAM
			ok = ( *((T*)rootOutputSignalInfo[signalIndex].signAddr) == *((T*)GetOutputSignalMemory(signalIndex)) );
			
			if (ok) {
				
				std::cout << *((T*)GetOutputSignalMemory(signalIndex)) << std::endl;
				
			} else {
				
				REPORT_ERROR(ErrorManagement::FatalError,
							 "Inconsistency between model and GAM signal detected for signal %i",
							 signalIndex);
				
			}
			
			break;
			
		case 1:
			
			for (uint32 i = 0; i < rootOutputSignalInfo[signalIndex].modelNumElements; i++) {
				
				// Consistency check between signal value in model and in GAM
				ok = ( *((T*)rootOutputSignalInfo[signalIndex].signAddr + i) == *((T*)GetOutputSignalMemory(signalIndex) + i) );
				
				if (ok) {
					
					std::cout << *((T*)GetOutputSignalMemory(signalIndex) + i) << std::endl;
					
				} else {
					
					REPORT_ERROR(ErrorManagement::FatalError,
								"Inconsistency between model and GAM signal detected for signal %i",
								signalIndex);
					
				}
				
			}
			break;
			
		case 2:
			elem = 0;
			
			for (uint32 i = 0; i < rootOutputSignalInfo[signalIndex].modelDimensions[0]; i++) {
				
				for (uint32 j = 0; j < rootOutputSignalInfo[signalIndex].modelDimensions[1]; j++) {
					
					std::cout << *((T*)GetOutputSignalMemory(signalIndex) + elem) << " ";
					
					elem++;
					
				}
				
				std::cout << "\n";
				
			}
			break;
			
		default:
			REPORT_ERROR(ErrorManagement::Exception,
						 "Error in PrintSignalForDebugging(). Signal no. %i: unsupported number of dimensions.",
						 signalIndex);
			break;
		
	}
	
}