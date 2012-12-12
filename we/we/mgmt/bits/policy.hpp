/*
 * =====================================================================================
 *
 *       Filename:  policy.hpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  03/04/2010 06:12:52 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef WE_MGMT_LAYER_BITS_POLICY_HPP
#define WE_MGMT_LAYER_BITS_POLICY_HPP 1

#include <string>
#include <we/we-config.hpp>
#include <we/util/codec.hpp>
#include <we/mgmt/bits/execution_policy.hpp>
#include <we/mgmt/exception.hpp>

namespace we
{
  namespace mgmt
  {
    namespace policy
    {
      struct layer_policy
      {
        typedef execution_policy<we::mgmt::type::activity_t> exec_policy;

        static const size_t NUM_EXTRACTORS = WE_NUM_EXTRACTORS;
        static const size_t NUM_INJECTORS  = WE_NUM_INJECTORS;
      };
    }
  }
}

#endif
