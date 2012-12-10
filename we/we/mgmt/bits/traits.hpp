/*
 * =====================================================================================
 *
 *       Filename:  traits.hpp
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  03/03/2010 04:16:58 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef WE_MGMT_BITS_TRAITS_HPP
#define WE_MGMT_BITS_TRAITS_HPP 1

#include <string>
#include <limits>

#include <we/type/id.hpp>
#include <we/util/codec.hpp>

namespace we
{
  namespace mgmt
  {
    namespace traits
    {
      namespace def
      {
        template <typename IdType>
        struct id_traits
        {
          typedef IdType type;
          //		must be specialized
          static const type nil() { return std::numeric_limits<type>::max(); }
        };

        template <>
        struct id_traits<std::string>
        {
          typedef std::string type;

          inline static const type nil() { return ""; }
        };

        template <>
        struct id_traits<petri_net::place_id_type>
        {
        public:
          typedef petri_net::place_id_type type;

          inline static type nil()
          {
            return std::numeric_limits<type>::max();
          }

          inline static type generate()
          {
            static type _id = init();
            return _id++;
          }
        private:
          inline static type init()
          {
            return 0;
          }
        };

        template <typename T>
        struct result_traits
        {
          typedef T type;
        };
      }

      template <typename Activity>
      struct layer_traits
      {
        typedef Activity activity_type;
        typedef def::id_traits<typename Activity::id_t> id_traits;
      };
    }
  }
}

#endif
