// alexander.petry@itwm.fraunhofer.de

#ifndef WE_MGMT_LAYER_HPP
#define WE_MGMT_LAYER_HPP 1

#include <we/mgmt/bits/descriptor.hpp>
#include <we/mgmt/bits/execution_policy.hpp>
#include <we/mgmt/type/activity.hpp>
#include <we/type/id.hpp>
#include <we/type/net.hpp>
#include <we/type/requirement.hpp>
#include <we/type/user_data.hpp>
#include <we/type/schedule_data.hpp>

#include <fhg/assert.hpp>
#include <fhg/error_codes.hpp>
#include <fhg/util/thread/queue.hpp>
#include <fhg/util/threadname.hpp>

#include <fhglog/fhglog.hpp>

#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/optional.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>

namespace we
{
  namespace mgmt
  {
    class layer
    {
    public:
      typedef std::string id_type;

      typedef std::string external_id_type;
      typedef petri_net::activity_id_type internal_id_type;

      typedef std::string encoded_type;
      typedef std::string reason_type;
      typedef std::string result_type;

    private:
      typedef boost::unordered_map<external_id_type, internal_id_type> external_to_internal_map_t;

      typedef boost::unordered_map< internal_id_type
                                  , detail::descriptor
                                  > activities_t;

      //! \todo is it necessary to use a locked data structure?
      typedef fhg::thread::queue<internal_id_type> active_nets_t;

      typedef boost::unique_lock<boost::recursive_mutex> lock_t;

    public:

      /******************************
       * EXTERNAL API
       *****************************/

      /**
       * Submit a new petri net to the petri-net management layer
       *
       *          pre-conditions: none
       *
       *          side-effects: parses the passed data
       *                                        registeres the petri-net with the mgmt layer
       *
       *          post-conditions: the net is registered is with id "id"
       *
       */
      void submit (const external_id_type&, const encoded_type&, const we::type::user_data &);
      void submit (const external_id_type&, const we::mgmt::type::activity_t&, const we::type::user_data &);

      /**
       * Inform the management layer to cancel a previously submitted net
       *
       *          pre-conditions:
       *                - a  network  must have  been registered  and
       *                  assigned the id "id"
       *
       *          side-effects:
       *                -  the  hierarchy   belonging  to  the  net  is
       *                   canceled in turn
       *
       *          post-conditions:
       *
       *                - the  internal   state  of  the   network  switches  to
       *                  CANCELING
       *                - all children of the network will be terminated
       */
      void cancel (const external_id_type&, const reason_type&);

      /**
       * Inform the management  layer that an execution finished  with the given
       * result
       *
       *          pre-conditions:
       *
       *              - the  management  layer   submitted  an  activity  to  be
       *                executed with id "id"
       *
       *          side-effects:
       *
       *              - the results are integrated into the referenced net
       *
       *          post-conditions:
       *
       *               - the node belonging to this is activity is removed
       **/
      void finished (const external_id_type&, const result_type&);

      /**
       * Inform the  management layer  that an execution  failed with  the given
       * result
       *
       *          pre-conditions:
       *
       *               - the  management  layer  submitted  an  activity  to  be
       *                 executed with id "id"
       *
       *          side-effects:
       *
       *               - the results are integrated into the referenced net
       *
       *          post-conditions:
       *
       *                - the node belonging to this activity is removed
       **/
      void failed ( const external_id_type&
                  , const result_type&
                  , const int error_code
                  , const std::string&
                  );

      /**
       * Inform the management layer that an execution has been canceled
       *
       *          pre-conditions:
       *                  - the management layer submitted an activity to be executed with id "id"
       *                  - the management layer requested the cancellation of an activity
       *
       *          side-effects:
       *                  - the enclosing workflow will be informed that an activity has been canceled
       *
       *          post-conditions:
       *                  - the node belonging to this activity is removed
       **/
      void canceled (const external_id_type&);

      // END: EXTERNAL API

    private:
      // handle execution layer
      boost::function<void ( external_id_type const &
                           , encoded_type const &
                           , const std::list<we::type::requirement_t>&
                           , const we::type::schedule_data&
                           , const we::type::user_data &
                           )> ext_submit;
      boost::function<void ( external_id_type const &
                           , reason_type const &
                           )>  ext_cancel;
      boost::function<void ( external_id_type const &
                           , result_type const &
                           )>  ext_finished;
      boost::function<void ( external_id_type const &
                           , result_type const &
                           , const int error_code
                           , std::string const & reason
                           )>  ext_failed;
      boost::function<void (external_id_type const &)> ext_canceled;

      void submit (const detail::descriptor & desc);

      void add_map_to_internal ( const external_id_type & external_id
                               , const internal_id_type & internal_id
                               );

      void del_map_to_internal ( const external_id_type & external_id
                               , const internal_id_type & internal_id
                               );

      void del_map_to_internal (const internal_id_type & internal_id);

      external_to_internal_map_t::mapped_type map_to_internal ( const external_id_type & external_id ) const;
    public:
      template <class E>
        layer (E* exec_layer, boost::function<external_id_type()> gen)
          : ext_submit (boost::bind (&E::submit, exec_layer, _1, _2, _3, _4, _5))
          , ext_cancel (boost::bind (&E::cancel, exec_layer, _1, _2))
          , ext_finished (boost::bind (&E::finished, exec_layer, _1, _2))
          , ext_failed (boost::bind (&E::failed, exec_layer, _1, _2, _3, _4))
          , ext_canceled (boost::bind (&E::canceled, exec_layer, _1))
          , external_id_gen_ (gen)
          , internal_id_gen_ (&petri_net::activity_id_generate)
          , executor_ (boost::bind (&layer::executor, this))
      {
        fhg::util::set_threadname (executor_, "[we-execute]");
      }

      ~layer()
      {
        executor_.interrupt();
        if (executor_.joinable())
        {
          executor_.join();
        }

        if (not activities_.empty ())
        {
          DMLOG (WARN, "#" << activities_.size () << " activities remaining:");
          activities_t::const_iterator it = activities_.begin ();
          const activities_t::const_iterator end = activities_.end ();
          while (it != end)
          {
            DMLOG (WARN, it->second);
            ++it;
          }
        }

        if (not ext_to_int_.empty ())
        {
          DMLOG (WARN, "#" << ext_to_int_.size () << " mappings remaining:");
          external_to_internal_map_t::iterator it = ext_to_int_.begin ();
          const external_to_internal_map_t::iterator end = ext_to_int_.end ();

          while (it != end)
          {
            DMLOG (WARN, it->first << " -> " << it->second);
            ++it;
          }
        }
      }

    private:
      void execute_externally (const internal_id_type & int_id);

      void executor ();

      detail::descriptor & do_extract (detail::descriptor & parent);

      void do_execute (detail::descriptor& desc);

      bool is_valid (const internal_id_type & id) const;

      void do_inject (detail::descriptor& desc);
    private:
      boost::function<external_id_type()> external_id_gen_;
      boost::function<internal_id_type()> internal_id_gen_;
      mutable boost::recursive_mutex mutex_;
      mutable boost::recursive_mutex id_gen_mutex_;
      activities_t activities_;

      active_nets_t active_nets_;

      external_to_internal_map_t ext_to_int_;
      std::set<internal_id_type> _to_be_removed;

      boost::thread executor_;

      external_id_type generate_external_id (void) const
      {
        boost::unique_lock<boost::recursive_mutex> lock (id_gen_mutex_);
        return external_id_gen_();
      }

      internal_id_type generate_internal_id (void) const
      {
        boost::unique_lock<boost::recursive_mutex> lock (id_gen_mutex_);
        return internal_id_gen_();
      }

      void activity_failed (internal_id_type const internal_id);
      void activity_canceled (internal_id_type const internal_id);
      void cancel_activity (internal_id_type const internal_id);
      void insert_activity(const detail::descriptor & desc);
      void remove_activity(const detail::descriptor & desc);
      detail::descriptor& lookup (const internal_id_type& id);
    };
  }
}

#endif
