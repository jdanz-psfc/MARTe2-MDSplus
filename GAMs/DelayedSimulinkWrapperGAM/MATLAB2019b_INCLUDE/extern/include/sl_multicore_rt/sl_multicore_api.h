/* Copyright 2018 The MathWorks, Inc. */

#ifndef sl_multicore_api_h
#define sl_multicore_api_h

#ifdef SUPPORTS_PRAGMA_ONCE
#pragma once
#endif

#ifdef BUILDING_LIBMWSLMULTICORE_RT

# define SLMULTICORE_RT_EXPORT_EXTERN_C            extern "C" DLL_EXPORT_SYM
#elif defined(DLL_IMPORT_SYM)

# define SLMULTICORE_RT_EXPORT_EXTERN_C            extern "C" DLL_IMPORT_SYM
#else

# ifdef __cplusplus
#  define SLMULTICORE_RT_EXPORT_EXTERN_C extern "C"
# else
#  define SLMULTICORE_RT_EXPORT_EXTERN_C extern
# endif
#endif

SLMULTICORE_RT_EXPORT_EXTERN_C void* slmcrtCreateSectionProfiles(int sectionCount,
                                                                   const char* checksum,
                                                                   const char* cachePath);
SLMULTICORE_RT_EXPORT_EXTERN_C void slmcrtStartProfiling(void* opaqueSectionProfiles,
                                                           int sectionNumber);
SLMULTICORE_RT_EXPORT_EXTERN_C void slmcrtStopProfiling(void* opaqueSectionProfiles,
                                                          int sectionNumber);
SLMULTICORE_RT_EXPORT_EXTERN_C void slmcrtWriteProfileDataToMat(void* opaqueSectionProfiles);
SLMULTICORE_RT_EXPORT_EXTERN_C void slmcrtDestroySectionProfiles(void* opaqueSectionProfiles);

#endif
