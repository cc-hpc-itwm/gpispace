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
#include <we/util/codec.hpp>

namespace we
{
  namespace mgmt
  {
    namespace policy
    {
      namespace def
      {
        template <typename T, typename BaseCodec = ::we::util::text_codec>
        struct codec
        {
          typedef T type;
          typedef BaseCodec base_codec;
          typedef std::string bytes_type;

          static type decode(const bytes_type & data)
          {
            return base_codec::template decode<T>(data);
          }

          static bytes_type encode(type const & thing)
          {
            return base_codec::template encode<T>(thing);
          }
        };

        template <typename T, bool valid=true>
        struct validator
        {
          typedef T type;
          static void validate (T const &) throw (std::exception)
          {
            return valid;
          };
          static bool is_valid (T const & t)
          {
            try
            {
              validate (t);
            }
            catch (...)
            {
              return false;
            }
            return true;
          }
        };
      }

      template <typename Traits>
      struct layer_policy
      {
        typedef def::codec<typename Traits::activity_type> codec;
        typedef def::validator<Activity, true> validator;
      };
    }
  }
}

#endif
