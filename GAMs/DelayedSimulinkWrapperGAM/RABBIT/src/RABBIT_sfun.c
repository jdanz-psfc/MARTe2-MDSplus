#include "RABBIT_sfun.h"

/* Start of Simulink functions */
static void mdlInitializeSizes(SimStruct *S)
{
  int ii;
  bool paramsok = true;
  double dnsource, dnl, dnp, dnrhotor_out, dnv;
  
  MEXPRINTF("\n*** RABBIT Wrapper - Call to mdlInitializeStates ***\n");
  MEXPRINTF("Getting parameters.\n");
  
  ssSetNumSFcnParams(S, 16); /* number of parameters */
  if (ssGetNumSFcnParams(S) != ssGetSFcnParamsCount(S)) {
    MEXERROR("RABBITwrapper:IncorrectNumberOfParameters",
            "Incorrect number of parameters \n., Required: %d, Received: %d\n",
            ssGetNumSFcnParams(S),ssGetSFcnParamsCount(S));
    return; /* Parameter mismatch reported by the Simulink engine */
  }
  
  /* Check parameter validity. For scalars: min/max value. For matrices, check size
  Some parameters are doubles since Simulink codegen does not support code generation for S functoins that 
  contain non-double parameters; */
  
  paramsok &= getScalarDoubleParameter(S, &dnsource, 0, "nsource", (double)1, (double)NSOURCEMAX);
  nsource = (int)dnsource;
  paramsok &= getScalarDoubleParameter(S, &aplasma, 1, "aplasma", AMIN, AMAX);
  paramsok &= getScalarDoubleParameter(S, &zplasma, 2, "zplasma", ZMIN, ZMAX);
  paramsok &= getScalarDoubleParameter(S, &aimp, 3, "aimp", AMIN, AMAX);
  paramsok &= getScalarDoubleParameter(S, &zimp, 4, "zimp", ZMIN, ZMAX);
  paramsok &= getMatrixDoubleParameter(S, &abeam, 5, "abeam", nsource, 1);
  paramsok &= getMatrixDoubleParameter(S, &zbeam, 6, "zbeam", nsource, 1);
  paramsok &= getMatrixDoubleParameter(S, &xstart, 7, "xstart", 3, nsource);
  paramsok &= getMatrixDoubleParameter(S, &xvec, 8, "xvec", 3, nsource);
  paramsok &= getMatrixDoubleParameter(S, &beamwidthpoly, 9, "beamwidthpoly", 3, nsource);
  paramsok &= getScalarDoubleParameter(S, &dnv, 10, "nv", (double)1, (double)3); /* max 3, min 1 velocity component */
  paramsok &= getScalarDoubleParameter(S, &dnrhotor_out, 11, "nrhotor_out", (double)1, (double)NRHOOUTMAX);
  paramsok &= getMatrixDoubleParameter(S, &R, 12, "R", 0 , 1);
  paramsok &= getMatrixDoubleParameter(S, &Z, 13, "Z", 0 , 1);
  paramsok &= getScalarDoubleParameter(S, &dnl, 14, "l", (double)1 , (double)NMAXL);
  paramsok &= getScalarDoubleParameter(S, &dnp, 15, "p", (double)1 , (double)NMAXP);
  
  /* hard-coded namelist, assigned here */
  namelist = NAMELIST; 

  /* get some key sizes */
  m = mxGetM(ssGetSFcnParam(S,12)); /* R size */
  n = mxGetM(ssGetSFcnParam(S,13)); /* Z size */
  namelist_strlen = strlen(NAMELIST);

  /* convert to ints for RABBIT*/
  np = (int)dnp;
  nl = (int)dnl;
  nv = (int)dnv;
  nrhotor_out = (int)dnrhotor_out;
  
  if(!paramsok)
  {
    MEXERROR("RABBITWrapper:init:ErrorLoadingParameters",
            "One or more parameters passed incorrectly, can not intitialize sizes");
    return;
  }
  else
  {
    MEXPRINTF(SFNAME ", All parameters loaded successully \n");
  }
  


  #ifdef VERBOSE  
  MEXPRINTF("\nInputs loaded by mdlInitializeSizes \n");

  display_rabbit_init_inputs(&aplasma,&zplasma,&aimp,&zimp,abeam, zbeam,
          xstart, xvec, beamwidthpoly,
          &nv, &nsource, &nrhotor_out,
          R, Z, &m, &n,
          &nl, &np, namelist, &namelist_strlen);
  #endif 

  /* set port input sizes */
  ssSetNumInputPorts(S, 22);
  
  ssSetInputPortWidth(S, 0, np);  /* rhotor_kinprof */
  ssSetInputPortWidth(S, 1, np);  /* ne */
  ssSetInputPortWidth(S, 2, np);  /* te */
  ssSetInputPortWidth(S, 3, np);  /* ti */
  ssSetInputPortWidth(S, 4, np);  /* ze */
  ssSetInputPortWidth(S, 5, np);  /* omega_tor */
  ssSetInputPortMatrixDimensions(S, 6, m,n); /* Psi */
  ssSetInputPortWidth(S, 7, nl);  /* Psi1d */
  ssSetInputPortWidth(S, 8, nl);  /* Volume */
  ssSetInputPortWidth(S, 9, nl);  /* Area */
  ssSetInputPortWidth(S, 10, nl); /* rhotor1d */
  ssSetInputPortWidth(S, 11, nl); /* iota */
  ssSetInputPortWidth(S, 12, nl); /* F */
  ssSetInputPortWidth(S, 13, 1);  /* Psi_sep */
  ssSetInputPortWidth(S, 14, 1);  /* Psi_ax */
  ssSetInputPortWidth(S, 15, 1);  /* Rmag */
  ssSetInputPortWidth(S, 16, 1);  /* Zmag */
  ssSetInputPortWidth(S, 17, nsource); /* Pinj */
  ssSetInputPortWidth(S, 18, nsource); /* Einj */
  ssSetInputPortMatrixDimensions(S, 19, nv,nsource); /* specmix */
  ssSetInputPortWidth(S, 19, nv*nsource); /* specmix */
  ssSetInputPortWidth(S, 20, nrhotor_out); /* bdens_in */
  ssSetInputPortWidth(S, 21, 1);  /* dt */
  
  for(ii=0; ii<=21; ii++)
  {
    ssSetInputPortDirectFeedThrough(S, ii, 1); /* All are direct feedthrough since used in mdlOutput */
    ssSetInputPortDataType(S, ii, SS_DOUBLE);  /* All are double */
  }
  
  /* Output port sizes */
  ssSetNumOutputPorts(S,21);
  
  if (nsource==1){
  ssSetOutputPortWidth(S, 0, nrhotor_out); /* power to electrons */
  ssSetOutputPortWidth(S, 1, nrhotor_out); /* power to ions */
  ssSetOutputPortWidth(S, 2, nrhotor_out); /* fast ion pressure */
  ssSetOutputPortWidth(S, 3, nrhotor_out); /* particle source */
  ssSetOutputPortWidth(S, 4, nrhotor_out); /* fast ion density */
  ssSetOutputPortWidth(S, 5, nrhotor_out); /* fast ion current */
  ssSetOutputPortWidth(S, 6, nrhotor_out); /* total driven current */
  ssSetOutputPortWidth(S, 7, nrhotor_out); /* torque to electrons */
  ssSetOutputPortWidth(S, 8, nrhotor_out); /* torque to ions */
  ssSetOutputPortWidth(S, 9, nrhotor_out); /* torque to impurities */
  ssSetOutputPortWidth(S, 10, nrhotor_out); /* neutron rate */
  ssSetOutputPortWidth(S, 11, nrhotor_out); /* rhotor_out */
  ssSetOutputPortWidth(S, 12, 1); /* power total to electrons */
  ssSetOutputPortWidth(S, 13, 1); /* power total to ions */
  ssSetOutputPortWidth(S, 14, 1); /* shinethrough power */
  ssSetOutputPortWidth(S, 15, 1); /* power to rotation */
  ssSetOutputPortWidth(S, 16, 1); /* power to orbit loss */
  ssSetOutputPortWidth(S, 17, 1); /* power to CX loss */
  ssSetOutputPortWidth(S, 18, 1); /* total driven current */
  ssSetOutputPortWidth(S, 19, 1); /* exit flag */
  ssSetOutputPortWidth(S, 20, 1); /* execution time (for performance checking) */
  }
  else
  {
  ssSetOutputPortMatrixDimensions(S, 0, nrhotor_out,nsource); /* power to electrons */
  ssSetOutputPortMatrixDimensions(S, 1, nrhotor_out,nsource); /* power to ions */
  ssSetOutputPortMatrixDimensions(S, 2, nrhotor_out,nsource); /* fast ion pressure */
  ssSetOutputPortMatrixDimensions(S, 3, nrhotor_out,nsource); /* particle source */
  ssSetOutputPortMatrixDimensions(S, 4, nrhotor_out,nsource); /* fast ion density */
  ssSetOutputPortMatrixDimensions(S, 5, nrhotor_out,nsource); /* fast ion current */
  ssSetOutputPortMatrixDimensions(S, 6, nrhotor_out,nsource); /* total driven current */
  ssSetOutputPortMatrixDimensions(S, 7, nrhotor_out,nsource); /* torque to electrons */
  ssSetOutputPortMatrixDimensions(S, 8, nrhotor_out,nsource); /* torque to ions */
  ssSetOutputPortMatrixDimensions(S, 9, nrhotor_out,nsource); /* torque to impurities */
  ssSetOutputPortWidth(S, 10,nrhotor_out); /* neutron rate */
  ssSetOutputPortWidth(S, 11,nrhotor_out);  /* rhotor_out */
  ssSetOutputPortMatrixDimensions(S, 12, 1,nsource); /* power total to electrons */
  ssSetOutputPortMatrixDimensions(S, 13, 1,nsource); /* power total to ions */
  ssSetOutputPortMatrixDimensions(S, 14, 1,nsource); /* shinethrough power */
  ssSetOutputPortMatrixDimensions(S, 15, 1,nsource); /* power to rotation */
  ssSetOutputPortMatrixDimensions(S, 16, 1,nsource); /* power to orbit loss */
  ssSetOutputPortMatrixDimensions(S, 17, 1,nsource); /* power to CX loss */
  ssSetOutputPortMatrixDimensions(S, 18, 1,nsource); /* total driven current */
  ssSetOutputPortMatrixDimensions(S, 19, 1,nsource); /* exit flag */
  ssSetOutputPortWidth(S, 20, 1); /* execution time (for performance checking) */
  }
    
  ssSetNumSampleTimes(S, 1);
  
  MEXPRINTF(" Port sizes initialized \n");
}

static void mdlInitializeSampleTimes(SimStruct *S)
{
    MEXPRINTF("Set inherited sample time\n");
  ssSetSampleTime(S, 0, INHERITED_SAMPLE_TIME);
  ssSetOffsetTime(S, 0, 0.0);
}

/* MDL START */

#define MDL_START                      /* Change to #undef to remove function */
#if defined(MDL_START)
static void mdlStart(SimStruct *S)
{
  int ii;
  int ierr[nsource];
  struct stat buffer;
  char cmd[200];
  
  MEXPRINTF("\n*** RABBIT Wrapper - Call to mdlStart ***\n");


  #ifndef MOCK
  /* Check existence of RABBIT_PATH*/
  MEXPRINTF(" Checking RABBIT_PATH...%s\n",getenv("RABBIT_PATH"));
  if (!stat(getenv("RABBIT_PATH"), &buffer) && S_ISDIR(buffer.st_mode))
   {
   MEXPRINTF("...${RABBIT_PATH} is a directory.\n");
   }
  else
   {
    MEXERROR("RABBITwrapper:RABBITpathError","aa ${RABBIT_PATH} is not defined or not a directory");
   }

  /* Check existence of namelist link */
  MEXPRINTF(" Checking namelist link %s\n",namelist);
  sprintf(cmd,"test -L %s",namelist); /* check if link exists */
  MEXPRINTF("\n%s\n",cmd);
  if(system(cmd)==0)
  {
    sprintf(cmd,"test -e %s",namelist);
    MEXPRINTF("\n%s\n",cmd);
    if (system(cmd)==0)
     {
      MEXPRINTF("...namelist link exists\n");
     }
    else 
     {
      MEXERROR("RABBITwrapper:NamelistLinkBroken","Namelist link %s is broken",namelist);
     }
  }
  else
  {
    MEXERROR("RABBITwrapper:NamelistLinkDoesNotExist",
              "Namelist link %s does not exist, use set_RABBIT_namelist.m or set manually",namelist);
  }
  #endif

  #ifdef VERBOSE  
  /* optional display of init inputs */
  MEXPRINTF("Inputs passed to RABBIT_INIT\n");
  display_rabbit_init_inputs(&aplasma,&zplasma,&aimp,&zimp,abeam, zbeam,
          xstart, xvec, beamwidthpoly,
          &nv, &nsource, &nrhotor_out,
          R, Z, &m, &n,
          &nl, &np, namelist, &namelist_strlen);
  #endif
  
  
  RABBIT_INIT(&aplasma,&zplasma,&aimp,&zimp,abeam,zbeam,
          xstart, xvec, beamwidthpoly,
          &nv, &nsource, &nrhotor_out,
          R, Z, &m, &n,
          &nl, &np, namelist, &namelist_strlen, ierr);

  char *dumpdir1 = getenv("RABBIT_PATH");
  char *dumpdir2 = "/dump/";
  char dumpdir[1000];
  strcpy(dumpdir, dumpdir1);
  strcat(dumpdir, dumpdir2);
  printf(dumpdir);
  int32_t dumpdir_len = strlen(dumpdir);
  rabbit_lib_set_dump_dir(dumpdir, &dumpdir_len);
  double einjarr[nsource];
  double specmixarr[nsource*nv];
  rabbit_lib_dump_beams(dumpdir, &dumpdir_len, einjarr, specmixarr);
  
  
  
  for(ii=0; ii<nsource; ii++)
  {
    MEXPRINTF("exitflag = %d returned for beam number %d\n",ierr[ii],ii+1);
    if ( ierr[ii]!=0 )
    {
      MEXPRINTF("exitflag = %d returned for beam number %d\n",ierr[ii],ii+1);
      MEXERROR("RABBITwrapper:RabbitInitError",
              "Nonzero errorflag returned by RABBIT initialization");
    }
  }
}
#endif /*  MDL_START */

/* MDL OUTPUTS  */

static void mdlOutputs(SimStruct *S, int_T tid)
{
  int ii;
  double output_timing; /* RT-compatible time of RABBIT outputs */
  
  clock_t tstart;
  clock_t telaps;
  double time_spent;
  
  time_T simulationtime;
  
  /* getting current simulation time */
  simulationtime = ssGetT(S);
  
  MEXPRINTF("\n\n***" SFNAME);
  MEXPRINTF(" Wrapper - Call to mdlOutputs for Simulation Time %f ***\n",simulationtime);
  
  
  /* Getting pointers to input and output signals */
  InputRealPtrsType rhotor_kinprof = ssGetInputPortRealSignalPtrs(S,0);
  InputRealPtrsType ne          = ssGetInputPortRealSignalPtrs(S,1);
  InputRealPtrsType Te          = ssGetInputPortRealSignalPtrs(S,2);
  InputRealPtrsType Ti          = ssGetInputPortRealSignalPtrs(S,3);
  InputRealPtrsType Zeff        = ssGetInputPortRealSignalPtrs(S,4);
  InputRealPtrsType omega_tor   = ssGetInputPortRealSignalPtrs(S,5);
  
  InputRealPtrsType Psi  = ssGetInputPortRealSignalPtrs(S,6);
  
  InputRealPtrsType Psi1d       = ssGetInputPortRealSignalPtrs(S,7);
  InputRealPtrsType Volume      = ssGetInputPortRealSignalPtrs(S,8);
  InputRealPtrsType Area        = ssGetInputPortRealSignalPtrs(S,9);
  InputRealPtrsType rhotor_equil= ssGetInputPortRealSignalPtrs(S,10);
  InputRealPtrsType iota        = ssGetInputPortRealSignalPtrs(S,11);
  InputRealPtrsType F           = ssGetInputPortRealSignalPtrs(S,12);
  InputRealPtrsType Psi_sep     = ssGetInputPortRealSignalPtrs(S,13);
  InputRealPtrsType Psi_axis    = ssGetInputPortRealSignalPtrs(S,14);
  InputRealPtrsType Rmag        = ssGetInputPortRealSignalPtrs(S,15);
  InputRealPtrsType Zmag        = ssGetInputPortRealSignalPtrs(S,16);
  InputRealPtrsType Pinj        = ssGetInputPortRealSignalPtrs(S,17);
  InputRealPtrsType Einj        = ssGetInputPortRealSignalPtrs(S,18);
  InputRealPtrsType specmix     = ssGetInputPortRealSignalPtrs(S,19);
  InputRealPtrsType bdens_in    = ssGetInputPortRealSignalPtrs(S,20);
  InputRealPtrsType dt          = ssGetInputPortRealSignalPtrs(S,21);
  
  real_T *powe        = ssGetOutputPortRealSignal(S,0);
  real_T *powi        = ssGetOutputPortRealSignal(S,1);
  real_T *press       = ssGetOutputPortRealSignal(S,2);
  real_T *bdep        = ssGetOutputPortRealSignal(S,3);
  real_T *bdens       = ssGetOutputPortRealSignal(S,4);
  real_T *jfi         = ssGetOutputPortRealSignal(S,5);
  real_T *jnbcd       = ssGetOutputPortRealSignal(S,6);
  real_T *torqe       = ssGetOutputPortRealSignal(S,7);
  real_T *torqi       = ssGetOutputPortRealSignal(S,8);
  real_T *torqimp     = ssGetOutputPortRealSignal(S,9);
  real_T *nrate       = ssGetOutputPortRealSignal(S,10);
  
  real_T *rhotor_out  = ssGetOutputPortRealSignal(S,11);
  real_T *powe_total  = ssGetOutputPortRealSignal(S,12);
  real_T *powi_total  = ssGetOutputPortRealSignal(S,13);
  real_T *Pshine      = ssGetOutputPortRealSignal(S,14);
  real_T *Prot        = ssGetOutputPortRealSignal(S,15);
  real_T *Porbitloss  = ssGetOutputPortRealSignal(S,16);
  real_T *Pcxloss     = ssGetOutputPortRealSignal(S,17);
  real_T *Inb_total   = ssGetOutputPortRealSignal(S,18);
  int *ierr        = (int *)ssGetOutputPortSignal(S,19);
  real_T *exec_time   = ssGetOutputPortRealSignal(S,20);
  
  output_timing = OUTPUTTIMING; /* hard-coded RABBIT variable to time at wich outputs are given */

  /* Call to step */
#ifdef TIMER
  tstart=clock();
#endif
  
#ifdef VERBOSE
  MEXPRINTF("Inputs to RABBIT_STEP\n");
  display_rabbit_step_inputs(rhotor_kinprof[0],ne[0],Te[0],Ti[0],Zeff[0],omega_tor[0],&np,
          Psi[0], Psi1d[0], Volume[0], Area[0], rhotor_equil[0], iota[0], F[0],
          Psi_sep[0], Psi_axis[0], Rmag[0], Zmag[0], &m, &n, &nl,
          Pinj[0], Einj[0],specmix[0], &nv, &nsource, bdens_in[0], &nrhotor_out,
          dt[0],&output_timing);
  
  MEXPRINTF("\nCall RABBIT_STEP \n");
#endif
  
  RABBIT_STEP(rhotor_kinprof[0],ne[0],Te[0],Ti[0],Zeff[0],omega_tor[0],&np,
          Psi[0], Psi1d[0], Volume[0], Area[0], rhotor_equil[0], iota[0], F[0],
          Psi_sep[0], Psi_axis[0], Rmag[0], Zmag[0], &m, &n, &nl,
          Pinj[0], Einj[0], specmix[0], &nv, &nsource, bdens_in[0],
          dt[0], &output_timing,
          powe, powi, press,
          bdep, bdens, jfi, jnbcd, torqe, torqi, torqimp, nrate,rhotor_out,
          &nrhotor_out,powe_total,powi_total,Pshine,Prot,Porbitloss,Pcxloss,
          Inb_total,ierr,NULL);
  
  MEXPRINTF("Done calling RABBIT_STEP\n");

#ifdef VERBOSE
  MEXPRINTF("Outputs of RABBIT_STEP\n");
  display_rabbit_step_outputs(
          powe, powi, press,
          bdep, bdens, jfi, jnbcd, torqe, torqi, torqimp, nrate,rhotor_out,
          &nrhotor_out,powe_total,powi_total,Pshine,Prot,Porbitloss,Pcxloss,
          Inb_total,ierr,&nsource);
  
#endif
  
  /* Check exitflags */
  for(ii=0; ii<nsource; ii++)
  { 
      MEXPRINTF("exitflag = %d returned for beam number %d\n",ierr[ii],ii+1);
    if ( ierr[ii]!=0 )
    {
      MEXERROR("RABBITwrapper:RabbitStepError",
              "Nonzero errorflag=% returned by RABBIT step for beam %d",ierr[ii],ii+1);
    }
  }
   
  /* Timing */
#ifdef TIMER
  telaps = clock()-tstart;
  time_spent = ((double)telaps/CLOCKS_PER_SEC);
#else
  time_spent = (double)0.0;
#endif
*exec_time = 1000*(time_spent);

#ifdef TIMER
  MEXPRINTF(SFNAME " exec time [ms] %f\n", *exec_time);
#endif
}

static void mdlTerminate(SimStruct *S)
{
  int ii;
  int ierr[nsource];
  
  MEXPRINTF("\n\n*** %s Wrapper - Call to mdlTerminate *** \n",SFNAME);
  
  RABBIT_DEINIT(&nsource,ierr);
  
  for(ii=0; ii<nsource; ii++)
  {
    MEXPRINTF("exitflag = %d returned for beam number %d\n",ierr[ii],ii+1);
    if ( ierr[ii]!=0 )
    {
      MEXPRINTF("exitflag = %d returned for beam number %d\n",ierr[ii],ii+1);
      MEXERROR("RABBITwrapper:RabbitDeInitError",
              "Nonzero errorflag returned by RABBIT de-initialization");
    }
  }

  /* free other memory */
  free(abeam);
  free(zbeam);
  free(xstart);
  free(xvec);
  free(beamwidthpoly);
  free(R);
  free(Z);
  return;
  
}

#ifdef MATLAB_MEX_FILE /* Is this file being compiled as a MEX-file? */
#include "simulink.c" /* MEX-file interface mechanism */
#else
#include "cg_sfun.h" /* Code generation registration function */
#endif
