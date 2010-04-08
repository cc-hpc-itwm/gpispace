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

#include <we/mgmt/exception.hpp>
#include <we/mgmt/bits/types.hpp>
#include <we/mgmt/bits/traits.hpp>
#include <we/mgmt/bits/policy.hpp>
#include <we/mgmt/bits/commands.hpp>
#include <we/mgmt/bits/synch_net.hpp>
#include <we/mgmt/bits/queue.hpp>
#include <we/mgmt/bits/set.hpp>
#include <we/mgmt/bits/signal.hpp>
#include <we/mgmt/bits/activity.hpp>
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

        typedef std::string encode_type;
//		typedef typename traits_type::codec_type codec_type;
//		typedef typename codec_type::encode_type encode_type;
		typedef typename traits_type::reason_traits::type reason_type;

		typedef typename net_type::place_type place_type;
		typedef typename net_type::edge_type edge_type;
		typedef typename net_type::transition_type transition_type;
		typedef typename net_type::token_type token_type;

        typedef detail::activity<id_type, transition_type> activity_type;
		typedef typename boost::unordered_map<id_type, activity_type> activities_t;
	  private:
		typedef detail::set<id_type> active_nets_t;

		// manager thread
		typedef detail::commands::command_t<detail::commands::CMD_ID, id_type> cmd_t;
		typedef detail::queue<cmd_t> cmd_q_t;

		// injector thread
//		typedef std::pair<id_type, typename net_type::output_t> inj_cmd_t;
		typedef id_type inj_cmd_t;
		typedef detail::queue<inj_cmd_t> inj_q_t;

        typedef id_type executor_cmd_t;
		typedef detail::queue<executor_cmd_t> exec_q_t;

	  public:

		/******************************
		 * EXTERNAL API
		 *****************************/

		// observe
		detail::signal<void (id_type, std::string)> sig_finished;
		detail::signal<void (id_type, std::string)> sig_failed;
		detail::signal<void (id_type, std::string)> sig_cancelled;
		detail::signal<void (id_type, std::string)> sig_execute;

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
          activity_type act(id);
          parser<net_type>::parse(act, bytes);
          submit (id, act);
          std::cerr << "D: submitted act["<< id << "]" << std::endl;
//
//		  // check network for validity
//		  if (net_validator::is_valid(*act.data.net))
//		  {
//		  }
//		  else
//		  {
//			// TODO: detailed error analysis
//			throw std::runtime_error("network invalid!");
//		  }
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
		  // TODO: parse results
		  we::util::remove_unused_variable_warning(result);
		  // hand results over to injector
		  inj_q_.put ( id );
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
          post_suspend_network_notification(id);
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
          post_resume_network_notification(id);
		  return true;
		}

		// END: EXTERNAL API

		status_type status(const id_type & id) throw (std::exception)
		{
		  we::util::remove_unused_variable_warning(id);
		  throw std::runtime_error("not yet implemented: status");
		}

      private:
		void submit (const id_type & id, activity_type & act)
		{
		  debug_activity (act);
		  {
            act.prepare_input();
			insert_activity(id, act);
		  }
		  post_activity_notification (id);
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
            , barrier_(4 + 1) // 1 + injector, manager, executor, extractor
			   , cmd_q_(1024)
			   , active_nets_(1024)
			   , inj_q_(1024)
               , exec_q_(1024)
	  {
		start();
	  }

		template <class E, typename G>
		  layer(E & exec_layer, G gen)
		  : exec_layer_(exec_layer)
		  , id_gen_(gen)
          , barrier_(4 + 1) // 1 + injector, manager, executor, extractor
		  , cmd_q_(1024)
		  , active_nets_(1024)
		  , inj_q_(1024)
          , exec_q_(1024)
	  {
		start();
	  }

		template <class E, typename G, typename V>
		  layer(E & exec_layer, G gen, V validator)
		  : exec_layer_(exec_layer)
		  , id_gen_(gen)
		  , validator_(validator)
          , barrier_(4 + 1) // injector, manager, executor, extractor
		  , cmd_q_(1024)
		  , active_nets_(1024)
		  , inj_q_(1024)
          , exec_q_(1024)
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
		  // clean up all activity
		  for (typename activities_t::iterator a = (activities_.begin()); a != activities_.end(); ++a)
		  {
			std::cerr << "D: removing act[" << a->first << "]" << std::endl;
		  }
		}

		/* internal functions */
	  private:
		void start()
		{
		  manager_   = boost::thread(boost::bind(&this_type::manager, this));
		  extractor_ = boost::thread(boost::bind(&this_type::extractor, this));
		  injector_  = boost::thread(boost::bind(&this_type::injector, this));
		  executor_  = boost::thread(boost::bind(&this_type::executor, this));
          barrier_.wait();
		}

		void stop()
		{
		  std::cerr << "D: cleaning up executor thread..." << std::endl;
		  executor_.interrupt();
		  executor_.join();

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
          barrier_.wait();
		  for (;;)
		  {
			cmd_t cmd = cmd_q_.get();
            try
            {
			  cmd.handle();
            }
            catch (std::exception const& ex)
            {
              std::cerr << "W: error during command handling: " << ex.what() << std::endl;
            }
		  }
		  std::cerr << "D: manager thread stopped..." << std::endl;
		}

		inline
		bool is_alive ( const activity_type & act ) const
		{
          return act.is_alive();
		}

		inline
		bool is_done ( const activity_type & act ) const
		{
		  return act.done();
		}

		inline
		void post_activity_notification( const id_type & id)
		{
		  cmd_q_.put(make_cmd(id, boost::bind(&this_type::net_needs_attention, this, _1)));
		}

		inline
		void post_finished_notification( const id_type & id)
		{
		  cmd_q_.put(make_cmd(id, boost::bind(&this_type::net_finished, this, _1)));
		}

		inline
		void post_failed_notification( const id_type & id)
		{
		  cmd_q_.put(make_cmd(id, boost::bind(&this_type::net_failed, this, _1)));
		}

		inline
		void post_cancelled_notification( const id_type & id)
		{
		  cmd_q_.put(make_cmd(id, boost::bind(&this_type::net_cancelled, this, _1)));
		}

        inline
        void post_suspend_network_notification( const id_type & id )
        {
		  cmd_q_.put (make_cmd(id, boost::bind(&this_type::suspend_net, this, _1)));
        }

        inline
        void post_resume_network_notification( const id_type & id )
        {
		  cmd_q_.put (make_cmd(id, boost::bind(&this_type::resume_net, this, _1)));
        }

        inline
        void post_execute_notification ( const id_type & id )
        {
          exec_q_.put ( id );
        }

        inline
        void post_inject_activity_results ( const id_type & id )
        {
          inj_q_.put ( id );
        }

		void extractor()
		{
		  using namespace we::mgmt::detail::commands;
		  std::cerr << "D: extractor thread started..." << std::endl;
          barrier_.wait();
		  for (;;)
		  {
			// TODO: probably timed wait?
			id_type active_net = active_nets_.get();

            try
            {
              activity_type & act = lookup(active_net);

              if (is_done (act))
              {
                std::cerr << "D: act[" << act.id() << "] is done." << std::endl;
                post_finished_notification (active_net);
                continue;
              }

              // TODO: check status flags
              if (! is_alive (act))
              {
                std::cerr << "D: act[" << act.id() << "] is on hold." << std::endl;
                continue;
              }

              // submit new activities
              while (act.has_enabled())
              {
                std::cerr << "I: act[" << act.id() << "] has "
                  << act.num_enabled()
                  << " enabled transition(s)" << std::endl;

                activity_type sub_act = act.extract(id_gen_());
                insert_activity (sub_act.id(), sub_act);
                // classify and execute
                async_execute (lookup (sub_act.id()) );
              }
            }
            catch (const net_not_found<id_type> & ex)
            {
              std::cerr << "W: activity could not be found: " << ex.id << std::endl;
            }
		  }
		  std::cerr << "D: extractor thread stopped..." << std::endl;
		}

		void injector()
		{
		  std::cerr << "D: injector thread started..." << std::endl;
          barrier_.wait();
		  for (;;)
		  {
			inj_cmd_t cmd = inj_q_.get();
			try
			{
              // TODO: when act is network, probably collect all output?
			  activity_type & act = lookup( cmd );
              activity_type & par = lookup( act.parent() );
			  std::cerr << "I: injecting results of "
						<< "act[" << act.id() << "] into "
						<< "act[" << par.id() << "]"
						<< std::endl;

              par.child_finished (act);

              post_activity_notification( par.id() );

			  remove_activity ( act );
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
          barrier_.wait();
          for (;;)
          {
            executor_cmd_t cmd = exec_q_.get();
            try
            {
              std::cerr << "I: executing: " << cmd << std::endl;
              activity_type & act = lookup (cmd);

              try
              {
                act.execute ( *this );
                // inject results
                post_inject_activity_results ( act.id () );
              }
              catch (std::exception const & ex)
              {
                // activity failed
              }
            } catch (const std::exception & ex)
            {
			  std::cerr << "E: error during execution: " << ex.what() << std::endl;
			  // ignore
            }
          }
		  std::cerr << "D: executor thread stopped..." << std::endl;
		}
		/** Member variables **/
	  private:
		exec_layer_type & exec_layer_;
		boost::function<id_type()> id_gen_;
		boost::function<bool (const net_type&)> validator_;
		boost::barrier barrier_;
		mutable boost::shared_mutex activities_mutex_;
		activities_t activities_;
		cmd_q_t cmd_q_;
		active_nets_t active_nets_;
		inj_q_t inj_q_;
        exec_q_t exec_q_;

		boost::mutex active_nets_mutex_;
		boost::condition active_nets_modified_;

		boost::thread extractor_;
		boost::thread manager_;
		boost::thread injector_;
		boost::thread executor_;

		inline void debug_net(net_type const & n)
		{
		  std::cerr << n << std::endl;
		}

		template <typename Activity>
		  inline void debug_activity(Activity const & act)
		  {
			std::cerr << "D: act[" << act.id() << "](type=" << act.transition().type << " " << act.transition() << "):" << std::endl;
			{
			  std::cerr << "\tin:" << std::endl;
			  for ( typename Activity::input_t::const_iterator it (act.input().begin())
				  ; it != act.input().end()
				  ; ++it)
			  {
				std::cerr << "\t\t" << it->first
				  << " on place " << it->second
				  << std::endl;
			  }
			}
		  }

		void net_needs_attention(const cmd_t & cmd)
		{
          activity_type & a = lookup ( cmd.dat );
          if (! a.flags().suspended )
          {
            std::cerr << "D: act[" << cmd.dat << "] active" << std::endl;
            active_nets_.put (cmd.dat);
          }
		}

		void net_finished(const cmd_t & cmd)
		{
		  exec_layer_.finished ( cmd.dat, "dummy result" );
		  assert_is_leaf ( cmd.dat );
		  remove_activity ( cmd.dat );
		  std::cerr << "D: act[" << cmd.dat << "] finished" << std::endl;
		}

		void net_failed(const cmd_t & cmd)
		{
		  exec_layer_.failed ( cmd.dat, "dummy result" );
		  assert_is_leaf ( cmd.dat );
		  remove_activity ( cmd.dat );
		  std::cerr << "D: act[" << cmd.dat << "] failed" << std::endl;
		}

		void net_cancelled(const cmd_t & cmd)
		{
		  exec_layer_.cancelled ( cmd.dat );
		  assert_is_leaf ( cmd.dat );
		  remove_activity ( cmd.dat );
		  std::cerr << "D: act[" << cmd.dat << "] cancelled" << std::endl;
		}

		inline
		void assert_is_leaf (const id_type & id) const
		{
		  const activity_type & act = lookup (id);
		  if (! act.is_leaf()) throw std::runtime_error("not leaf");
		}

		void suspend_net(const cmd_t & cmd)
		{
		  lookup(cmd.dat).flags().suspended = true;
		  std::cerr << "I: act[" << cmd.dat << "] suspended" << std::endl;
		}

		void resume_net(const cmd_t & cmd)
		{
		  lookup(cmd.dat).flags().suspended = false;
		  active_nets_.put (cmd.dat);
		  std::cerr << "I: act[" << cmd.dat << "] resumed" << std::endl;
		}

		void async_execute(activity_type & act)
		{
		  debug_activity (act);
          if (act.transition().flags.internal)
          {
            if (act.transition().is_net())
            {
              act.prepare_input();
              post_activity_notification (act.id());
            }
            else
            {
              act.prepare_input();
              post_execute_notification (act.id());
            }
          }
          else
          {
            sig_execute( act.id(), ::util::show(act.transition()) );
          }
		}

		inline void insert_activity(const id_type & id, const activity_type & act)
		{
		  boost::unique_lock<boost::shared_mutex> lock (activities_mutex_);
		  activities_.insert(std::make_pair(id, act));
		}

		inline void remove_activity(const activity_type & act)
		{
		  remove_activity (act.id());
		}

		inline void remove_activity(const id_type & id)
		{
		  boost::unique_lock<boost::shared_mutex> lock (activities_mutex_);
          // TODO: work here
		  activities_.erase (id);
		}

		inline activity_type & lookup(const id_type & id)
		{
		  boost::shared_lock <boost::shared_mutex> lock (activities_mutex_);
		  typename activities_t::iterator a = activities_.find(id);
          if (a == activities_.end()) throw net_not_found<id_type>("lookup", id);
		  return a->second;
		}

		inline const activity_type & lookup(const id_type & id) const
		{
		  boost::shared_lock <boost::shared_mutex> lock (activities_mutex_);
		  typename activities_t::const_iterator a = activities_.find(id);
		  if (a == activities_.end()) throw net_not_found<id_type>("lookup", id);
		  return a->second;
		}
	};
}}

#endif
