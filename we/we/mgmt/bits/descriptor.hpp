#ifndef WE_MGMT_LAYER_DESCRIPTOR_HPP
#define WE_MGMT_LAYER_DESCRIPTOR_HPP 1

#include <we/type/id.hpp>
#include <we/mgmt/type/activity.hpp>
#include <we/mgmt/bits/execution_policy.hpp>

#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_set.hpp>

#include <string>

namespace we
{
  namespace mgmt
  {
    namespace detail
    {
      //! \todo What is all this mess good for?
      class descriptor
      {
        typedef boost::lock_guard<boost::recursive_mutex> lock_t;

      public:
        typedef we::mgmt::type::activity_t activity_type;
        typedef petri_net::activity_id_type id_type;
        typedef std::string external_id_type;
        typedef boost::unordered_set<id_type> children_t;
        typedef boost::function<void (const petri_net::activity_id_type&)> fun_t;

        descriptor();
        descriptor (id_type const&, activity_type const&);
        descriptor (id_type const&, activity_type const&, id_type const&);
        descriptor (const descriptor&);

        descriptor& operator= (const descriptor&);

        const std::string& name() const;

        bool came_from_external() const;
        void came_from_external_as (external_id_type const&);

        bool sent_to_external() const;
        void sent_to_external_as (external_id_type const&);

        external_id_type const& from_external_id() const;
        external_id_type const& to_external_id() const;

        bool external() const;
        bool has_parent() const;
        id_type const& id() const;
        id_type const& parent() const;
        activity_type const& activity() const;

        void output (const we::mgmt::type::activity_t::output_t&);

        void inject_input();
        void inject (const descriptor&, fun_t);
        void inject (descriptor const& child);

        void child_failed ( descriptor const&
                          , int error_code
                          , std::string const& error_message
                          );
        void child_cancelled (descriptor const&, std::string const& /*reason*/);

        descriptor extract (id_type const&);

        void cancel (fun_t);
       int execute (policy::execution_policy*);
        void finished();
        void failed();
        void cancelled();

        std::size_t failure_counter() const;
        int error_code() const;
        void set_error_code(int ec);
        std::string const& error_message() const;
        void set_error_message (std::string const&);

        std::string const& result() const;
        void set_result (std::string const&);

        bool has_children() const;
        size_t child_count() const;

        bool is_done() const;
        bool is_alive() const;
        bool enabled() const;

        std::string show_input() const;
        std::string show_output() const;

        void apply_to_children (fun_t) const;

      private:
        void add_child (id_type const&);
        bool is_child (id_type const&);
        void del_child (id_type const&);

      private:
        mutable boost::recursive_mutex mutex_;
        id_type id_;
        activity_type activity_;
        bool has_parent_;
        id_type parent_;
        children_t children_;
        bool from_external_;
        bool to_external_;
        external_id_type from_external_id_;
        external_id_type to_external_id_;
        std::size_t failure_counter_;

        int m_error_code;
        std::string m_error_message;
        std::string m_result;

        friend std::ostream& operator<< (std::ostream&, const descriptor&);
      };

      std::ostream& operator<< (std::ostream&, const descriptor&);
    }
  }
}

#endif
