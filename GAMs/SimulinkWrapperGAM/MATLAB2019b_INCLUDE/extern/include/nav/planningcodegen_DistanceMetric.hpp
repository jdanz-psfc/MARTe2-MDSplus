// Copyright 2019 The MathWorks, Inc.

/**
* @file
* @brief Distance metric between two states
*/


#ifndef PLANNINGCODEGEN_DISTANCEMETRIC_HPP
#define PLANNINGCODEGEN_DISTANCEMETRIC_HPP

#include <vector>
#include <iostream>
#include <cmath>

namespace nav
{
    template <typename T>
    class DistanceMetric
    {
    public:
        DistanceMetric() {};
        virtual ~DistanceMetric() {};

        virtual std::vector<T> distance(const std::vector<T>& queryState, const std::vector<T>& states) = 0;
        
    protected:
        std::size_t m_dim;

    };

    template <typename T>
    class EuclideanMetric : public DistanceMetric<T>
    {
    public:
        EuclideanMetric(std::size_t dim)
        {
            this->m_dim = dim;
        }

        std::vector<T> distance(const std::vector<T>& queryState, const std::vector<T>& states)
        {
            std::vector<T> dists{};
            
            std::size_t numStates = states.size() / this->m_dim;
            typename std::vector<T>::const_iterator queryIt = queryState.begin();
            typename std::vector<T>::const_iterator statesIt = states.begin();

            for (std::size_t i = 0; i < numStates; i++)
            {
                dists.push_back(distanceInternal(queryIt, statesIt));
                statesIt += this->m_dim;
            }
            return dists;
        }
    protected:
        T distanceInternal(const typename std::vector<T>::const_iterator& state1It, const typename std::vector<T>::const_iterator& state2It)
        {
            T sum{};
            for (size_t i = 0; i < this->m_dim; i++)
            {
                sum += ( *(state2It + i) - *(state1It + i)) * ( *(state2It + i) - *(state1It + i));
            }
            return std::sqrt(sum);
        }
    };
}

#endif
