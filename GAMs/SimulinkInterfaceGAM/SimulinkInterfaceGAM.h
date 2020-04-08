/**
 * @file SimulinkInterfaceGAM.h
 * @brief Header file for class SimulinkInterfaceGAM
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

 * @details This header file contains the declaration of the class SimulinkInterfaceGAM
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
 */

#ifndef SIMULINKINTERFACEGAM_H_
#define SIMULINKINTERFACEGAM_H_

/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/

#include "GAM.h"
#include "StructuredDataI.h"
#include "MessageI.h"
#include "LoadableLibrary.h"

/*---------------------------------------------------------------------------*/
/*                        Simulink® model includes                           */
/*---------------------------------------------------------------------------*/

#include "rtw_modelmap.h"
#include "builtin_typeid_types.h"

/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/

namespace MARTe {
/**
 * @brief GAM which wraps Simulink®-generated code.
 * @details This GAM wraps Simulink®-generated code so that it can be run as a MARTe2 GAM.
 * 
 * Wrapping happens at run time by providing to the GAM a valid shared object
 * file (.so) generated from Simulink®. The configuration file shall have the
 * following syntax:
 * 
 * <pre>
 * +MyGAM = {
 *     Class      = SimulinkInterfaceGAM
 *     FileName   = "libraryFileName"       // Unnecessary unless it differs from ModelName
 *     ModelName  = "nameOfTheLibrary"
 *     Parameters = {
 *         ...
 *     }
 *     InputSignals = {
 *         ...
 *     {
 *     OutputSignals = {
 *         ...
 *     {
 * }
 * </pre>
 * 
 * The .so file containing the library must be placed in the same directory as
 * the GAM binaries.
 * 
 * @pre 1. Number of inputs and outputs of the configuration file for this GAM must match
 * exactely the number of inputs and outputs of the Simulink® model from which the code
 * was generated. Not respetting this precondition returns an error.
 * @pre 2. Values of parameters declared in the model must be set in the configuration file.
 * Not setting the value for all paramters returns an error.
 * 
 * @todo Implement structure parameters (that is, parameters used in the model that
 * are in structure form. The slDataType of such parameters is SS_STRUCT and is
 * currently unsupported. From the MARTe2 point of view, such parameters should be
 * given as nested values in the configuration file.
 */
class SimulinkInterfaceGAM : public GAM {
public:
	
	CLASS_REGISTER_DECLARATION()
	
	/**
	 * @brief Default costructor.
	 */
	SimulinkInterfaceGAM();
	
	/**
	 * @brief Default destructor.
	 */
	virtual ~SimulinkInterfaceGAM();
	
	/**
	 * @brief Initializes and retrieves informations from Simulink® model. Also sets parameter values. 
	 * @details
	 * @param[in] data the GAM configuration specified in the configuration file.
	 * @return true on succeed
	 * @pre 
	 */
	virtual bool Initialise(StructuredDataI & data);
	
	/**
	 * @brief Setup inputs, outputs and parameters of the GAM in accordance to the model. Also verifies consistency with what was declared in the configuration file.
	 * @details
	 * @return true on succeed
	 * @pre Initialise()
	 */
	virtual bool Setup();
	
	/**
	 * @brief Runs the Simulink®-generated model code in MARTe2 environment. 
	 * @details
	 * @return true
	 * @pre
	 *	Initialise() &&
	 *	Setup()
	 */
	virtual bool Execute();
	
private:
	
	/**
	 * @name Debugging
	 */
	//@{
	bool isFirstRun;	//!< Checks if it's the first run. @warning For debugging purpose only.
	float32 sleepTime;	//!< To put the execute method to sleep. Set to 0 to avoid sleep. @warning For debugging purpose only.
	//@}
	
	/*
	 * Declaration of C API-retrievable variables
	 */
	
	/**
	 * @name General variables
	 * Number of inputs, outputs and parameters in the Simulink® model are stored here.
	 */
	//@{
	StreamString modelName;		//!< Name of the model as declared in configuration file.
	StreamString fileName;		//!< Name of the shared object file as declared in configuration file.
	
	uint32 numRootInputs;	//!< Number of Simulink® model root input ports.
	uint32 numRootOutputs;	//!< Number of Simulink® model root output ports.
	uint32 numParameters;	//!< Number of Simulink® model parameters.
	//@}
	
	/**
	 * @name Model library handling
	 */
	//@{
	LoadableLibrary modelLibrary;	//!< Instance containing the model library.
	
	void (*modelFuncPtr_Init)();	//!< Pointer to model initialization function.
	void (*modelFuncPtr_Step)();	//!< Pointer to model step function.
	void (*modelFuncPtr_Term)();	//!< Pointer to model termination function.
	
	void* (*getMmiPtr)();			//!< Pointer to the model mapping info structure.
	//@}
	
	/**
	 * @name Pointers to general C APIs
	 * @brief Pointers to general C API structures (where data of the model are stored).
	 * @details These structures/arrays store general informations (data types, dimensions, addresses).
	 */
	//@{
	void**                      dataAddrMap;		//!< Array of pointers for dataAddrMap. @details An array for data addresses is not provided by Simulink® C APIs, since it's just and array of pointers to void.
	const rtwCAPI_DataTypeMap*  dataTypeMap;		//!< Pointer to dataTypeMap.            @details dataTypeMap is a structure provided by Simulink® C APIs.
	const rtwCAPI_DimensionMap* dimensionMap;		//!< Pointer to dimensionMap.           @details dimensionMap is a structure provided by Simulink® C APIs.
	const uint32*               dimensionArray;		//!< Pointer to dimensionArray.         @details dimensionArray is a structure provided by Simulink® C APIs.
	//@}
	
	/**
	 * @name Pointers to specific C APIs
	 * @brief Pointers to model-specific C API structures (where data of the model are stored).
	 * @details These structures/arrays contain indices associating each model-specific entity (signal, parameter...) with its data type, dimension and address stored in general structures.
	 */
	//@{
	const rtwCAPI_Signals*         modelRootInputs;			//!< Pointer to a data structure with input data.     @details rtwCAPI_Signals is a structure storing input data provided by Simulink® C APIs. 
	const rtwCAPI_Signals*         modelRootOutputs;		//!< Pointer to a data structure with output data.    @details rtwCAPI_Signals is a structure storing output data provided by Simulink® C APIs. 
	const rtwCAPI_ModelParameters* modelParameters;			//!< Pointer to a data structure with parameter data. @details rtwCAPI_ModelParameters is a structure storing parameter data provided by Simulink® C APIs. 
	//@}
	
	/**
	 * @brief Structure to store informations on Simulink® model signals retrieved using provided C APIs.
	 * @details Stores: indices of where data of the signal are stored in the respective structure,
	 * memory address of the signal, dimensions of the signal, name and type of the signal.
	 * Sizes (32, 16...) of variables are derived from C API dimensions.
	 * @warning For internal use only.
	 */
	struct modelSignalInfoStruct {
		
		uint32 addrIdx;				//!< Index of where addresses are stored in dataAddrMap.
		uint16 dimIdx;				//!< Index of where dimensions are stored in dimensionMap.
		uint32 dimArrayIdx;			//!< Starting index of where dimensions are stored in dimensionArray.
		uint16 dTypeIdx;			//!< Index of where data type informations are stored in dataTypeMap.
		uint8  slDataType;			//!< Enum of Simulink® internal data type classification.
		uint8  slDataOrientation;	//!< Enum of Simulink® internal data orientation (scalar, vector, col-major matrix...).
		
		void* signAddr;				//!< Memory address of the signal. Simulink® stores it as a pointer to void.
		
		uint32 numDims;				//!< Number of elements in the dimensionArray referring to this signal.
		uint32 modelNumElements;	//!< Number of elements of the signal.
		uint32 modelNumDimensions;	//!< Number of dimensions (in MARTe2 sense). 0 if scalar, 1 if vector, 2 if matrix
		uint32 modelDimensions[2];	//!< Dimensions of the signal. @details [0] =  number of rows, [1] = number of columns. 1x1 if scalar, 1xN if vector, NxM if matrix
		
		const char8*   cDataType;		//!< Data type name in C language
		TypeDescriptor MARTe2DataType;	//!< Data type in MARTe2 syntax
		
	};
	
	/**
	 * @brief Structure to store informations on Simulink® model parameters retrieved using provided C APIs.
	 * @details Stores: indices of where data about parameters are stored in the respective structure,
	 * memory address of the parameter, dimensions of the parameter, name and type of the parameter.
	 * Sizes (32, 16...) of variables are derived from C API dimensions.
	 * @warning For internal use only.
	 */
	struct modelParametersInfoStruct {
		
		uint32 addrIdx;				//!< Index of where addresses are stored in dataAddrMap.
		uint16 dimIdx;				//!< Index of where dimensions are stored in dimensionMap.
		uint32 dimArrayIdx;			//!< Starting index of where dimensions are stored in dimensionArray.
		uint16 dTypeIdx;			//!< Index of where data type informations are stored in dataTypeMap.
		uint8 slDataType;			//!< Enum of Simulink® internal data type classification.
		uint8 slDataOrientation;	//!< Enum of Simulink® internal data orientation (scalar, vector, col-major matrix...).
		
		void* paramAddr;			//!< Memory address of the parameter. Simulink® stores it as a pointer to void.
		
		uint32 numDims;				//!< Number of elements in the dimensionArray referring to this parameter.
		uint32 modelNumElements;	//!< Number of elements of the parameter.
		uint32 modelNumDimensions;	//!< Number of dimensions (in MARTe2 sense). 0 if scalar, 1 if vector, 2 if matrix.
		uint32 modelDimensions[2];	//!< Dimensions of the signal. @details modelDimensions[0] =  number of rows, modelDimensions[1] = number of columns. @details 1×1 if scalar, 1×N if vector, N×M if matrix.
		
		const char8* cDataType;			//!< Data type name in C language.
		TypeDescriptor MARTe2DataType;	//!< Data type in MARTe2 syntax
		
		const char8* paramName;			//!< Name of the parameter from Simulink® model.
		
	};
	
	/**
	 * @name GAM data storage structs
	 * Declarations of structures to store data retrieved from Simulink® model by using C APIs.
	 */
	//@{
	modelSignalInfoStruct*     rootInputSignalInfo;		//!< Structure to store input data.
	modelSignalInfoStruct*     rootOutputSignalInfo;	//!< Structure to store output data.
	modelParametersInfoStruct* parameterInfo;			//!< Structure to store parameter data.
	//@}
	
	/**
	 * @brief Signal info get method.
	 * @details This method extracts all signal data from the model: it uses 
	 * C APIs provided by Simulink® coder to retrieve informations
	 * on model signals and store them in rootSignalInfo structure.
	 * @param[in] direction the signal direction.
	 * @param[in] numOfSignals number of signals of the model in the specified direction.
	 * @param[in] modelRootSignals pointer to the C API structure where data are stored.
	 * @param[out] rootSignalInfo GAM private structure where data are stored.
	 * @warning For internal use only.
	 */
	bool GetModelSignalData(const SignalDirection direction,
							const uint32 numOfSignals,
							const rtwCAPI_Signals* modelRootSignals, 
							modelSignalInfoStruct* rootSignalInfo
							);
	
	/**
	 * @brief Parameter info get method.
	 * @details This method extracts all parameter data from the model: it uses
	 * C APIs provided by Simulink® coder to retrieve informations
	 * on model parameters and store them in parameterInfo structure.
	 * @param[in] numOfParams number of parameters of the model.
	 * @param[in] modelParameters pointer to the C API structure where data are stored.
	 * @param[out] parameterInfo GAM private structure where data are stored.
	 * @warning For internal use only.
	 */
	bool GetModelParameterData(const uint32 numOfParams,
							   const rtwCAPI_ModelParameters* modelParameters, 
							   modelParametersInfoStruct* parameterInfo
							   );
	
	/**
	 * @brief Memory transfer from model to GAM and viceversa.
	 * @details Actually this is just a wrapper function that calls the actual memory copying function
	 * CopyRealtimeSignalDataFromAndToModel() with the proper type.
	 * @param[in] direction the signal direction (InputSignals if copying from GAM to model, viceversa OutputSignals).
	 * @param[in] signalIndex index of the signal to be copied.
	 * @param[in] MARTe2SignalDataType type of the signal, so that the correct template function can be invoked.
	 * @warning For internal use only.
	 */
	void ExchangeData(const SignalDirection direction,
					  const uint32 signalIndex,
					  const TypeDescriptor MARTe2SignalDataType
					  );
	
	/**
	 * @brief Actual template function that performs the memory copy from model to GAM and viceversa.
	 * @details This function is invoked by ExchangeData() with the proper type specifier.
	 * It includes row-major/col-major conversion, since Simulink® stores data in column-major
	 * format, while MARTe2 uses row-major format only.
	 * @param[in] direction the signal direction (InputSignals if copying from GAM to model, viceversa OutputSignals).
	 * @param[in] signalIndex index of the signal to be copied. 
	 * @warning For internal use only.
	 */
	template <typename T>
	void CopyRealtimeSignalDataFromAndToModel(const SignalDirection direction,
											  const uint32 signalIndex
											  );
	
	/**
	 * @brief Debugging method
	 */
	template <typename T>
	void PrintSignalForDebugging(uint32 signalIndex);
	
	};
	
} /* namespace MARTe */

/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif 
	
