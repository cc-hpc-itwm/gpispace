#ifndef WE_MGMT_LAYER_DESCRIPTOR_HPP
#define WE_MGMT_LAYER_DESCRIPTOR_HPP 1

#include <we/type/id.hpp>
#include <we/mgmt/type/activity.hpp>

#include <fhg/util/show.hpp>

#include <boost/thread.hpp>
#include <boost/unordered_set.hpp>
#include <boost/lexical_cast.hpp>

#include <string>
#include <algorithm>

namespace we
{
  namespace mgmt
  {
    namespace detail
    {
      class descriptor
      {
        typedef boost::lock_guard<boost::recursive_mutex> lock_t;

      public:
        typedef we::mgmt::type::activity_t activity_type;
        typedef petri_net::activity_id_type id_type;
        typedef std::string external_id_type;
        typedef boost::unordered_set<id_type> children_t;

        descriptor ()
          : has_parent_(false)
          , from_external_(false)
          , to_external_(false)
          , failure_counter_(0)
          , m_error_code(0)
          , m_error_message()
          , m_result()
        { }

        descriptor (id_type const & a_id, activity_type const & a_activity)
          : id_(a_id)
          , activity_(a_activity)
          , has_parent_(false)
          , from_external_(false)
          , to_external_(false)
          , failure_counter_(0)
          , m_error_code(0)
          , m_error_message()
          , m_result()
        {
          activity_.set_id (a_id);
        }

        descriptor (id_type const & a_id, activity_type const & a_activity, id_type const & a_parent)
          : id_(a_id)
          , activity_(a_activity)
          , has_parent_(true)
          , parent_(a_parent)
          , from_external_(false)
          , to_external_(false)
          , failure_counter_(0)
          , m_error_code(0)
          , m_error_message()
          , m_result()
        {
          activity_.set_id (a_id);
        }

        descriptor (const descriptor & other)
          : id_(other.id_)
          , activity_(other.activity_)
          , has_parent_(other.has_parent_)
          , parent_(other.parent_)
          , children_(other.children_)
          , from_external_(other.from_external_)
          , to_external_(other.to_external_)
          , from_external_id_(other.from_external_id_)
          , to_external_id_(other.to_external_id_)
          , failure_counter_(other.failure_counter_)
          , m_error_code(other.m_error_code)
          , m_error_message(other.m_error_message)
          , m_result(other.m_result)
        { }

        descriptor & operator= (const descriptor & other)
        {
          if (this != &other)
          {
            lock_t lock1(mutex_);
            lock_t lock2(other.mutex_);

            id_ = other.id_;
            activity_ = other.activity_;
            has_parent_ = other.has_parent_;
            parent_ = other.parent_;
            children_ = other.children_;
            from_external_ = other.from_external_;
            to_external_ = other.to_external_;
            from_external_id_ = other.from_external_id_;
            to_external_id_ = other.to_external_id_;
            failure_counter_ = other.failure_counter_;
            m_error_code = other.m_error_code;
            m_error_message = other.m_error_message;
            m_result = other.m_result;
          }
          return *this;
        }

        const std::string & name () const
        {
          lock_t lock(mutex_);
          return activity_.transition().name();
        }

        bool came_from_external () const
        {
          lock_t lock(mutex_);
          return from_external_;
        }

        void came_from_external_as (external_id_type const & ext_id)
        {
          lock_t lock(mutex_);
          from_external_ = true;
          from_external_id_ = ext_id;
        }

        bool sent_to_external () const
        {
          lock_t lock(mutex_);
          return to_external_;
        }

        void sent_to_external_as (external_id_type const & ext_id)
        {
          lock_t lock(mutex_);
          to_external_ = true;
          to_external_id_ = ext_id;
        }

        external_id_type const & from_external_id () const
        {
          lock_t lock(mutex_);
          return from_external_id_;
        }

        external_id_type const & to_external_id () const
        {
          lock_t lock(mutex_);
          return to_external_id_;
        }

        bool external() const
        {
          lock_t lock(mutex_);
          return came_from_external() || sent_to_external();
        }

        bool has_parent () const
        {
          lock_t lock(mutex_);
          return has_parent_;
        }

        id_type const & id() const
        {
          lock_t lock(mutex_);
          return id_;
        }

        id_type const & parent() const
        {
          lock_t lock(mutex_);
          return parent_;
        }

        activity_type const & activity() const
        {
          lock_t lock(mutex_);
          return activity_;
        }

        template <typename Output>
        void output (Output o)
        {
          lock_t lock(mutex_);
          activity_.set_output (o);
        }

        void inject_input ()
        {
          lock_t lock(mutex_);
          activity_.inject_input ();
        }

        template <typename F>
        void inject (descriptor const & child, F cb)
        {
          lock_t lock(mutex_);
          this->inject (child);
          cb ( id() );
        }

        void inject (descriptor const & child)
        {
          lock_t lock(mutex_);
          if (! is_child (child.id()))
            throw std::runtime_error ( "Tried to inject '"
                                     + boost::lexical_cast<std::string>(child)
                                     + "' into '"
                                     + boost::lexical_cast<std::string>(*this)
                                     + "' which is not my child!"
                                     );
          activity_.inject (child.activity());
          del_child (child.id());
        }

        void child_failed ( descriptor const & child
                          , int error_code
                          , std::string const & error_message
                          )
        {
          lock_t lock(mutex_);
          if (! is_child (child.id()))
            throw std::runtime_error ( "Tried to notify child failure '"
                                     + boost::lexical_cast<std::string>(child)
                                     + "' to '"
                                     + boost::lexical_cast<std::string>(*this)
                                     + "' which is not my child!"
                                     );
          del_child (child.id());

          // TODO: add to log of tuples: (childname, code, message)?
          set_error_code(error_code);
          set_error_message(error_message);
        }

        void child_cancelled (descriptor const & child, std::string const & /*reason*/)
        {
          lock_t lock(mutex_);
          if (! is_child (child.id()))
            throw std::runtime_error ( "Tried to notify child cancellation '"
                                     + boost::lexical_cast<std::string>(child)
                                     + "' to '"
                                     + boost::lexical_cast<std::string>(*this)
                                     + "' which is not my child!"
                                     );
          del_child (child.id());
        }

        descriptor extract (id_type const & child_id)
        {
          lock_t lock(mutex_);

          descriptor child ( child_id
                           , activity_.extract()
                           , id()
                           );
          add_child (child_id);
          return child;
        }

        template <typename Fun>
        void cancel (Fun f)
        {
          lock_t lock(mutex_);
          activity_.set_cancelling (true);
          apply_to_children (f);
        }

        template <typename C>
        typename C::result_type execute (C c)
        {
          lock_t lock(mutex_);
          return activity_.execute (c);
        }

        void finished ()
        {
          lock_t lock(mutex_);
          activity_.collect_output();
          activity_.set_failed (false);
          activity_.set_finished (true);
        }

        void failed ()
        {
          lock_t lock(mutex_);
          activity_.collect_output();
          activity_.set_failed (true);
          activity_.set_finished (false);
          ++failure_counter_;
        }

        void cancelled ()
        {
          lock_t lock(mutex_);
          activity_.collect_output();
          activity_.set_cancelled (true);
          activity_.set_finished (false);
        }

        std::size_t failure_counter () const
        {
          lock_t lock(mutex_);
          return failure_counter_;
        }

        int error_code () const
        {
          lock_t lock(mutex_);
          return m_error_code;
        }

        void set_error_code(int ec)
        {
          lock_t lock(mutex_);
          m_error_code = ec;
        }

        std::string const & error_message () const
        {
          lock_t lock(mutex_);
          return m_error_message;
        }

        void set_error_message (std::string const &msg)
        {
          lock_t lock(mutex_);
          m_error_message = msg;
        }

        std::string const & result () const
        {
          lock_t lock(mutex_);
          return m_result;
        }

        void set_result (std::string const &result)
        {
          lock_t lock(mutex_);
          m_result = result;
        }

        bool has_children () const
        {
          lock_t lock(mutex_);
          return ! children_.empty();
        }

        size_t child_count () const
        {
          lock_t lock(mutex_);
          return children_.size();
        }

        // checks if a network is finished
        //     - all children have been injected
        //     - no more children can be produced
        bool is_done () const
        {
          lock_t lock(mutex_);
          return (! activity_.can_fire()) && (children_.size() == 0);
        }

        inline
        bool is_alive () const
        {
          lock_t lock(mutex_);
          return activity_.is_alive();
        }

        inline
        bool enabled () const
        {
          lock_t lock(mutex_);
          return activity_.can_fire();
        }

        std::ostream & operator << (std::ostream &p) const
        {
          lock_t lock(mutex_);

          p << "descriptor [" << id() << "]:" << std::endl;
          p << "         name := " << activity_.transition().name() << std::endl;
          p << std::boolalpha;
          p << "       parent := ";
          if (has_parent())      p << parent();
          else                   p << "n/a";
          p << std::endl;
          p << "     children := " << fhg::util::show (children_.begin(), children_.end()) << std::endl;
          p << "     internal := "
            << activity_.transition().is_internal()
            << std::endl;
          p << "     can_fire := "
            << activity_.can_fire()
            << std::endl;
          p << "         done := "
            << is_done()
            << std::endl;
          if (from_external_)
          {
            p << "     from-ext := " << from_external_id_ << std::endl;
          }
          if (to_external_)
          {
            p << "       to-ext := " << to_external_id_ << std::endl;
          }
          p << std::noboolalpha;
          p << "         type := " << activity_.type_to_string () << std::endl;
          p << "        input := "; activity_.print (p, activity_.input()); p << std::endl;
          p << "       output := "; activity_.print (p, activity_.output()); p << std::endl;

          return p;
        }

        std::string show_input() const
        {
          lock_t lock (mutex_);
          return fhg::util::show (activity_.input().begin(), activity_.input().end());
        }

        std::string show_output() const
        {
          lock_t lock (mutex_);
          return fhg::util::show (activity_.output().begin(), activity_.output().end());
        }

        template <typename Fun>
        void apply_to_children (Fun f) const
        {
          lock_t lock (mutex_);
          std::for_each (children_.begin(), children_.end(), f);
        }

      private:
        void add_child (id_type const & child)
        {
          lock_t lock(mutex_);
          children_.insert(child);
        }

        bool is_child (id_type const & child)
        {
          lock_t lock(mutex_);
          return children_.find(child) != children_.end();
        }

        void del_child (id_type const & child)
        {
          lock_t lock(mutex_);
          children_.erase (child);
        }
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
      };

      inline std::ostream& operator<< (std::ostream& os, const descriptor& d)
      {
        return d.operator<< (os);
      }
    }
  }
}

#endif
