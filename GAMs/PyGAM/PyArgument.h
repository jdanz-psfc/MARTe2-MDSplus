/**
 * @file PyArgument.h
 * @brief Header file for class PyGAM
 * @date Jan 8, 2020
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

#ifndef PYARGUMENT_H_
#define PYARGUMENT_H_

/*---------------------------------------------------------------------------*/
/*                          Standard header includes                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                          Project header includes                          */
/*---------------------------------------------------------------------------*/

#include "GAM.h"
#include "StructuredDataI.h"
#include "MessageI.h"

#include "PyGAM.h"

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
 * @brief PyGAM helper file containing PyArgumentStruct class.
 * 
 * @todo 1. encapsulate all variables (name, pyType etc.) in a sub-class
 * @todo 2. insert as variable of the PyArgument superclass the argument
 *          direction and number of arguments, so that GetTypes(),
 *          GetDimensions() and GetNames() methods do not need them as inputs
 *          anymore
 */

enum PyArgumentDirection {
	
	PyArgInputs,
	PyArgOutputs,
	PyArgParameters
	
};
	
class PyArgumentStruct						/// Class storing informations on inputs, outputs and parameters of the Python code.
{	
public:
	
	StreamString   name;							//!< Argument name.
	
	// Type
	PyObject*      pyType;							//!< Argument type retrieved from Python code as a Python typeobject.
	TypeDescriptor pyTypeInMARTe2;					//!< Argument type retrieved from Python code as a MARTe2 type.               @details Stored by Initialise().
	TypeDescriptor GAMType;							//!< Argument type retrieved from MARTe2 configuration file as a MARTe2 type.
	uint32         enumType;						//!< Argument type retrieved from MARTe2 configuration file as numpy typenum. @details See numpy C API docs.
	
	// Dimensions
	uint32         GAMNumOfElts;					//!< Argument number of elements retrieved from MARTe2 configuration file.
	uint32         numberOfDimensions;				//!< Argument number of dimensions.                                           @details 0 if scalar, 1 if vector, 2 if matrix.
	uint32         dimensions[2];					//!< Argument dimensions.                                                     @details [1 1] if scalar, [1 N] or [N 1] if vector, [N M] if matrix.
	uint32         size;
	
	// Memory
	void*          pyAddress;
	void*          GAMAddress;
	
	// Methods
	
	/**
	 * @brief      Fill a PyArgumentStruct datatype fields with data from pythonModule.
	 * @param[in]  direction    input, output or parameter.
	 * @param[in]  numberOfArg  length of the array.
	 * @param[in]  pythonModule module from which datatypes are imported. 
	 * @return     True on success.
	 */
	bool GetTypes(PyArgumentDirection direction, uint32 numberOfArgs, PyObject* pythonModule);
	
	/**
	 * @brief      Fill a PyArgumentStruct dimension fields with data from pythonModule.
	 * @param[in]  direction    input, output or parameter.
	 * @param[in]  numberOfArg  length of the array.
	 * @param[in]  pythonModule module from which datatypes are imported. 
	 * @return     True on success.
	 */
	bool GetDimensions(PyArgumentDirection direction, uint32 numberOfArgs, PyObject* pythonModule);
	
	/**
	 * @brief      Fill a PyArgumentStruct name fields with data from pythonModule.
	 * @param[in]  direction    input, output or parameter.
	 * @param[in]  numberOfArg  length of the array.
	 * @param[in]  pythonModule module from which datatypes are imported. 
	 * @return     True on success.
	 */
	bool GetNames(PyArgumentDirection direction, uint32 numberOfArgs, PyObject* pythonModule);
	
	/**
	 * @brief      Returns the MARTe2 type from a PyObject* PyTypeObject.
	 * @return     TypeDescriptor object. InvalidType on fail.
	 * @details    This method can deal with Python native types (int and float,
	 *             which are considered int64 and float64 rispectively) and with
	 *             numpy types (which are directly translated in their MARTe2
	 *             counterpart of the same name).
	 */
	TypeDescriptor MARTe2TypeFromPythonType(PyObject* typeObj);
	
};
	
} // namespace MARTe

/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

#endif // PYARGUMENT_H_