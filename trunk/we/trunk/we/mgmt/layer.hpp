/*
 * =====================================================================================
 *
 *       Filename:  layer.hpp
 *
 *    Description:  workflow engine management layer
 *
 *        Version:  1.0
 *        Created:  02/25/2010 12:51:21 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef WE_MGMT_LAYER_HPP
#define WE_MGMT_LAYER_HPP 1

#include <we/util/warnings.hpp>
#include <we/concurrent/net.hpp>

#include <boost/random.hpp>
#include <boost/function.hpp>
#include <boost/functional/hash.hpp>
#include <boost/thread.hpp>

#include <we/mgmt/bits/types.hpp>
#include <we/mgmt/parser.hpp>

namespace we { namespace mgmt {
  namespace detail {
	namespace def { // defaults
	template <typename Net>
	struct net_traits
	{
	};

	template <typename IdType=unsigned long>
	struct id_traits
	{
	public:
	  typedef IdType id_type;

	  static id_type next()
	  {
		static id_type _id = zero();
		return _id++;
	  }

	  static id_type zero()
	  {
		return 0;
	  }
	};

	template <typename StatusType=int>
	struct status_traits
	{
	  typedef StatusType value_type;

	  static value_type RUNNING() { return 0; }
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
		net_type n;
		return n;
	  }

	  static encode_type encode(const decode_type & data)
	  {
		we::util::remove_unused_variable_warning(data);
		bytes_type b;
		return b;
	  }
	};

	template <typename Net>
	struct result_traits
	{
	  typedef std::string value_type;
	};

	} // namespace def

	template <typename Exec, typename Net>
	struct layer_traits
	{
	  typedef def::status_traits<int> status_traits;
	  typedef def::id_traits<unsigned long> id_traits;
	  typedef def::net_traits<Net> net_traits;
	  typedef def::codec<Net, std::string> codec_type;
	  typedef def::result_traits<Net> result_traits;
	};
  }

  template <typename ExecutionLayer
		  , typename Net
		  , typename Traits = detail::layer_traits<ExecutionLayer, Net>
		  , std::size_t NUM_EXTRACTOR=1, std::size_t NUM_INJECTOR=1>
  class layer
  {
  public:
	typedef ExecutionLayer exec_layer_type;
	typedef Net net_type;
	typedef Traits traits_type;

	typedef typename traits_type::net_traits net_traits;

	typedef typename traits_type::id_traits  id_traits;
	typedef typename id_traits::id_type id_type;

	typedef typename traits_type::status_traits status_traits;
	typedef typename status_traits::value_type status_type;

	typedef typename traits_type::result_traits result_traits;
	typedef typename result_traits::value_type result_type;

	typedef typename traits_type::codec_type codec_type;

	typedef typename net_type::place_type place_type;
	typedef typename net_type::edge_type edge_type;
	typedef typename net_type::transition_type transition_type;
	typedef typename net_type::token_type token_type;

	/******************************
	 * EXTERNAL API
	 *****************************/
	id_type submit(const typename codec_type::encode_type & bytes)
	{
	  net_type n = codec_type::decode(bytes);
	  
	  return id_traits::next();
	}

	template <typename ReasonType>
	bool cancel(const id_type & id, const ReasonType & reason)
	{
	  we::util::remove_unused_variable_warning(id);
	  we::util::remove_unused_variable_warning(reason);
	  return true;
	}

	bool finished(const id_type & id, const result_type & result)
	{
	  we::util::remove_unused_variable_warning(id);
	  we::util::remove_unused_variable_warning(result);
	  return true;
	}

	bool failed(const id_type & id, const result_type & result)
	{
	  we::util::remove_unused_variable_warning(id);
	  we::util::remove_unused_variable_warning(result);
	  return true;
	}

	status_type status(const id_type & id)
	{
	  we::util::remove_unused_variable_warning(id);
	  return status_traits::RUNNING();
	}
  };
}}

#endif
