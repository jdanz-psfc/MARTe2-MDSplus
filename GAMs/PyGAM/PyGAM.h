/**
 * @file PyGAM.h
 * @brief Header file for class PyGAM
 * @date Oct 8, 2019
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
 *
 * @details This header file contains the declaration of the class ConstantGAM
 * with all of its public, protected and private members. It may also include
 * definitions for inline methods which need to be visible to the compiler.
 */

#ifndef PYGAM_H_
#define PYGAM_H_

/*---------------------------------------------------------------------------*/
/*                          Standard header includes                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                          Project header includes                          */
/*---------------------------------------------------------------------------*/


#pragma gcc diagnostic push
#pragma gcc diagnostic ignored "-Wall"
#pragma gcc diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wcomment"
#pragma GCC diagnostic ignored "-Wreturn-type"
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wundef"
#pragma GCC diagnostic ignored "-fpermissive"
#pragma GCC diagnostic ignored "-pedantic"
#include <xla/literal.h>
#include <xla/literal_util.h>
#include <xla/pjrt/pjrt_client.h>
#include <xla/pjrt/gpu/gpu_helpers.h>
#include <xla/pjrt/gpu/se_gpu_pjrt_client.h>
#include <xla/status.h>
#include <xla/statusor.h>
#include <xla/tools/hlo_module_loader.h>
#include <tsl/platform/init_main.h>
#include <tsl/platform/logging.h>
#pragma gcc diagnostic pop

#include "GAM.h"
#include "StructuredDataI.h"
#include "MessageI.h"

#include "PyArgument.h"

/*---------------------------------------------------------------------------*/
/*                              Python includes                              */
/*---------------------------------------------------------------------------*/

#define PY_SSIZE_T_CLEAN
#include "Python.h"

#define NPY_NO_DEPRECATED_API NPY_API_VERSION
#include "numpy/arrayobject.h"

/*---------------------------------------------------------------------------*/
/*                             Class declaration                             */
/*---------------------------------------------------------------------------*/

namespace MARTe {

/**
* @brief GAM which wraps Python code.
* @details
* This GAM wraps Python code so that it can be run as a MARTe2 GAM.
* 
* By default the GAM checks that the output tuple returned by the Python code
* is coherent with the layout of the output signals as declared in the
* configuration file at every cycle. This check can be disabled to improve
* performances by setting RealtimeOutputCheck parameter to 0.
* If no RealtimeOutputCheck parameter is specified, the GAM assumes it to be 1.
* 
* <pre>
* +MyGAM = {
*     Class               = PyGAM
*     FileName            = "pythonModuleName"
*     RealtimeOutputCheck = 1                   // Set to 0 to disable checks on the output tuple.
*     Parameters = {
*         param1 = 1
*         param2 = {1, 2}
*         param3 = { {1, 2}, {3, 4} }
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
* ... some additional info ...
* 
* @pre 1. Number of inputs and outputs of the configuration file for this GAM
* must match exactely the number of (respectively) arguments and return values
* of the Python code the GAM will run.
* @pre 2. Values of parameters declared in the model must be set in the configuration file.
* Not setting the value for all paramters returns an error.
* 
* @todo 1. Presence of dictionaries in the Python file should be made optional, and
* in that case data on arguments  (that is, inputs, outputs and parameters)
* should be retrieved from MARTe2 configuration file.
* 
* @todo 2. Multiple return statements should be removed in accompliance with
* MISRA:C++2008 rules. They could be substituted with multiple if-statements
* or with a clever usage of goto, which is not explicitely banned, or with
* try-throw-catch blocks (which seems to be the best solution).
*/

class PyGAM : public GAM {

public:
	
	CLASS_REGISTER_DECLARATION()
	
	PyGAM();
	
	virtual ~PyGAM();
	
	virtual bool Initialise(StructuredDataI & data);
	
	virtual bool Setup();
	
	virtual bool Execute();
	
private:
	
	/**
	 * @name Debug
	 * 
	 */
	//@{
	bool    isFirstRun;
	float32 sleepTime;	//!< To put the execute method to sleep. Set to 0 to avoid sleep. @warning For debugging purpose only.
	//@}
	
	/**
	 * @name Parameters
	 * 
	 */
	//@{
	StreamString fileName;
	uint8         realtimeCheckFlag;
	//@}
	
	/**
	 * @name Python objects
	 * 
	 */
	//@{
	PyObject *pName, *pModule, *pModuleName, *pFunc, *pygamModule;
	PyObject *pValue;
	
	PyObject *rootInputDict;							//!< PyObject dictionary with input data imported from Python code.
	PyObject *rootOutputDict;							//!< PyObject dictionary with output data imported from Python code.
	PyObject *parameterDict;							//!< PyObject dictionary with parameter data imported from Python code.
	
	PyObject *pInputs;									//!< PyObject tuple holding input values.  @details A tuple is needed since only tuples are allowed as input of PyObject_CallObject().
	PyObject *pOutputs;									//!< PyObject tuple holding output values. @details A tuple is needed since only tuples are allowed as input of PyObject_CallObject().

	// Take this, wrap it in a struct
	// void * to be generic pointer because
	// there's stuff to do with unions to make that a little
	// finding the XLA's ability to create a buffer and then write to it again
	// an XLA structure that we can update is the key
	// if it really comes down to having to make an XLA structure every frame, the Jax stuff is probably a failure
	float *inputMemory;
	//@}
	
	/**
	 * @name Python model informations
	 * 
	 */
	//@{
	uint32 pyNumOfInputs;
	uint32 pyNumOfOutputs;
	uint32 pyNumOfParameters;
	
	PyArgumentStruct *pyInputStruct;					//!< Structure to store informations on Python code inputs.
	PyArgumentStruct *pyOutputStruct;					//!< Structure to store informations on Python code outputs.
	PyArgumentStruct *pyParameterStruct;				//!< Structure to store informations on Python code outputs.
	//@}
	
	/**
	 * @name Error handling
	 * 
	 */
	//@{
	PyObject *pErrorType;
	PyObject *pErrorValue;
	PyObject *pTraceback;
	//@}
	
	/**
	 * @brief      Creates a tuple that can hold values for inputs and outputs.
	 * @details    Since Python C-APIs require tuples to be passed in and out from
	 *             function, all data must be packed into tuples.
	 * @param[in]  
	 * @param[out] 
	 * @warning    PyObject_CallObject() changes the memory location of the tuple
	 *             it uses as output, so if the tuple created by this function is
	 *             used as output, then its addresses must be re-retrieved on each
	 *             cycle.
	 */
	PyObject* CreateArgTuple(const SignalDirection   direction,
							       PyArgumentStruct* pyStruct);
	
	/**
	 * @brief      Refreshes data.
	 * @details    This methon updates input or output tuple.
	 * @param[in]  
	 * @param[out]
	 * @warning    CallObject() returns a new reference of the tuple that is used
	 *             as its output, so this is required only for the output tuple.
	 */
	bool RefreshData(const SignalDirection direction);
	
	/**
	 * @brief Actual refreshing (templated).
	 * 
	 */
	template <typename T>
	void PyMemCopy(const     SignalDirection direction,
				   uint32    argIdx,
				   uint32    elemIdx,
				   PyObject* temp);
	
	/**
	 * @brief      Returns the PyArray enumerated type from a MARTe2 TypeDescriptor.
	 * @return     PyArray enumerated type. 0 on fail.
	 * 
	 */
	uint32 NumPyEnumTypeFromMARTe2Type(TypeDescriptor MARTe2Type);
	
	std::unique_ptr<xla::PjRtClient> client;
	std::unique_ptr<xla::PjRtLoadedExecutable> executable;
};

} // namespace MARTe

/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif 
	
