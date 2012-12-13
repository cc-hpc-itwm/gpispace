// mirko.rahn@itwm.fhg.de

#include <we/mgmt/bits/descriptor.hpp>

#include <fhg/util/show.hpp>

//! \todo Why is this include needed really?
#include <we/net.hpp>

#include <boost/lexical_cast.hpp>

#include <algorithm>

namespace we
{
  namespace mgmt
  {
    namespace detail
    {
      descriptor::descriptor()
        : has_parent_(false)
        , from_external_(false)
        , to_external_(false)
        , failure_counter_(0)
        , m_error_code(0)
        , m_error_message()
        , m_result()
      {}

      descriptor::descriptor ( id_type const& a_id
                             , activity_type const& a_activity
                             )
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

      descriptor::descriptor ( id_type const& a_id
                             , activity_type const& a_activity
                             , id_type const& a_parent
                             )
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

      descriptor::descriptor (const descriptor& other)
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
      {}

      descriptor& descriptor::operator= (const descriptor& other)
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

      const std::string& descriptor::name() const
      {
        lock_t lock(mutex_);
        return activity_.transition().name();
      }

      bool descriptor::came_from_external() const
      {
        lock_t lock(mutex_);
        return from_external_;
      }

      void descriptor::came_from_external_as (external_id_type const& ext_id)
      {
        lock_t lock(mutex_);
        from_external_ = true;
        from_external_id_ = ext_id;
      }

      bool descriptor::sent_to_external() const
      {
        lock_t lock(mutex_);
        return to_external_;
      }

      void descriptor::sent_to_external_as (external_id_type const& ext_id)
      {
        lock_t lock(mutex_);
        to_external_ = true;
        to_external_id_ = ext_id;
      }

      descriptor::external_id_type const& descriptor::from_external_id() const
      {
        lock_t lock(mutex_);
        return from_external_id_;
      }

      descriptor::external_id_type const& descriptor::to_external_id() const
      {
        lock_t lock(mutex_);
        return to_external_id_;
      }

      bool descriptor::external() const
      {
        lock_t lock(mutex_);
        return came_from_external() || sent_to_external();
      }

      bool descriptor::has_parent() const
      {
        lock_t lock(mutex_);
        return has_parent_;
      }

      descriptor::id_type const& descriptor::id() const
      {
        lock_t lock(mutex_);
        return id_;
      }

      descriptor::id_type const& descriptor::parent() const
      {
        lock_t lock(mutex_);
        return parent_;
      }

      descriptor::activity_type const& descriptor::activity() const
      {
        lock_t lock(mutex_);
        return activity_;
      }

      void descriptor::output (const we::mgmt::type::activity_t::output_t& o)
      {
        lock_t lock(mutex_);
        activity_.set_output (o);
      }

      void descriptor::inject_input()
      {
        lock_t lock(mutex_);
        activity_.inject_input();
      }

      void descriptor::inject (const descriptor& child, fun_t cb)
      {
        lock_t lock(mutex_);
        this->inject (child);
        cb (id());
      }

      void descriptor::inject (descriptor const& child)
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

      void descriptor::child_failed ( descriptor const& child
                                    , int error_code
                                    , std::string const& error_message
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

      void descriptor::child_cancelled ( descriptor const& child
                                       , std::string const& /*reason*/
                                       )
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

      descriptor descriptor::extract (id_type const& child_id)
      {
        lock_t lock(mutex_);

        descriptor child (child_id, activity_.extract(), id());

        add_child (child_id);

        return child;
      }

      void descriptor::cancel (fun_t f)
      {
        lock_t lock(mutex_);
        activity_.set_cancelling (true);
        apply_to_children (f);
      }

      int descriptor::execute (policy::execution_policy* c)
      {
        lock_t lock(mutex_);
        return activity_.execute (c);
      }

      void descriptor::finished()
      {
        lock_t lock(mutex_);
        activity_.collect_output();
        activity_.set_failed (false);
        activity_.set_finished (true);
      }

      void descriptor::failed()
      {
        lock_t lock(mutex_);
        activity_.collect_output();
        activity_.set_failed (true);
        activity_.set_finished (false);
        ++failure_counter_;
      }

      void descriptor::cancelled()
      {
        lock_t lock(mutex_);
        activity_.collect_output();
        activity_.set_cancelled (true);
        activity_.set_finished (false);
      }

      std::size_t descriptor::failure_counter() const
      {
        lock_t lock(mutex_);
        return failure_counter_;
      }

      int descriptor::error_code() const
      {
        lock_t lock(mutex_);
        return m_error_code;
      }

      void descriptor::set_error_code(int ec)
      {
        lock_t lock(mutex_);
        m_error_code = ec;
      }

      std::string const& descriptor::error_message() const
      {
        lock_t lock(mutex_);
        return m_error_message;
      }

      void descriptor::set_error_message (std::string const&msg)
      {
        lock_t lock(mutex_);
        m_error_message = msg;
      }

      std::string const& descriptor::result() const
      {
        lock_t lock(mutex_);
        return m_result;
      }

      void descriptor::set_result (std::string const&result)
      {
        lock_t lock(mutex_);
        m_result = result;
      }

      bool descriptor::has_children() const
      {
        lock_t lock(mutex_);
        return ! children_.empty();
      }

      size_t descriptor::child_count() const
      {
        lock_t lock(mutex_);
        return children_.size();
      }

      // checks if a network is finished
      //     - all children have been injected
      //     - no more children can be produced
      bool descriptor::is_done() const
      {
        lock_t lock(mutex_);
        return (! activity_.can_fire())&& (children_.size() == 0);
      }

      bool descriptor::is_alive() const
      {
        lock_t lock(mutex_);
        return activity_.is_alive();
      }

      bool descriptor::enabled() const
      {
        lock_t lock(mutex_);
        return activity_.can_fire();
      }

      std::string descriptor::show_input() const
      {
        lock_t lock (mutex_);
        return fhg::util::show (activity_.input().begin(), activity_.input().end());
      }

      std::string descriptor::show_output() const
      {
        lock_t lock (mutex_);
        return fhg::util::show (activity_.output().begin(), activity_.output().end());
      }

      void descriptor::apply_to_children (fun_t f) const
      {
        lock_t lock (mutex_);
        std::for_each (children_.begin(), children_.end(), f);
      }

      void descriptor::add_child (id_type const& child)
      {
        lock_t lock(mutex_);
        children_.insert(child);
      }

      bool descriptor::is_child (id_type const& child)
      {
        lock_t lock(mutex_);
        return children_.find(child) != children_.end();
      }

      void descriptor::del_child (id_type const& child)
      {
        lock_t lock(mutex_);
        children_.erase (child);
      }

      std::ostream& operator<< (std::ostream& p, const descriptor& d)
      {
        descriptor::lock_t lock(d.mutex_);

        p << "descriptor [" << d.id() << "]:" << std::endl;
        p << "         name := " << d.activity_.transition().name() << std::endl;
        p << std::boolalpha;
        p << "       parent := ";
        if (d.has_parent())      p << d.parent();
        else                   p << "n/a";
        p << std::endl;
        p << "     children := " << fhg::util::show (d.children_.begin(), d.children_.end()) << std::endl;
        p << "     internal := "
          << d.activity_.transition().is_internal()
          << std::endl;
        p << "     can_fire := "
          << d.activity_.can_fire()
          << std::endl;
        p << "         done := "
          << d.is_done()
          << std::endl;
        if (d.from_external_)
          {
            p << "     from-ext := " << d.from_external_id_ << std::endl;
          }
        if (d.to_external_)
          {
            p << "       to-ext := " << d.to_external_id_ << std::endl;
          }
        p << std::noboolalpha;
        p << "         type := " << d.activity_.type_to_string() << std::endl;
        p << "        input := "; d.activity_.print (p, d.activity_.input()); p << std::endl;
        p << "       output := "; d.activity_.print (p, d.activity_.output()); p << std::endl;

        return p;
      }
    }
  }
}
