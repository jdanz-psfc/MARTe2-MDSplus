#include <stdint.h>

extern void rabbit_lib_init(
        const double* aplasma, const double* zplasma,
        const double* aimp, const double* zimp,
        const double* abeam, const double* zbeam,
        const double* xstart, const double* xvec, const double* beamwidthpoly,
        const int* nv, const int* nsource, const int* nrhotor_out,
        const double* R, const double* Z,
        const int* m, const int* n,
        const int* l, const int* p,
        const char namelist[], int* namelist_strlen,
        int* ierr
        );

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
        double* Powe_total, double* Powi_total,
        double* Pshine, double* Prot, double* Porbitloss, double* Pcxloss,
        double* Inb_total,
        int* ierr,
        double* rhotor2   /* rhotor2 is an additional optional input */
        );

extern void rabbit_lib_deinit(
        const int* nbeams,
        int* ierr);


    extern void rabbit_lib_get_dV_dArea(
            double* dV,
            double* dArea,
            const int32_t* nrhoout);

    extern void rabbit_lib_set_dump_dir(char* dir_in, int32_t* dir_strlen);

    extern void rabbit_lib_dump_beams(char* dir_in, int32_t* dir_strlen, 
				      double* Einj, double* specmix);
