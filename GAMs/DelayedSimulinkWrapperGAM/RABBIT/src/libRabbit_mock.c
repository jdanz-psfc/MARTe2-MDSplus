#include <math.h>
/* Test Functions */

extern void rabbit_lib_init( 
        const double* asplasma, const double* zplasma, 
        const double* aimp, const double* zimp, 
        const double* abeam, const double* zbeam, 
        const double* xstart, const double* xvec, const double* beamwidthpoly, 
        const int* nv, const int* nsource, const int* nrhotor_out, 
        const double* R, const double* Z, 
        const int* m, const int* n, 
        const int* l, const int* p, 
        const char namelist[], int* namelist_strlen, 
        int* ierr)
{
  int ii;
  MEXPRINTF("\n***Called mock version of RABBIT_init for testing***\n");
  
   for(ii=0; ii<*nsource; ii++)
   {
     ierr[ii] = 0;
   }
  return;
}

extern void rabbit_lib_step(const double* rhotor_kinprof, const double* ne, 
        const double* Te, const double* Ti, 
        const double* Zeff, const double* omega_tor,
        const int* p, const double* Psi, const double* Psi1d, 
        const double* Volume, const double* Area, const double* rhotor_equil, 
        const double* iota, const double* F, const double* Psi_sep, 
        const double* Psi_axis, const double* Rmag, const double* Zmag, 
        const int* m, const int* n, const int* l, 
        const double* Pinj, const double* Einj, const double* specmix, 
        const int* nv, const int* nsource, const double* bdens_in, 
        const double* dt, const double* output_timing, 
        /* output signals below: */
        double* powe, double* powi, double* press, 
        double* bdep, double* bdens, double* jfi, 
        double* jnbcd, double* torqe, double* torqi, double* torqimp, double* nrate, 
        double* rhotor_out, const int* nrhoout,
        double* powe_total, double* powi_total, 
        double* Pshine, double* Prot, double* Porbitloss, double* Pcxloss,  
        double* Inb_total,
        int* ierr,
        double* rhotor2   /* rhotor2 is optional */
)
{
  MEXPRINTF("***Called mock version of RABBIT_step for testing***\n");
  
  int ii, jj, ij;
  int nrho = *nrhoout;
  int nsrc = nsource[0];
  
  /* Display inputs */
  
  MEXPRINTF("Assigning dummy values\n");
  
  for (ii=0;ii<(*nrhoout);ii++)
  {
    for (jj=0;jj<(*nsource);jj++)
    {
    ij = (ii+jj*(*nrhoout));
    powe[ij] = Pinj[jj]*(double)(ii*(jj+1));
    powi[ij] = Pinj[jj]*(double)(ii*(jj+1));
    press[ij] = 0.0;
    bdep[ij] = 0.0;
    bdens[ij] = 1.0;
    jfi[ij] = 0.0;
    jnbcd[ij] = 10.0;
    torqe[ij] = -1.0;
    torqi[ij] = -2.0;
    torqimp[ij] = -3.0;
    }
    nrate[ii] = 0.0;
    rhotor_out[ii] = (double)ii/(nrho-1);
  }
  
  for (jj=0;jj<(*nsource);jj++)
  {
  powe_total[jj] = 1.0*(double)(jj+1);
  powi_total[jj] = 1.0;
  Pshine[jj] = 0.0;
  Pcxloss[jj] = -2.0;
  Prot[jj] = 3.0;
  Porbitloss[jj] = -10.0;
  Inb_total[jj] = 1000.0;
  ierr[jj] = 0;
  }
  return;
}

extern void rabbit_lib_deinit(const int* nsource, 
        int* ierr)
{
  int ii;
  
  for (ii=0;ii<nsource[0];ii++)
  {
    ierr[ii] = 0;
  } 
  MEXPRINTF("Called mock version of RABBIT_deinit for testing\n");
  return;
}



    extern void rabbit_lib_get_dV_dArea(
            double* dV,
            double* dArea,
            const int32_t* nrhoout){}

    extern void rabbit_lib_set_dump_dir(char* dir_in, int32_t* dir_strlen){}

    extern void rabbit_lib_dump_beams(char* dir_in, int32_t* dir_strlen, 
				      double* Einj, double* specmix){}
