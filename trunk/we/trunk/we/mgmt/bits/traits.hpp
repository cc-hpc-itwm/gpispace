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

#include <we/mgmt/parser.hpp>

namespace we { namespace mgmt { namespace detail {
  namespace def {
	template <typename Net, bool default_value = true>
	  struct basic_net_validator
	  {
		typedef Net net_type;

		static bool is_valid(const net_type & n)
		{
		  we::util::remove_unused_variable_warning(n);
		  return default_value;
		}
	  };

	template <typename Net>
	  struct net_traits
	  {
		typedef Net type;
		typedef basic_net_validator<Net> validator_type;
	  };

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
	  struct id_traits<long unsigned int>
	  {
		public:
		  typedef long unsigned int type;

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

	template <typename StatusType=int>
	  struct status_traits
	  {
		typedef StatusType type;

		static type RUNNING() { return 0; }
	  };

	template <typename ReasonType>
	  struct reason_traits
	  {
		typedef ReasonType type;

		static type USER_CANCEL();
		static type SYSTEM_CANCEL();
	  };
	template <>
	  struct reason_traits<std::string>
	  {
		typedef std::string type;

		static const type USER_CANCEL() { return "USER_CANCEL"; }
		static const type SYSTEM_CANCEL() { return "SYSTEM_CANCEL"; }
	  };

	template <typename Net, typename BytesType=std::string>
	  struct codec
	  {
		typedef BytesType bytes_type;
		typedef Net net_type;

		typedef bytes_type encode_type;
		typedef net_type decode_type;

		static decode_type decode(const encode_type & data)
		{
		  we::util::remove_unused_variable_warning(data);
		  decode_type n;
		  parser<decode_type>::parse(n, data);
		  return n;
		}

		static encode_type encode(const decode_type & data)
		{
		  we::util::remove_unused_variable_warning(data);
		  bytes_type b;
		  return b;
		}
	  };

	template <typename T>
	  struct result_traits
	  {
		typedef T type;
	  };
  }

  template <typename Net, typename IdType>
	struct layer_traits
	{
	  typedef def::id_traits<IdType> id_traits;
	  typedef def::status_traits<int> status_traits;
	  typedef def::net_traits<Net> net_traits;
	  typedef def::result_traits<std::string> result_traits;
	  typedef def::reason_traits<std::string> reason_traits;
	};
}}}

#endif
