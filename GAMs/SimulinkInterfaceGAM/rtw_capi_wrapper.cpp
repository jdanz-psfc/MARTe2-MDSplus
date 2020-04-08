#include "rtw_capi.h"
#include "rtw_modelmap.h"

extern "C" {
	
int WCAPI_GetNumRootInputs(rtwCAPI_ModelMappingInfo* MMI) {
	
	return rtwCAPI_GetNumRootInputs(MMI);
	
}

int WCAPI_GetNumRootOutputs(rtwCAPI_ModelMappingInfo* MMI) {
	
	return rtwCAPI_GetNumRootOutputs(MMI);
	
}

int WCAPI_GetNumModelParameters(rtwCAPI_ModelMappingInfo* MMI) {
	
	return rtwCAPI_GetNumModelParameters(MMI);
	
}

/* Get structures */

// Signal and paramter structures
const rtwCAPI_Signals* WCAPI_GetRootInputs(rtwCAPI_ModelMappingInfo* MMI) {
	
	return rtwCAPI_GetRootInputs(MMI);
	
}

const rtwCAPI_Signals* WCAPI_GetRootOutputs(rtwCAPI_ModelMappingInfo* MMI) {
	
	return rtwCAPI_GetRootOutputs(MMI);
	
}

const rtwCAPI_ModelParameters* WCAPI_GetModelParameters(rtwCAPI_ModelMappingInfo* MMI) {
	
	return rtwCAPI_GetModelParameters(MMI);
	
}

// Data structures
const rtwCAPI_DataTypeMap* WCAPI_GetDataTypeMap(rtwCAPI_ModelMappingInfo* MMI) {
	
	return rtwCAPI_GetDataTypeMap(MMI);
	
}

const rtwCAPI_DimensionMap* WCAPI_GetDimensionMap(rtwCAPI_ModelMappingInfo* MMI) {
	
	return rtwCAPI_GetDimensionMap(MMI);
	
}

uint const* WCAPI_GetDimensionArray(rtwCAPI_ModelMappingInfo* MMI) {
	
	return rtwCAPI_GetDimensionArray(MMI);
	
}

void** WCAPI_GetDataAddressMap(rtwCAPI_ModelMappingInfo* MMI) {
	
	return rtwCAPI_GetDataAddressMap(MMI);
	
}

/* Get single data */
const char* WCAPI_GetSignalName(rtwCAPI_Signals* bio, int i) {
	
	return rtwCAPI_GetSignalName(bio, i);
	
}

const char* WCAPI_GetModelParameterName(rtwCAPI_ModelParameters* prm, int i) {
	
	return rtwCAPI_GetModelParameterName(prm, i);
	
}

uint WCAPI_GetSignalDataTypeIdx(rtwCAPI_Signals* bio, int i) {
	
	return rtwCAPI_GetSignalDataTypeIdx(bio, i);
	
}

uint WCAPI_GetModelParameterDataTypeIdx(rtwCAPI_ModelParameters* prm, int i) {
	
	return rtwCAPI_GetModelParameterDataTypeIdx(prm, i);
	
}

uint WCAPI_GetDataTypeSLId(rtwCAPI_DataTypeMap* dTypeMap, int i) {
	
	return rtwCAPI_GetDataTypeSLId(dTypeMap, i);
	
}

void* WCAPI_GetDataAddress(void** dataAddrMap, int i) {
	
	return rtwCAPI_GetDataAddress(dataAddrMap, i);
	
}

const char* WCAPI_GetDataTypeCName(rtwCAPI_DataTypeMap* dTypeMap, int i) {
	
	return rtwCAPI_GetDataTypeCName(dTypeMap, i);
	
}

uint WCAPI_GetSignalDimensionIdx(rtwCAPI_Signals* bio, int i) {
	
	return rtwCAPI_GetSignalDimensionIdx(bio, i);
	
}

uint WCAPI_GetModelParameterDimensionIdx(rtwCAPI_ModelParameters* prm, int i) {
	
	return rtwCAPI_GetModelParameterDimensionIdx(prm, i);
	
}

uint WCAPI_GetModelParameterAddrIdx(rtwCAPI_ModelParameters* prm, int i) {
	
	return rtwCAPI_GetModelParameterAddrIdx(prm, i);
	
}

uint WCAPI_GetDimArrayIndex(rtwCAPI_DimensionMap* dimMap, int i) {
	
	return rtwCAPI_GetDimArrayIndex(dimMap, i);
	
}

uint WCAPI_GetNumDims(rtwCAPI_DimensionMap* dimMap, int i) {
	
	return rtwCAPI_GetNumDims(dimMap, i);
	
}

} // extern "C"