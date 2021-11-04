#include "RABBIT_aux.h"
#include <math.h>

static void minmax(const double* array,const int narray,double* minval,double* maxval)
{
  /* Find minimum and maximum of an array */
  int ii;
  *minval = array[0];  *maxval = array[0]; /* init */
  for (ii=0;ii<narray;ii++)
  {
    *minval = fmin(*minval,array[ii]);
    *maxval = fmax(*maxval,array[ii]);
  }
}

/* aux function */
static void mexprintf_double_array(const char* name, const double* array,const char* format,const int narray)
{
  int ii;
  MEXPRINTF("%20s :",name);
  for (ii=0;ii<narray;ii++)
  {
    MEXPRINTF(format,array[ii]);
  }
  MEXPRINTF("\n");
}

/* display functions */
extern void display_rabbit_init_inputs(const double* aplasma, const double* zplasma, 
        const double* aimp, const double* zimp, 
        const double* abeam, const double* zbeam, 
        const double* xstart, const double* xvec, const double* beamwidthpoly, 
        const int* nv, const int* nsource, const int* nrhotor_out, 
        const double* R, const double* Z, 
        const int* m, const int* n, 
        const int* nl, const int* np, 
        const char namelist[], int* namelist_strlen)
{
  mexprintf_double_array("aplasma",aplasma,"%2.2f ",1);
  mexprintf_double_array("zplasma",zplasma,"%2.2f ",1);
  mexprintf_double_array("aimp",aimp,"%2.2f ",1);
  mexprintf_double_array("zimp",zimp,"%2.2f ",1);
  mexprintf_double_array("abeam",abeam,"%2.2f ",*nsource);
  mexprintf_double_array("zbeam",zbeam,"%2.2f ",*nsource);
  mexprintf_double_array("xstart",xstart,"%2.2e ",3*(*nsource));
  mexprintf_double_array("xvec",xvec,"%2.2e ",3*(*nsource));
  mexprintf_double_array("beamwidthpoly",beamwidthpoly,"%2.2e ",3*(*nsource));
  MEXPRINTF("%20s: %d \n","nv",*nv);
  MEXPRINTF("%20s: %d \n","nsource",*nsource);
  MEXPRINTF("%20s: %d \n","nrhotor_out",*nrhotor_out);
  mexprintf_double_array("R",R,"%2.2e ",*m);
  mexprintf_double_array("Z",Z,"%2.2e ",*n);
  MEXPRINTF("%20s: (%d,%d) \n","(m,n)",*m,*n);
  MEXPRINTF("%20s: %d \n","p",*nl);
  MEXPRINTF("%20s: %d \n","l",*np);
  MEXPRINTF("%20s: %s \n","namelist",namelist);
  MEXPRINTF("%20s: %d \n","namelist_strlen",*namelist_strlen);
}

extern void display_rabbit_step_inputs(const double* rhotor_kinprof, const double* ne, 
        const double* Te, const double* Ti, 
        const double* Zeff, const double* omega_tor,
        const int* p, const double* Psi, const double* Psi1d, 
        const double* Volume, const double* Area, const double* rhotor_equil, 
        const double* iota, const double* F, const double* Psi_sep, 
        const double* Psi_axis, const double* Rmag, const double* Zmag, 
        const int* m, const int* n, const int* nl, 
        const double* Pinj, const double* Einj, const double* specmix, 
        const int* nv, const int* nsource, 
        const double* bdens_in, int* nrhoout,  
        const double* dt, const double* output_timing)
{
  double minval, maxval;
  minmax(Psi,(*n)*(*m),&minval,&maxval);
  
  MEXPRINTF("  1D kinetic profile quantities, size %d\n",*p);
  mexprintf_double_array("rhotor_kinprof",rhotor_kinprof,"%2.2e ",*p);
  mexprintf_double_array("ne",ne,"%2.2e ",*p);
  mexprintf_double_array("Te",Te,"%2.2e ",*p);
  mexprintf_double_array("Ti",Ti,"%2.2e ",*p);
  mexprintf_double_array("Zeff",Zeff,"%2.2e ",*p);
  mexprintf_double_array("omega_tor",omega_tor,"%2.2e ",*p);
  MEXPRINTF("  Equilibrium quantities, 1D size %d\n",*nl);
  MEXPRINTF("%20s: not printed due to size issues, size: [%d x %d]. First 3 values: %f,%f,%f. (max,min) = (%2.2e,%2.2e) \n","Psi",
          Psi[0],Psi[1],Psi[2],*m,*n,minval,maxval);
  mexprintf_double_array("Psi1d",Psi1d,"%2.2e ",*nl);
  mexprintf_double_array("Volume",Volume,"%2.2e ",*nl);
  mexprintf_double_array("Area",Area,"%2.2e ",*nl);
  mexprintf_double_array("rhotor_equil",rhotor_equil,"%2.2e ",*nl);
  mexprintf_double_array("F",F,"%2.2e ",*nl);
  mexprintf_double_array("iota",iota,"%2.2e ",*nl);
  MEXPRINTF("  Other inputs: \n");
  MEXPRINTF("%20s: %d \n","nsource",*nsource);
  MEXPRINTF("%20s: %d \n","nv",*nv);

  mexprintf_double_array("Pinj",Pinj,"%2.2e ",*nsource);
  mexprintf_double_array("Einj",Einj,"%2.2e ",*nsource);
  mexprintf_double_array("specmix",specmix,"%2.2e ",(*nsource)*(*nv));
  mexprintf_double_array("bdens_in",bdens_in,"%2.2e ",*nrhoout);
  MEXPRINTF("%20s: %2.2e \n","dt",*dt);
  MEXPRINTF("%20s: %2.2e \n","output_timing",*output_timing);
}

extern void  display_rabbit_step_outputs(const double* powe, const double* powi, const double* press,
          const double* bdep, const double* bdens, const double* jfi, const double* jnbcd, 
          const double* torqe, const double* torqi, const double* torqimp, const double* nrate,const double* rhotor_out,
          const int* nrhotor_out,const double* powe_total,const double* powi_total,
          const double* Pshine,const double* Prot,const double* Porbitloss,const double* Pcxloss,
          const double* Inb_total,const int* ierr, const int* nsource)
{
  int ii;
  int offset;
  for (ii=0;ii<*nsource;ii++)
  {
  offset = ii* (*nrhotor_out);
  MEXPRINTF("Source number: %d/%d\n",ii+1,*nsource);
  MEXPRINTF("  1D output profiles, size %d\n",*nrhotor_out);
  mexprintf_double_array("powe",powe + offset,"%2.2e ",*nrhotor_out);
  mexprintf_double_array("powi",powi + offset,"%2.2e ",*nrhotor_out);
  mexprintf_double_array("press",press + offset,"%2.2e ",*nrhotor_out);
  mexprintf_double_array("bdep",bdep + offset,"%2.2e ",*nrhotor_out);
  mexprintf_double_array("bdens",bdens + offset,"%2.2e ",*nrhotor_out);
  mexprintf_double_array("jfi",jfi + offset,"%2.2e ",*nrhotor_out);
  mexprintf_double_array("jnbcd",jnbcd + offset,"%2.2e ",*nrhotor_out);
  mexprintf_double_array("torqe",torqe + offset,"%2.2e ",*nrhotor_out);
  mexprintf_double_array("torqimp",torqi + offset,"%2.2e ",*nrhotor_out);
  mexprintf_double_array("torqi",torqimp + offset,"%2.2e ",*nrhotor_out);
  MEXPRINTF("  Scalars:\n");
  MEXPRINTF("%20s: %2.2e \n","Pshine",*Pshine);
  MEXPRINTF("%20s: %2.2e \n","Prot",*Prot);
  MEXPRINTF("%20s: %2.2e \n","Porbitloss",*Porbitloss);
  MEXPRINTF("%20s: %2.2e \n","Pcxloss",*Pcxloss);
  MEXPRINTF("%20s: %2.2e \n","Inb_total",*Inb_total);
  MEXPRINTF("%20s: %d \n","ierr",*ierr);
  MEXPRINTF("\n");
  }
  MEXPRINTF("  Totals:\n");
  mexprintf_double_array("nrate",nrate,"%2.2e ",*nrhotor_out);
  mexprintf_double_array("rhotor_out",rhotor_out,"%2.2e ",*nrhotor_out);
  MEXPRINTF("\n");
}
