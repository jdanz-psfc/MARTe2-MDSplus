// Copyright 2018-2019 The MathWorks, Inc.

/**
 * @file
 * @brief Provide additional typedefs needed for collisioncodegen 
 */

#ifndef COLLISIONCODEGEN_UTIL_HPP
#define COLLISIONCODEGEN_UTIL_HPP

#include <ccd/vec3.h>
#include <Eigen/Core>

#if defined(BUILDING_LIBMWCOLLISIONCODEGEN) // should be defined by the mw build infrastructure
/* For DLL_EXPORT_SYM and EXTERN_C */
#include "package.h"

#define COLLISIONCODEGEN_API DLL_EXPORT_SYM

#else

#ifndef COLLISIONCODEGEN_API
#define COLLISIONCODEGEN_API
#endif

#endif /* else */


#ifndef EXTERN_C
#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif
#endif


namespace shared_robotics 
{
    typedef Eigen::Matrix<ccd_real_t, 3, 1> Vector3F;
    typedef Eigen::Quaternion<ccd_real_t> QuaternionF;
    typedef Eigen::Matrix<ccd_real_t, Eigen::Dynamic, 3> MatrixX3F;
    typedef Eigen::Matrix<ccd_real_t, Eigen::Dynamic, Eigen::Dynamic> MatrixXF;
} // namespace shared_robotics   

#endif /* COLLISIONCODEGEN_UTIL_HPP_ */
