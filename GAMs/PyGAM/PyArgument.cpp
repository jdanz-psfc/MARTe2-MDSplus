/**
 * @file PyArgument.cpp
 * @brief Source file for class PyGAM
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

 * @details This source file contains the definition of all the methods for
 * the class PyGAM (public, protected, and private). Be aware that some
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

#include "PyGAM.h"
#include "PyArgument.h"
#include "AdvancedErrorManagement.h"
#include <signal.h>

/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

namespace MARTe {

bool PyArgumentStruct::GetTypes(PyArgumentDirection direction, uint32 numberOfArgs, PyObject* pythonModule, PyObject *moduleName) {
	
	bool ok = false;
	PyObject* pFunc;

	for (uint32 argIdx = 0; argIdx < numberOfArgs; argIdx++) {
		
		if (direction == PyArgInputs) {
			
			pFunc = PyObject_GetAttrString(pythonModule, "getInputType");
			
		} else if (direction == PyArgOutputs) {
			
			pFunc = PyObject_GetAttrString(pythonModule, "getOutputType");
			
		} else if (direction == PyArgParameters) {
			
			pFunc = PyObject_GetAttrString(pythonModule, "getParameterType");
		}
		
		ok = (pFunc && PyCallable_Check(pFunc));
		if (!ok) return ok;
									
		PyObject* pIndex = PyLong_FromLong(argIdx);
		this[argIdx].pyType = PyObject_CallFunctionObjArgs(pFunc, moduleName, pIndex, NULL);
		ok=(this[argIdx].pyType != NULL);
		
		if (!ok) return ok;
		
		this[argIdx].pyTypeInMARTe2 = MARTe2TypeFromPythonType(this[argIdx].pyType);
		if (this[argIdx].pyTypeInMARTe2 == 0) return false;
		ok = (this[argIdx].pyTypeInMARTe2 != InvalidType);
		if (!ok) return ok;
									
	}
	
	Py_XDECREF(pFunc);
	
	return ok;
	
} // GetTypes

bool PyArgumentStruct::GetDimensions(PyArgumentDirection direction, uint32 numberOfArgs, PyObject* pythonModule, PyObject *moduleName) {
	
	bool ok = false;
	StreamString directionString;
	PyObject* pFunc;
	PyObject* pValue;

	for (uint32 argIdx = 0; argIdx < numberOfArgs; argIdx++) {
		
		if (direction == PyArgInputs) {
			
			pFunc = PyObject_GetAttrString(pythonModule, "getInputNumberOfDimensions");
			directionString = "Input ";
			
		} else if (direction == PyArgOutputs) {
			
			pFunc = PyObject_GetAttrString(pythonModule, "getOutputNumberOfDimensions");
			directionString = "Output";
			
		} else if (direction == PyArgParameters) {
			
			pFunc = PyObject_GetAttrString(pythonModule, "getParameterNumberOfDimensions");
			directionString = "Param.";
			
		}
		
		ok = (pFunc && PyCallable_Check(pFunc));
		if (!ok) return ok; //TODO not working, replace with "goto error"
		
		pValue = PyObject_CallFunctionObjArgs(pFunc, moduleName, PyLong_FromLong(argIdx), NULL);
		
		this[argIdx].numberOfDimensions = PyLong_AsLong(pValue);
		
		if (direction == PyArgInputs) {
			
			pFunc = PyObject_GetAttrString(pythonModule, "getInputDimensions");
			
		} else if (direction == PyArgOutputs) {
			
			pFunc = PyObject_GetAttrString(pythonModule, "getOutputDimensions");
			
		} else if (direction == PyArgParameters) {
			
			pFunc = PyObject_GetAttrString(pythonModule, "getParameterDimensions");
		}
		
		ok = (pFunc && PyCallable_Check(pFunc));
		if (!ok) return ok; //TODO not working, replace with "goto error"
		
		pValue = PyObject_CallFunctionObjArgs(pFunc, moduleName, PyLong_FromLong(argIdx), NULL);
		
		PyObject* temp = PyTuple_GetItem(pValue, 0);
		this[argIdx].dimensions[0] = PyLong_AsLong(temp);
		
		temp = PyTuple_GetItem(pValue, 1);
		this[argIdx].dimensions[1] = PyLong_AsLong(temp);
		
		// Debug
		REPORT_ERROR_STATIC(ErrorManagement::Debug,
					 "%s [%i] dim: %i [%i %i]",
					 directionString.Buffer(),
					 argIdx,
					 this[argIdx].numberOfDimensions,
					 this[argIdx].dimensions[0], this[argIdx].dimensions[1]);
		
	}
	
	Py_XDECREF(pFunc);
	Py_XDECREF(pValue);
	
	return ok;
	
} // GetDimensions

bool PyArgumentStruct::GetNames(PyArgumentDirection direction, uint32 numberOfArgs, PyObject* pythonModule, PyObject *moduleName) {
	
	bool ok = false;
	StreamString directionString;
	PyObject* pFunc;
	PyObject* pValue;
	
	for (uint32 argIdx = 0; argIdx < numberOfArgs; argIdx++) {
		
		if (direction == PyArgInputs) {
			
			pFunc = PyObject_GetAttrString(pythonModule, "getInputName");
			directionString = "Input ";
			
		} else if (direction == PyArgOutputs) {
			
			pFunc = PyObject_GetAttrString(pythonModule, "getOutputName");
			directionString = "Output";
			
		} else if (direction == PyArgParameters) {
			
			pFunc = PyObject_GetAttrString(pythonModule, "getParameterName");
			directionString = "Param.";
			
		}
		
		ok = (pFunc && PyCallable_Check(pFunc));
		if (!ok) return ok; //TODO not working, replace with "goto error"
		
		pValue = PyObject_CallFunctionObjArgs(pFunc, moduleName, PyLong_FromLong(argIdx), NULL);
		
		this[argIdx].name = PyUnicode_AsUTF8(pValue);
		
		// Debug
		REPORT_ERROR_STATIC(ErrorManagement::Debug,
					 "%s [%i] name: %s",
					directionString.Buffer(),
					argIdx,
					(this[argIdx].name).Buffer()
					 );
		
	}
	
	Py_XDECREF(pFunc);
	Py_XDECREF(pValue);
	
	return ok;
	
}

TypeDescriptor PyArgumentStruct::MARTe2TypeFromPythonType(PyObject* typeObj) {
	
	TypeDescriptor MARTe2Type;
	
	PyObject* repr = PyObject_Repr(PyObject_GetAttrString(typeObj, "__name__"));
	PyObject* str  = PyUnicode_AsEncodedString(repr, "utf-8", "~E~");
	StreamString typeStr = PyBytes_AS_STRING(str);
	
	if (typeStr=="'int8'") {
		
		MARTe2Type = SignedInteger8Bit;
		
	} else if (typeStr=="'int16'") {
		
		MARTe2Type = SignedInteger16Bit;
		
	} else if (typeStr=="'int32'") {
		
		MARTe2Type = SignedInteger32Bit;
		
	} else if (typeStr=="'int'" || typeStr=="'int64'") {
		
		MARTe2Type = SignedInteger64Bit;
		
	} else if (typeStr=="'uint8'") {
		
		MARTe2Type = UnsignedInteger8Bit;
		
	} else if (typeStr=="'uint16'") {
		
		MARTe2Type = UnsignedInteger16Bit;
		
	} else if (typeStr=="'uint32'") {
		
		MARTe2Type = UnsignedInteger32Bit;
		
	} else if (typeStr=="'uint64'") {
		
		MARTe2Type = UnsignedInteger64Bit;
		
	} else if (typeStr=="'float32'") {
		
		MARTe2Type = Float32Bit;
		
	} else if (typeStr=="'float'" || (typeStr=="'float64'")) {
		
		MARTe2Type = Float64Bit;
		
	} else {
		
		REPORT_ERROR_STATIC(ErrorManagement::Exception,
					 "Unsupported Python type. MARTe2 compatible types are supported.");
		MARTe2Type = InvalidType;
		
	}
	
	return MARTe2Type;
	
} // MARTe2TypeFromPythonType

} // namespace MARTe

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/