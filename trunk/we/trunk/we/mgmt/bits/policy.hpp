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
#include <we/mgmt/bits/execution_policy.hpp>

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
            if (! valid)
            {
              throw std::runtime_error ("validator<false>:: forced validation error!");
            }
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
        typedef def::validator<typename Traits::activity_type, true> validator;
        typedef execution_policy<typename Traits::activity_type> exec_policy;

        static const size_t DEFAULT_COMMAND_QUEUE_SIZE = 1024;
        static const size_t DEFAULT_ACTIVE_NETS_SIZE = 1024;
        static const size_t DEFAULT_INJECTOR_QUEUE_SIZE = 1024;
        static const size_t DEFAULT_EXECUTOR_QUEUE_SIZE = 1024;

        static size_t max_command_queue_size ()
        {
          return DEFAULT_COMMAND_QUEUE_SIZE;
        }
        static size_t max_active_nets ()
        {
          return DEFAULT_ACTIVE_NETS_SIZE;
        }
        static size_t max_injector_queue_size ()
        {
          return DEFAULT_INJECTOR_QUEUE_SIZE;
        }
        static size_t max_executor_queue_size ()
        {
          return DEFAULT_EXECUTOR_QUEUE_SIZE;
        }
      };
    }
  }
}

#endif
