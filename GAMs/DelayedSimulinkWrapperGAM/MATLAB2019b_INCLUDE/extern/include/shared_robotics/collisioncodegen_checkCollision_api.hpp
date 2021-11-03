// Copyright 2018-2019 The MathWorks, Inc.

/**
* @file
* @brief This file defines the exported APIs of the collisioncodegen (kernel collision checking functions) 
*/

#ifndef COLLISIONCODEGEN_CHECKCOLLISION_API_HPP
#define COLLISIONCODEGEN_CHECKCOLLISION_API_HPP

#include "collisioncodegen_CollisionGeometry.hpp"
#include "collisioncodegen_ccdExtensions.hpp"
//#include <memory>


namespace shared_robotics
{

    /**
     * @brief Check collision between @p obj1 and @p obj2 if they are not in collision, also computes the minimal distance and witness points if requested.
     * @param[in] obj1 Pointer to @c CollisionGeometry object 1
     * @param[in] obj2 Pointer to @c CollisionGeometry object 2
     * @param[in] computeDistance An integer indicating whether to compute minimal distance when checking collision. 1 - yes, 0 - no
     * @param[out] p1Vec Pointer to witness point on @p obj1
     * @param[out] p2Vec Pointer to witness point on @p obj2
     * @param[out] distance Minimal distance between @p obj1 and @p obj2, if they are not in collision
     * @return The minimal distance between @p obj1 and @p obj2
     */
    EXTERN_C COLLISIONCODEGEN_API int intersect(const void *obj1, const void *obj2, int computeDistance, ccd_real_t* p1Vec, ccd_real_t* p2Vec, ccd_real_t &distance);



   /**
    * @brief Create a collision geometry as a box primitive (the caller is responsible for freeing the memory pointed by the returned pointer)
    */
   EXTERN_C COLLISIONCODEGEN_API void* makeBox(ccd_real_t x, ccd_real_t y, ccd_real_t z);

   /**
   * @brief Create a collision geometry as a sphere primitive (the caller is responsible for freeing the memory pointed by the returned pointer)
   */
   EXTERN_C COLLISIONCODEGEN_API void* makeSphere(ccd_real_t r);

   /**
    * @brief Update the pose of the given collision geometry
    */    
   EXTERN_C COLLISIONCODEGEN_API void updatePose(void *geom, const ccd_real_t * position, const ccd_real_t * orientation);
}

#endif /* COLLISIONCODEGEN_CHECKCOLLISION_API_HPP */

