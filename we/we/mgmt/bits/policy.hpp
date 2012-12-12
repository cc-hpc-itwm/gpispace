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
      namespace def
      {
        template <typename T, typename BaseCodec = ::we::util::codec>
        struct codec
        {
          typedef T type;
          typedef BaseCodec base_codec;
          typedef std::string bytes_type;

          static type decode(const bytes_type & data)
          {
            return base_codec::template decode<T>(data);
          }

          static type decode(std::istream & is)
          {
            return base_codec::template decode<T>(is);
          }

          static bytes_type encode(type const & thing)
          {
            return base_codec::template encode<T>(thing);
          }

          static void encode (std::ostream & os, type const & thing)
          {
            base_codec::template encode<T>(os, thing);
          }
        };

        template <typename T, bool valid=true>
        struct validator
        {
          typedef T type;
          static void validate (T const &)
          {
            if (! valid)
            {
              throw exception::validation_error ("validator<false>:: forced validation error!");
            }
          };

          static bool is_valid (T const & t)
          {
            try
            {
              validate (t);
            }
            catch (exception::validation_error const &)
            {
              return false;
            }
            return true;
          }
        };
      }

      struct layer_policy
      {
        typedef def::codec<we::mgmt::type::activity_t> codec;
        typedef def::validator<we::mgmt::type::activity_t, true> validator;
        typedef execution_policy<we::mgmt::type::activity_t> exec_policy;

        static const size_t NUM_EXTRACTORS = WE_NUM_EXTRACTORS;
        static const size_t NUM_INJECTORS  = WE_NUM_INJECTORS;

        static const size_t COMMAND_QUEUE_SIZE   = 0;
        static const size_t INJECTOR_QUEUE_SIZE  = 0;
        static const size_t EXTRACTOR_QUEUE_SIZE = 0;
      };
    }
  }
}

#endif
