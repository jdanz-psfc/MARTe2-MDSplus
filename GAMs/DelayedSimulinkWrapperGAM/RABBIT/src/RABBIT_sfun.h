#ifndef _RABBIT_SFUN_
#define _RABBIT_SFUN_

#define S_FUNCTION_NAME RABBIT_sfun /* Defines and Includes */
#define S_FUNCTION_LEVEL 2

/* #define MOCK // if defined, force use test rabbit interfaces */
/* #define VERBOSE // verbose messages */
/* #define TIMER // if defined, calculate and output execution time */

#define STR1(x)  #x
#define STR(x) STR1(x)
#define SFNAME STR(S_FUNCTION_NAME)

/* General includes */
#include "simstruc.h"
#include <time.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdint.h>

/* Print verbose information or not */
#ifdef VERBOSE 
    #ifdef MATLAB_MEX_FILE
        #pragma message "compile for verbose MEX file"
    #define MEXPRINTF mexPrintf
    #else
    #define MEXPRINTF printf /* Standard Printf, always works */
        #pragma message "compile for verbose build"
    #endif
#else 
#define MEXPRINTF fakePrintf
static void fakePrintf(const char *message, ...) {}
#endif

/* error and warnings for MEX vs realtime version */
#ifdef MATLAB_MEX_FILE
#define MEXWARNING mexWarnMsgIdAndTxt
#define MEXERROR mexErrMsgIdAndTxt
#else
#define MEXWARNING rtWarnMsgIdAndTxt
#define MEXERROR rtErrMsgIdAndTxt
static void rtWarnMsgIdAndTxt(const char *warningid, const char *warningmsg, ...) {printf("WARNING: %s\n",warningid); printf(warningmsg);}
static void rtErrMsgIdAndTxt(const char *errorid, const char *errormsg, ...) {perror(errormsg); exit(1);}
#endif


/* RABBIT includes */
#include "mexreadparams.h" /* parameter parsing functions, from mextools lib */
#include "mexreadparams.c" /* parameter parsing functions, from mextools lib */

#include "RABBIT_aux.h"
#include "RABBIT_aux.c"

#define RABBIT_INIT rabbit_lib_init
#define RABBIT_STEP rabbit_lib_step
#define RABBIT_DEINIT rabbit_lib_deinit

#include "libRabbit.h"  /* function prototypes for RABBIT library */
/* Test case handling */
#ifdef MOCK
#include "libRabbit_mock.c" /* contains mock functions to replace libRabbit */
#pragma message "compile RABBIT wrapper in MOCK mode: with mock library calls"
#endif

/* Set some maxima and minima */

#define NSOURCEMAX 16 /* Maximum number of sources accepted */
#define AMAX 150.0 /* Maxmium atomic number */
#define ZMAX 80.0  /* Maximum charge of any species */
#define AMIN 1.0 /* Maxmium atomic number */
#define ZMIN 1.0  /* Maximum charge of any species */
#define NRHOOUTMAX 101 /* Maximum output rho */
#define NMAXL 151 /* Maximum size of input equilibrium 1D grid */
#define NMAXP 151 /* Maximum size of input kinetic 1D grid */
#define EMAX 100000.0 /* Maximum injection energy for tables */
#define EMIN 0.0 /* Minimum */
#define OUTPUTTIMING 1.0 /* setting for RT-compatible output */
#define NAMELIST "${RABBIT_PATH}/inputs/namelist_used"

/* RABBIT variables, shared across functions */
double aplasma, zplasma, aimp, zimp, output_timing;
double *abeam, *zbeam, *xstart, *xvec, *beamwidthpoly, *R, *Z;
int nv, nsource, nrhotor_out, nl, np;
int m, n, namelist_strlen;
char* namelist;

#endif
