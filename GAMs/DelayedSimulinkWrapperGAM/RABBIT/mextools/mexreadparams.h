#ifndef _MEXREADPARAMS_
#define _MEXREADPARAMS_

static bool getScalarDoubleParameter(SimStruct *S, double *result, int position, const char *name, double min, double max);
static bool getMatrixDoubleParameter(SimStruct *S, double **result, int position, const char *name, size_t mcheck, size_t ncheck);
static bool getStringParameter(SimStruct *S, char **result, int position, const char *name);

#endif
