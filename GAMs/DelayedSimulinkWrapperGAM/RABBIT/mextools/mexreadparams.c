static bool getScalarDoubleParameter(SimStruct *S, double *result, int position, const char *name, double min, double max)
{
  const mxArray *pinput;
  char strtmp[256];
  double dtemp;
  size_t m,n;
  
  pinput = ssGetSFcnParam(S, position);
  
  if(pinput == NULL)
  {
    MEXWARNING("ReadParams:ParameterNotRead","parameter %s not read!", name);
    return false; 
  }
  
#ifdef MATLAB_MEX_FILE
  if(!mxIsClass(pinput, "double"))
  {
      MEXWARNING("ReadParams:ParameterOfWrongType","Parameter %s is not of expected type %s !\n", name,"double");
      return false;
  }
  else
  {
      MEXPRINTF(SFNAME " parameter %s has expected class %s.\n", name,"double");
  }
#endif

  if((m=mxGetM(pinput))!=1 || (n=mxGetN(pinput))!=1)
    {
      MEXPRINTF(SFNAME " parameter %s nrows: %d, ncols %d\n", name, mxGetM(pinput), mxGetN(pinput));
      MEXWARNING("ReadParams:ParameterOfWrongSize"," parameter %s MUST BE a scalar value.\n", name);
      return false;
    }

  dtemp = (double) *((double *)mxGetData(pinput));
  /* Check value min and max*/
  if(dtemp<min || dtemp>max)
  {
    MEXWARNING("ReadParams:ParameterExceedsLimits","Value of parameter %s MUST BE between %f and %f, now: %f\n", name, (float)min, (float)max, (float)dtemp);
    return false;
  }
  MEXPRINTF(STR(S_FUNCTION_NAME) " Scalar double parameter: %s = %f\n", name, (float)dtemp);

  *result = dtemp;
  return true;
  
}

static bool getMatrixDoubleParameter(SimStruct *S, double **result, int position, const char *name, size_t mcheck, size_t ncheck)
{
  size_t m, n;
  const mxArray *mymxMatrix;
  
  mymxMatrix = ssGetSFcnParam(S, position);
  
  if(mymxMatrix == NULL)
  {
    MEXWARNING("ReadParams:ParameterNotRead","parameter %s not read!", name);
    return false;
  }
  
  #ifdef MATLAB_MEX_FILE
  if(!mxIsClass(mymxMatrix, "double"))
  {
    MEXWARNING("ReadParams:ParameterNotRead","parameter %s is of type %s instead of %s !",name,mxGetClassName(mymxMatrix),"double");
    return false;
  }
  #endif

  
  m=mxGetM(mymxMatrix);
  n=mxGetN(mymxMatrix);
    
  MEXPRINTF(STR(S_FUNCTION_NAME) " parameter %s nrows: %d, ncols %d of type double\n", name, m, n);
  
  if(((m != mcheck)&&(mcheck!=0)) || ((n!=ncheck)&&(ncheck!=0)))
  {
    MEXWARNING("ReadParams:WrongSize","wrong %s parameter size, must be %d x %d, found %d x %d\n", name, mcheck, ncheck, m, n);
    return false;
  }
  
  /* allocate memory for this quantity and return pointer*/
  *result = (double *)malloc(m*n*sizeof(double));
  memcpy(*result, mxGetData(mymxMatrix), m*n*sizeof(double));

  return true;
}

static bool getStringParameter(SimStruct *S, char **result, int position, const char *name)
{
  const mxArray *pinput;
  size_t n;
  int status;
  
  pinput = ssGetSFcnParam(S, position);
  
  if(pinput == NULL)
  {
    MEXWARNING("ReadParams:ParameterNotRead","parameter %s not read!", name);
    return false;
    
  }
  
  #ifdef MATLAB_MEX_FILE
  if(mxIsClass(pinput, "char"))
  {
    n = mxGetN((ssGetSFcnParam(S, position)))*sizeof(mxChar)+1;
    *result = (char *)malloc(n); /* allocate result */
    status = mxGetString((ssGetSFcnParam(S, position)), *result,n);
    
    MEXPRINTF(STR(S_FUNCTION_NAME) " parameter %s is class char (%d), value '%s'.\n", name, n, *result);
    return true;
  }
  else
  {
    MEXWARNING("ReadParams:InvalidClass","Parameter %s is NOT class char!\n", name);
    return false;
  }
  #else
    MEXPRINTF(STR(S_FUNCTION_NAME) " string parameter %s loaded %s \n", name, *result );
  #endif
  
  return true;
}
