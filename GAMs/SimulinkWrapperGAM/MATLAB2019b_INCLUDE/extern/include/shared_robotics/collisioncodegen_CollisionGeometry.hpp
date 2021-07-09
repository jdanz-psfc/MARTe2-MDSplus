// Copyright 2018-2019 The MathWorks, Inc.

/**
 * @file
 * @brief Interface for collision geometry representation based on support mapping
 */

#ifndef COLLISIONCODEGEN_COLLISIONGEOMETRY_HPP
#define COLLISIONCODEGEN_COLLISIONGEOMETRY_HPP

    #include <ccd/vec3.h> // need to use some data type definition 
    #include <Eigen/Core>
    #include "collisioncodegen_util.hpp"
    #include <Eigen/Geometry>
    #include <iostream>
    #include <vector>


    namespace shared_robotics 
    {
		class CollisionGeometry;
		struct CollisionGeometryDeleter
		{
			void operator ()(CollisionGeometry* ptr);
		};

        /**
        * @brief The @c CollisionGeometry class. 
        * @details A collision geometry can be specified as a primitive (box, cylinder or sphere)
        *          or a convex mesh (vertices only, or vertices and faces). ccd algorithm interacts
        *          with a collision geometry through its support function
        */
        class COLLISIONCODEGEN_API CollisionGeometry
        {
        public:
    
            /// enum for collision geometry types
            enum Type
            {
                Box,
                Sphere,
                Cylinder,
                ConvexMesh,
                ConvexMeshFull
            };    

            /// destructor
			virtual ~CollisionGeometry();
    
            /// constructor for box primitive 
            CollisionGeometry(ccd_real_t x, ccd_real_t y, ccd_real_t z): m_x(x), m_y(y), m_z(z)
            {
                m_type = Type::Box; 
            }
    
            /// constructor for sphere primitive
            CollisionGeometry(ccd_real_t radius): m_radius(radius)
            {
                m_type = Type::Sphere; 
            }
    
            /// constructor for cylinder primitive
            CollisionGeometry(ccd_real_t radius, ccd_real_t height): m_radius(radius), m_height(height)
            {
                 m_type = Type::Cylinder;
            }

            /// constructor for convex mesh (vertices only)
            CollisionGeometry(ccd_real_t * vertices, int numVertices, bool isColumnMajor = true)
            {
                m_type = Type::ConvexMesh;
                m_numVertices = numVertices;

                if (isColumnMajor)
                {
                    m_vertices = Eigen::Map<MatrixX3F>(vertices, numVertices, 3);
                }
                else
                {
                    MatrixXF verticesPrime = Eigen::Map<MatrixXF>(vertices, 3, numVertices);
                    verticesPrime.transposeInPlace();
                    m_vertices = Eigen::Map<MatrixX3F>(verticesPrime.data(), numVertices, 3);
                    //std::cout << m_vertices << std::endl;
                }

            }

            /// constructor for convex mesh (vertices and faces)
            CollisionGeometry(ccd_real_t * vertices, int * faces, int numVertices, int numFaces, bool isColumnMajor = true)
            {
                m_type = Type::ConvexMeshFull;
                m_numVertices = numVertices;
                m_numFaces = numFaces;

                if (isColumnMajor)
                {
                    m_vertices = Eigen::Map<MatrixX3F>(vertices, numVertices, 3);
                    m_faces = Eigen::Map<Eigen::MatrixX3i>(faces, numFaces, 3);
                }
                else
                {
                    MatrixXF verticesPrime = Eigen::Map<MatrixXF>(vertices, 3, numVertices);
                    verticesPrime.transposeInPlace();
                    m_vertices = Eigen::Map<MatrixX3F>(verticesPrime.data(), numVertices, 3);

                    Eigen::MatrixXi facesPrime = Eigen::Map<Eigen::MatrixXi>(faces, 3, numFaces);
                    facesPrime.transposeInPlace();
                    m_faces = Eigen::Map<Eigen::MatrixX3i>(facesPrime.data(), numFaces, 3);
                }

                //std::cout << m_vertices << std::endl;
                //std::cout << m_faces << std::endl;

                constructConnectionList();
            }
            
    
            /// query the type of the collision geometry
            std::string getType() const;
    
            /// support function
            void support(const Vector3F& dir,
                         const Vector3F& pos,          // position of the geometry
                         const QuaternionF& quat,      // orientation of the geometry
                         Vector3F& supportVertex) const;

            /// support function for libccd interface
            void support(const ccd_vec3_t* dir,
                         ccd_vec3_t* supportVertex) const;

        protected:
            /// sign function
            int sign(ccd_real_t val) const;

            /// must only be invoked after m_vertices and m_faces are initialized
            void constructConnectionList();
    
            /// type of the geometry
            Type m_type; 
    
    
            /// dimension x of the box, if the geometry is a box primitive 
            ccd_real_t m_x;

            /// dimension x of the box, if the geometry is a box primitive
            ccd_real_t m_y;

            /// dimension x of the box, if the geometry is a box primitive
            ccd_real_t m_z;
    
        
            /// radius of the sphere, if the geometry is a sphere primitive, or radius of the bottom of a cylinder, if the geometry is a cylinder primitive
            ccd_real_t m_radius;

    
            /// height of the cylinder, if the geometry is a cylinder primitive 
            ccd_real_t m_height;


            /// number of vertices in the convex mesh, if the geometry is a convex primitive 
            int m_numVertices;

            /// vertices of the convex mesh, as an m_numVertices-by-3 matrix, if the geometry is a convex mesh
            MatrixX3F m_vertices;


            /// number of faces of the convex mesh, if the geometry is a convex primitive 
            int m_numFaces;

            /// faces of the convex mesh, as an m_numFaces-by-3 matrix consisting of node indices, if the geometry is a convex primitive 
            Eigen::MatrixX3i m_faces;

            ///
            std::vector<std::vector<int>> m_connectionList;

        public:

            /// position of the geometry, used for ccd compatibility
            Vector3F m_pos = Vector3F::Zero();

            /// orientation of the geometry, used for ccd compatibility
            QuaternionF m_quat = QuaternionF::Identity();
        }; 

        /// sign function
        inline int shared_robotics::CollisionGeometry::sign(ccd_real_t val) const
        {
    
            if (CCD_FABS(val) < FLT_EPSILON)
            {
                return 0;
            }
            else if (val < 0.0)
            {
                return -1;
            }
            return 1;
        }

        inline std::string shared_robotics::CollisionGeometry::getType() const
        {
            switch(m_type)
            {
                case Type::Box:
                    return "Box";
                case Type::Sphere:
                    return "Sphere";
                case Type::Cylinder:
                    return "Cylinder";
                case Type::ConvexMeshFull:
                    return "ConvexMeshFull";
                default:
                    return "ConvexMesh";
            }
        }

    } // namespace shared_robotics   
#endif