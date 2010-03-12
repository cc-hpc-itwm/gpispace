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

#include <boost/random.hpp>
#include <boost/function.hpp>
#include <boost/functional/hash.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include <we/mgmt/bits/types.hpp>
#include <we/mgmt/bits/traits.hpp>
#include <we/mgmt/bits/policy.hpp>
#include <we/mgmt/bits/descriptor.hpp>
#include <we/mgmt/bits/commands.hpp>
#include <we/mgmt/bits/synch_net.hpp>
#include <we/mgmt/bits/queue.hpp>
#include <we/mgmt/bits/set.hpp>
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
		typedef detail::set<id_type> active_nets_t;
		typedef detail::commands::command_t<detail::commands::CMD_ID, id_type> cmd_t;
		typedef detail::queue<cmd_t> cmd_q_t;

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
			std::cerr << "D: submitted petri-net["<< id << "]" << std::endl;
			debug_net (n);
			{
			  descriptor_type desc(id, n);
			  desc.parent = id_traits::nil();
			  insert_descriptor(id, desc);
			}
			cmd_q_.put(make_cmd(id, boost::bind(&this_type::net_needs_attention, this, _1)));
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
//		  cmd_q_.put (make_cmd(id, boost::bind(&this_type::net_needs_attention, this, _1)));
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
//		  cmd_q_.put (make_cmd(id, boost::bind(&this_type::net_needs_attention, this, _1)));
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
//		  cmd_q_.put (make_cmd(id, boost::bind(&this_type::suspend_net, this, _1)));
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
//		  cmd_q_.put (make_cmd(id, boost::bind(&this_type::resume_net, this, _1)));
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
			   , cmd_q_(1024)
			   , active_nets_(1024)
	  {
		start();
	  }

		template <class E, typename G>
		  layer(E & exec_layer, G gen)
		  : exec_layer_(exec_layer)
		  , id_gen_(gen)
		  , cmd_q_(1024)
		  , active_nets_(1024)
	  {
		start();
	  }

		template <class E, typename G, typename V>
		  layer(E & exec_layer, G gen, V validator)
		  : exec_layer_(exec_layer)
		  , id_gen_(gen)
		  , validator_(validator)
		  , cmd_q_(1024)
		  , active_nets_(1024)
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
		  manager_   = boost::thread(boost::bind(&this_type::manager, this));
		  extractor_ = boost::thread(boost::bind(&this_type::extractor, this));
		  injector_  = boost::thread(boost::bind(&this_type::injector, this));
		}

		void stop()
		{
		  std::cerr << "D: cleaning up injector thread..." << std::endl;
		  injector_.interrupt();
		  injector_.join();

		  std::cerr << "D: cleaning up manager thread..." << std::endl;
		  manager_.interrupt();
		  manager_.join();

		  std::cerr << "D: cleaning up extractor thread..." << std::endl;
		  extractor_.interrupt();
		  extractor_.join();
		}

		void manager()
		{
		  using namespace we::mgmt::detail::commands;
		  std::cerr << "D: manager thread started..." << std::endl;
		  for (;;)
		  {
			cmd_t cmd = cmd_q_.get();
			cmd.handle();
		  }
		  std::cerr << "D: manager thread stopped..." << std::endl;
		}

		void extractor()
		{
		  using namespace we::mgmt::detail::commands;
		  std::cerr << "D: extractor thread started..." << std::endl;
		  for (;;)
		  {
			// TODO: probably timed wait?
			id_type active_net = active_nets_.get();

			descriptor_type & desc = lookup(active_net);
			assert ( desc.is_net() );

			if (desc.flags.suspended)
			{
			  std::cerr << "I: net[" << desc.id << "] is suspended" << std::endl;
			  continue;
			}

			if (desc.net.done())
			{
			  std::cerr << "I: net finished" << std::endl;
			  debug_net (desc.get_real_net());
			  continue;
			}

			// submit new activities
			while (desc.net.has_enabled())
			{
			  std::cerr << "I: net[" << desc.id << "] has "
				<< desc.net.num_enabled()
				<< " enabled transition(s)" << std::endl;

			  typename net_type::activity_t act = desc.net.extract();
			  // classify 
			  async_execute (act, desc);
			}
		  }
		  std::cerr << "D: extractor thread stopped..." << std::endl;
		}

		void injector()
		{
		  std::cerr << "D: injector thread started..." << std::endl;
		  std::cerr << "D: injector thread stopped..." << std::endl;
		}

		void executor()
		{
		  std::cerr << "D: executor thread started..." << std::endl;
		  std::cerr << "D: executor thread stopped..." << std::endl;
		}

		inline void insert_descriptor(const id_type & id, const descriptor_type & desc)
		{
		  boost::unique_lock<boost::shared_mutex> lock (descriptors_mutex_);
		  descriptors_.insert(std::make_pair(id, desc));
		}

		inline descriptor_type & lookup(const id_type & id)
		{
		  boost::shared_lock <boost::shared_mutex> lock (descriptors_mutex_);
		  typename id_descriptor_map_t::iterator d = descriptors_.find(id);
		  assert( d != descriptors_.end() );
		  return d->second;
		}

		/** Member variables **/
	  private:
		exec_layer_type & exec_layer_;
		boost::function<id_type()> id_gen_;
		boost::function<bool (const net_type&)> validator_;
		boost::shared_mutex descriptors_mutex_;
		id_descriptor_map_t descriptors_;
		cmd_q_t cmd_q_;
		active_nets_t active_nets_;

		boost::mutex active_nets_mutex_;
		boost::condition active_nets_modified_;

		boost::thread extractor_;
		boost::thread manager_;
		boost::thread injector_;

		inline void debug_net(net_type const & n)
		{
		  std::cerr << n << std::endl;
		}

		template <typename Activity, typename N>
		  inline void debug_activity(Activity const & act, N const & net)
		  {
			std::cerr << "D: transition[" << act.tid << "](" << net.get_transition(act.tid) << "):" << std::endl;
			{
			  std::cerr << "\tin:" << std::endl;
			  for ( typename net_type::input_t::const_iterator it (act.input.begin())
				  ; it != act.input.end()
				  ; ++it)
			  {
				std::cerr << "\t\t" << it->first
				  << " from place " << it->second.first
				  << " via edge " << it->second.second
				  << std::endl;
			  }
			}

			{
			  std::cerr << "\tout:" << std::endl;
			  for ( typename net_type::output_descr_t::const_iterator it (act.output_descr.begin())
				  ; it != act.output_descr.end()
				  ; ++it)
			  {
				std::cerr << "\t\t"
				  << "to place " << it->first
				  << " via edge " << it->second 
				  << std::endl;
			  }
			}
		  }

		void net_needs_attention(const cmd_t & cmd)
		{
		  active_nets_.put (cmd.dat);
		}

		void suspend_net(const cmd_t & cmd)
		{
		  std::cerr << "I: net[" << cmd.dat << "] suspended" << std::endl;
		  lookup(cmd.dat).flags.suspended = true;
		}

		void resume_net(const cmd_t & cmd)
		{
		  std::cerr << "I: net[" << cmd.dat << "] resumed" << std::endl;
		  lookup(cmd.dat).flags.suspended = false;
		  active_nets_.put (cmd.dat);
		}

		void async_execute(typename net_type::activity_t const & act, descriptor_type & desc)
		{
		  debug_activity (act, desc.get_real_net());

		  typename net_type::output_t out;
		  desc.get_real_net().get_transition (act.tid) (act.input, act.output_descr, out);
		  desc.net.inject (out);

		  active_nets_.put ( desc.id );
		}
	};
}}

#endif
