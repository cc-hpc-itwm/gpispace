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

#include <cassert>

#include <we/util/warnings.hpp>
#include <we/concurrent/net.hpp>

#include <boost/random.hpp>
#include <boost/function.hpp>
#include <boost/functional/hash.hpp>
#include <boost/thread.hpp>

#include <we/mgmt/bits/types.hpp>
#include <we/mgmt/basic_layer.hpp>
#include <we/mgmt/parser.hpp>

namespace we { namespace mgmt {
  namespace detail {
	namespace def { // defaults

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
		typedef basic_net_validator<Net> validator_type;
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
	  typedef std::string reason_type;
	};
  }

  template <typename ExecutionLayer
		  , typename Net
		  , typename Traits = detail::layer_traits<ExecutionLayer, Net>
		  , std::size_t NUM_EXTRACTOR=1, std::size_t NUM_INJECTOR=1>
  class layer : public basic_layer
  {
  public:
	typedef ExecutionLayer exec_layer_type;
	typedef Net net_type;
	typedef Traits traits_type;

	typedef typename traits_type::net_traits net_traits;
	typedef typename net_traits::validator_type net_validator;

	typedef typename traits_type::id_traits  id_traits;
	typedef typename id_traits::id_type id_type;

	typedef typename traits_type::status_traits status_traits;
	typedef typename status_traits::value_type status_type;

	typedef typename traits_type::result_traits result_traits;
	typedef typename result_traits::value_type result_type;

	typedef typename traits_type::codec_type codec_type;
	typedef typename traits_type::reason_type reason_type;

	typedef typename net_type::place_type place_type;
	typedef typename net_type::edge_type edge_type;
	typedef typename net_type::transition_type transition_type;
	typedef typename net_type::token_type token_type;


	/******************************
	 * EXTERNAL API
	 *****************************/

	/** 
	 * Submit a new petri net to the petri-net management layer
	 *
	 *	  pre-conditions: none
	 *
	 *	  side-effects: parses the passed data
	 *					registeres the petri-net with the mgmt layer
	 *					returns a newly generated id
	 *
	 *	  post-conditions: the net is registered is with id "id"
	 *
	 */
	id_type submit(const id_type & id, const typename codec_type::encode_type & bytes)
	{
	  net_type n = codec_type::decode(bytes);

	  // check network for validity
	  if (net_validator::is_valid(n))
	  {
		 std::cerr << "D: submitted petri-net["<< id << "]" << std::endl;
		 exec_layer_.submit(id, bytes);
		 return id;
	  }
	  else
	  {
		// TODO: detailed error analysis
		throw std::runtime_error("network invalid!");
	  }
	}

	/** 
	 * Inform the management layer to cancel a previously submitted net
	 *
	 *	  pre-conditions:
	 *		  - a network must have been registered and assigned the id "id"
	 *
	 *	  side-effects:
	 *		  - the hierarchy belonging to the net is cancelled in turn
	 *
	 *	  post-conditions:
	 *		  - the internal state of the network switches to CANCELLING
	 *		  - all children of the network will be terminated
	 * */
	bool cancel(const id_type & id, const reason_type & reason)
	{
	  we::util::remove_unused_variable_warning(id);
	  we::util::remove_unused_variable_warning(reason);
	  return true;
	}

	/**
	 * Inform the management layer that an execution finished with the given result
	 *
	 *	  pre-conditions:
	 *		  - the management layer submitted an activity to be executed with id "id"
	 *
	 *	  side-effects:
	 *		  -	the results are integrated into the referenced net
	 *
	 *	  post-conditions:
	 *		  - the node belonging to this is activity is removed
	 **/
	bool finished(const id_type & id, const result_type & result)
	{
	  we::util::remove_unused_variable_warning(id);
	  we::util::remove_unused_variable_warning(result);
	  return true;
	}

	/**
	 * Inform the management layer that an execution failed with the given result
	 *
	 *	  pre-conditions:
	 *		  - the management layer submitted an activity to be executed with id "id"
	 *
	 *	  side-effects:
	 *		  -	the results are integrated into the referenced net
	 *
	 *	  post-conditions:
	 *		  - the node belonging to this activity is removed
	 **/
	bool failed(const id_type & id, const result_type & result)
	{
	  we::util::remove_unused_variable_warning(id);
	  we::util::remove_unused_variable_warning(result);
	  return true;
	}

	/**
	 * Inform the management layer that an execution has been cancelled
	 *
	 *	  pre-conditions:
	 *		  - the management layer submitted an activity to be executed with id "id"
	 *		  - the management layer requested the cancellation of an activity
	 *
	 *	  side-effects:
	 *		  - the enclosing workflow will be informed that an activity has been cancelled
	 *
	 *	  post-conditions:
	 *		  - the node belonging to this activity is removed
	 **/
	bool cancelled(const id_type & id)
	{
	  we::util::remove_unused_variable_warning(id);
	  return true;
	}

	// END: EXTERNAL API

	status_type status(const id_type & id)
	{
	  we::util::remove_unused_variable_warning(id);
	  return status_traits::RUNNING();
	}

	const net_type & lookup(const id_type & id)
	{
	  we::util::remove_unused_variable_warning(id);
	  throw std::runtime_error("not yet implemented");
	}

  public:
	explicit
	layer(exec_layer_type & exec_layer)
	  : exec_layer_(exec_layer)
	{ }

  private:
	exec_layer_type & exec_layer_;
  };
}}

#endif
