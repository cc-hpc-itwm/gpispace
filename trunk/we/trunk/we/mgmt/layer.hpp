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

#include <we/net.hpp>
#include <we/util/warnings.hpp>
#include <we/concurrent/deque.hpp>

#include <boost/random.hpp>
#include <boost/function.hpp>
#include <boost/functional/hash.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>

#include <we/mgmt/bits/types.hpp>
#include <we/mgmt/bits/traits.hpp>
#include <we/mgmt/bits/policy.hpp>
#include <we/mgmt/bits/descriptor.hpp>
#include <we/mgmt/bits/commands.hpp>
#include <we/mgmt/bits/synch_net.hpp>
#include <we/mgmt/basic_layer.hpp>
#include <we/mgmt/parser.hpp>

namespace we { namespace mgmt {

  template <typename ExecutionLayer
	, typename Net
	, typename Traits = detail::layer_traits<Net, typename ExecutionLayer::id_type>
	, typename Policy = detail::layer_policy<Traits>
	>
	class layer : public basic_layer<typename Traits::id_traits::type>
	{
	  public:
		typedef layer<ExecutionLayer, Net, Traits, Policy> this_type;
		typedef ExecutionLayer exec_layer_type;
		typedef Net net_type;
		typedef Traits traits_type;

		typedef typename traits_type::net_traits net_traits;
		typedef typename net_traits::validator_type net_validator;

		typedef typename traits_type::id_traits  id_traits;
		typedef typename id_traits::type id_type;

		typedef typename traits_type::status_traits status_traits;
		typedef typename status_traits::type status_type;

		typedef typename traits_type::result_traits result_traits;
		typedef typename result_traits::type result_type;

		typedef typename traits_type::codec_type codec_type;
		typedef typename codec_type::encode_type encode_type;
		typedef typename traits_type::reason_traits::type reason_type;

		typedef typename net_type::place_type place_type;
		typedef typename net_type::edge_type edge_type;
		typedef typename net_type::transition_type transition_type;
		typedef typename net_type::token_type token_type;

		typedef detail::descriptor<id_type, net_type, status_type, encode_type> descriptor_type;
		typedef typename boost::unordered_map<id_type, descriptor_type> id_descriptor_map_t;
	  private:
		typedef detail::commands::command_t<detail::commands::E_CMD_ID, id_type> e_cmd_t;
		typedef concurrent::deque<e_cmd_t> e_cmd_q_t;

	  public:

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
		void submit(const id_type & id, const encode_type & bytes) throw (std::exception)
		{
		  using detail::commands::make_cmd;
		  net_type n = codec_type::decode(bytes);

		  // check network for validity
		  if (net_validator::is_valid(n))
		  {
			std::cerr << "D: submitted petri-net["<< id << "] = " << n << std::endl;
			{
			  descriptor_type desc(id, n);
			  desc.parent = id_traits::nil();
			  insert_descriptor(id, desc);
			}
			e_cmd_q_.put(make_cmd(id, boost::bind(&this_type::net_needs_attention, this, _1)));
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
		bool cancel(const id_type & id, const reason_type & reason) throw ()
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
		bool finished(const id_type & id, const result_type & result) throw()
		{
		  we::util::remove_unused_variable_warning(id);
		  we::util::remove_unused_variable_warning(result);
		  e_cmd_q_.put(make_cmd(id, boost::bind(&this_type::net_needs_attention, this, _1)));
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
		bool failed(const id_type & id, const result_type & result) throw()
		{
		  we::util::remove_unused_variable_warning(id);
		  we::util::remove_unused_variable_warning(result);
		  e_cmd_q_.put(make_cmd(id, boost::bind(&this_type::net_needs_attention, this, _1)));
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
		bool cancelled(const id_type & id) throw()
		{
		  we::util::remove_unused_variable_warning(id);
		  return true;
		}

		/**
		 * Temporarily suspend the execution of the given petri-net
		 *
		 *	  pre-conditions:
		 *		  - a net with the given id had been submitted
		 *
		 *	  side-effects:
		 *		  - sub-activities are suspended as well
		 *
		 *	  post-conditions:
		 *		  - the network will not be considered in the selection of new activities
		 *		  - the execution of the network is on hold
		 */
		bool suspend(const id_type & id) throw()
		{
		  we::util::remove_unused_variable_warning(id);
		  e_cmd_q_.put(make_cmd(id, boost::bind(&this_type::suspend_net, this, _1)));
		  return true;
		}

		/**
		 * Execution of a network is resumed.
		 *
		 *	  pre-conditions:
		 *		  - a net with the given id had been submitted
		 *
		 *	  side-effects:
		 *		  - sub-activities are resumed as well
		 *
		 *	  post-conditions:
		 *		  - the network will again be considered in the selection of new activities
		 *
		 */
		bool resume(const id_type & id) throw()
		{
		  we::util::remove_unused_variable_warning(id);
		  e_cmd_q_.put(make_cmd(id, boost::bind(&this_type::resume_net, this, _1)));
		  return true;
		}

		// END: EXTERNAL API

		status_type status(const id_type & id) throw (std::exception)
		{
		  we::util::remove_unused_variable_warning(id);
		  throw std::runtime_error("not yet implemented: status");
		}

		void submit(const id_type & id, const id_type & parent, const encode_type & bytes)
		{
		  descriptor_type desc(id, descriptor_type::ACTIVITY);
		  desc.parent = parent;
		  desc.data = bytes;
		  descriptors_.insert(std::make_pair(id, desc));
		}

	  public:
		/**
		 * Constructor calls
		 */ 
		template <class E>
		  explicit
		  layer(E & exec_layer)
		  : exec_layer_(exec_layer)
			, id_gen_(&id_traits::generate)
			   , e_cmd_q_(1024)
	  { }

		template <class E, typename G>
		  layer(E & exec_layer, G gen)
		  : exec_layer_(exec_layer)
			, id_gen_(gen)
			   , e_cmd_q_(1024)
	  {
		start();
	  }

		template <class E, typename G, typename V>
		  layer(E & exec_layer, G gen, V validator)
		  : exec_layer_(exec_layer)
			, id_gen_(gen)
			   , validator_(validator)
	  {
		start();
	  }

		~layer()
		{
		  stop();

		  // stop threads
		  //
		  // cancel activities
		  //
		  // delete memory allocations
		  //
		  // clean up all descriptors
		  for (typename id_descriptor_map_t::iterator d = (descriptors_.begin()); d != descriptors_.end(); ++d)
		  {
			std::cerr << "D: removing descriptor[" << d->first << "]" << std::endl;
		  }
		}

		/* internal functions */
	  private:
		void start()
		{
		  extractor_ = boost::thread(boost::bind(&this_type::extractor, this));
		}

		void stop()
		{
		  std::cerr << "D: cleaning up extractor thread..." << std::endl;
		  extractor_.interrupt();
		  extractor_.join();
		}

		void extractor()
		{
		  using namespace we::mgmt::detail::commands;
		  std::cerr << "D: extractor thread started..." << std::endl;
		  for (;;)
		  {
			e_cmd_t cmd = e_cmd_q_.get();
			cmd.handle();
		  }
		  std::cerr << "D: extractor thread stopped..." << std::endl;
		}

		void injector()
		{
		}

		void executor()
		{
		}

		inline void insert_descriptor(const id_type & id, const descriptor_type & desc)
		{
		  boost::unique_lock<boost::mutex> lock (descriptors_mutex_);
		  descriptors_.insert(std::make_pair(id, desc));
		}

		inline descriptor_type & lookup(const id_type & id)
		{
		  boost::unique_lock<boost::mutex> lock (descriptors_mutex_);
		  typename id_descriptor_map_t::iterator d = descriptors_.find(id);
		  assert( d != descriptors_.end() );
		  return d->second;
		}

		/** Member variables **/
	  private:
		exec_layer_type & exec_layer_;
		boost::function<id_type()> id_gen_;
		boost::function<bool (const net_type&)> validator_;
		boost::mutex descriptors_mutex_;
		id_descriptor_map_t descriptors_;
		e_cmd_q_t e_cmd_q_;

		boost::thread extractor_;

		void net_needs_attention(const e_cmd_t & cmd)
		{
		  descriptor_type & desc = lookup(cmd.dat);
		  assert ( desc.is_net() );
		  if (desc.net.has_enabled())
		  {
			do
			{
			  std::cerr << "I: net[" << desc.id << "] has " << desc.net.num_enabled() << " enabled transition(s)" << std::endl;
			  typename net_type::activity_t act = desc.net.extract();

			} while (desc.net.has_enabled());
		  }
		  else
		  {
			std::cerr << "E: attention for net[" << desc.id << "] had been requested, but nothing to do!" << std::endl;
		  }
		}
		void suspend_net(const e_cmd_t & cmd)
		{
		  std::cerr << "I: net[" << cmd.dat << "] suspended" << std::endl;
		}
		void resume_net(const e_cmd_t & cmd)
		{
		  std::cerr << "I: net[" << cmd.dat << "] resumed" << std::endl;
		  e_cmd_q_.put(make_cmd(cmd.dat, boost::bind(&this_type::net_needs_attention, this, _1)));
		}
	};
}}

#endif
