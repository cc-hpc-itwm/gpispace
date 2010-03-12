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

		// manager thread
		typedef detail::commands::command_t<detail::commands::CMD_ID, id_type> cmd_t;
		typedef detail::queue<cmd_t> cmd_q_t;

		// injector thread
		typedef std::pair<id_type, std::pair<int, typename net_type::output_t> > inj_cmd_t;
		typedef detail::queue<inj_cmd_t> inj_q_t;

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
			submit (id, n);
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

		  // TODO: parse results
		  // hand results over to injector
		  typename net_type::output_t output /* (parse_result(result)) */;
		  inj_q_.put ( std::make_pair (id, std::make_pair(0, output)) );
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

		void submit (const id_type & id, const net_type & n)
		{
		  debug_net (n);
		  {
			descriptor_type desc(id, n);
			desc.parent = id_traits::nil();
			insert_descriptor(id, desc);
		  }
		  cmd_q_.put(make_cmd(id, boost::bind(&this_type::net_needs_attention, this, _1)));
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
			   , inj_q_(1024)
	  {
		start();
	  }

		template <class E, typename G>
		  layer(E & exec_layer, G gen)
		  : exec_layer_(exec_layer)
		  , id_gen_(gen)
		  , cmd_q_(1024)
		  , active_nets_(1024)
		  , inj_q_(1024)
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
		  , inj_q_(1024)
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

		inline
		bool is_net_alive ( const descriptor_type & desc ) const
		{
		  if ( desc.flags.suspended || desc.flags.cancelling ) return false;
		  return true;
		}

		inline
		bool is_net_done ( const descriptor_type & desc ) const
		{
		  return desc.net.done();
		}

		inline void post_finished_notification( const id_type & id)
		{
		  cmd_q_.put(make_cmd(id, boost::bind(&this_type::net_finished, this, _1)));
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

			if (! is_net_alive (desc))
			{
			  continue;
			}

			if (is_net_done (desc))
			{
			  post_finished_notification (active_net);
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
		  for (;;)
		  {
			inj_cmd_t cmd = inj_q_.get();
			try
			{
			  descriptor_type & act_desc = lookup( cmd.first );
			  descriptor_type & net_desc = lookup( act_desc.parent );
			  std::cerr << "I: injecting results of "
						<< "activity[" << cmd.first << "] into "
						<< "net[" << act_desc.parent << "]"
						<< std::endl;
			  net_desc.net.inject (cmd.second.second);
			  net_desc.children.erase ( act_desc.id );
			  remove_descriptor ( act_desc );
			  cmd_q_.put (make_cmd( net_desc.id, boost::bind(&this_type::net_needs_attention, this, _1)));
			}
			catch (const std::exception & ex)
			{
			  std::cerr << "E: error during injecting: " << ex.what() << std::endl;
			  // ignore
			}
		  }
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

		inline void remove_descriptor(const descriptor_type & desc)
		{
		  remove_descriptor (desc.id);
		}

		inline void remove_descriptor(const id_type & id)
		{
		  boost::unique_lock<boost::shared_mutex> lock (descriptors_mutex_);
		  descriptors_.erase (id);
		}

		inline descriptor_type & lookup(const id_type & id)
		{
		  boost::shared_lock <boost::shared_mutex> lock (descriptors_mutex_);
		  typename id_descriptor_map_t::iterator d = descriptors_.find(id);
		  if (d == descriptors_.end()) throw std::runtime_error( "not found" );
//		  assert( d != descriptors_.end() );
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
		inj_q_t inj_q_;

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
		  std::cerr << "D: net[" << cmd.dat << "] active" << std::endl;
		  active_nets_.put (cmd.dat);
		}

		void net_finished(const cmd_t & cmd)
		{
		  std::cerr << "D: net[" << cmd.dat << "] finished" << std::endl;
		  exec_layer_.finished ( cmd.dat, "dummy result" );
		  remove_descriptor ( cmd.dat );
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

		void async_execute(typename net_type::activity_t const & act, descriptor_type & parent_desc)
		{
		  descriptor_type act_desc (id_gen_(), descriptor_type::ACTIVITY);
		  act_desc.parent = parent_desc.id;
		  parent_desc.children.insert ( act_desc.id );
		  insert_descriptor (act_desc.id, act_desc);
		  std::cerr << "D: executing activity[" << act_desc.id << "]..." << std::endl;
		  debug_activity (act, parent_desc.get_real_net());

		  typename net_type::output_t out;
		  parent_desc.get_real_net().get_transition (act.tid) (act.input, act.output_descr, out);
		  inj_q_.put ( std::make_pair (act_desc.id, std::make_pair(0, out)) );
		}
	};
}}

#endif
