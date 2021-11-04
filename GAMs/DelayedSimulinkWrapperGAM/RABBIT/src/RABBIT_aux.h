#ifndef _RABBIT_AUX_
#define _RABBIT_AUX_

/* aux function */
static void mexprintf_double_array(const char* name, const double* array,const char* format,const int narray);

/* display functions */
extern void display_rabbit_init_inputs(const double* aplasma, const double* zplasma, 
        const double* aimp, const double* zimp, 
        const double* abeam, const double* zbeam, 
        const double* xstart, const double* xvec, const double* beamwidthpoly, 
        const int* nv, const int* nsource, const int* nrhotor_out, 
        const double* R, const double* Z, 
        const int* m, const int* n, 
        const int* l, const int* p, 
        const char namelist[], int* namelist_strlen);

extern void display_rabbit_step_inputs(const double* rhotor_kinprof, const double* ne, 
        const double* Te, const double* Ti, 
        const double* Zeff, const double* omega_tor,
        const int* p, const double* Psi, const double* Psi1d, 
        const double* Volume, const double* Area, const double* rhotor_equil, 
        const double* iota, const double* F, const double* Psi_sep, 
        const double* Psi_axis, const double* Rmag, const double* Zmag, 
        const int* m, const int* n, const int* l, 
        const double* Pinj, const double* Einj, const double* specmix, 
        const int* nv, const int* nsource, 
        const double* bdens_in,  int* nrhoout,  
        const double* dt, const double* output_timing);

#endif
