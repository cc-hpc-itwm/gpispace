#ifndef WE_MGMT_LAYER_DESCRIPTOR_HPP
#define WE_MGMT_LAYER_DESCRIPTOR_HPP 1

#include <boost/thread.hpp>
#include <algorithm>

namespace we
{
  namespace mgmt
  {
    namespace detail
    {
      template <typename Id, typename Activity>
      class descriptor
      {
        typedef boost::unique_lock<boost::recursive_mutex> lock_t;

      public:
        typedef Id id_type;
        typedef Activity activity_type;
        typedef std::vector<id_type> children_t;

        descriptor (id_type const & a_id, activity_type const & a_activity)
          : id_(a_id)
          , activity_(a_activity)
          , has_parent_(false)
        { }

        descriptor (id_type const & a_id, activity_type const & a_activity, id_type const & a_parent)
          : id_(a_id)
          , activity_(a_activity)
          , has_parent_(true)
          , parent_(a_parent)
        { }

        descriptor (const descriptor & other)
          : id_(other.id_)
          , activity_(other.activity_)
          , has_parent_(other.has_parent_)
          , parent_(other.parent_)
          , children_(other.children_)
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
          }
          return *this;
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

        activity_type & activity()
        {
          lock_t lock(mutex_);
          return activity_;
        }

        activity_type const & activity() const
        {
          lock_t lock(mutex_);
          return activity_;
        }

        void add_child (id_type const & child)
        {
          lock_t lock(mutex_);
          children_.push_back (child);
        }

        void del_child (id_type const & child)
        {
          lock_t lock(mutex_);
          children_.erase ( std::find ( children_.begin()
                                      , children_.end()
                                      , child
                                      )
                          );
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
        bool done () const
        {
          lock_t lock(mutex_);
          return (! activity_.has_enabled()) && (children_.size() == 0);
        }

        std::ostream & operator << (std::ostream &s) const
        {
          lock_t lock(mutex_);

          we::mgmt::type::detail::printer <activity_type> p (activity_, s);
          p << "   **** activity [" << id() << "]:" << std::endl;
          p << "         name := " << activity_.transition().name() << std::endl;
          p << std::boolalpha;

          p << "     internal := "
            << activity_.transition().is_internal()
            << std::endl;
          p << "      enabled := "
            << done()
            << std::endl;
          p << std::noboolalpha;
          p << std::endl;

          p << "         type := " << activity_.type_to_string () << std::endl;
          p << "        input := " << activity_.input() << std::endl;
          p << "       output := " << activity_.output() << std::endl;
          p << "       parent := ";
          if (has_parent())      p << parent();
          else                   p << "n/a";
          p << std::endl;

          p << "     children := [";
          for ( typename children_t::const_iterator child (children_.begin())
              ; child != children_.end()
              ; ++child
              )
          {
            if (child != children_.begin())
            {
              p << ", ";
            }
            p << *child;
          }
          p << "]";

          p << std::endl;

          return s;
        }

        template <typename Fun>
        void apply_to_children (Fun f) const
        {
          lock_t lock (mutex_);
          std::for_each (children_.begin(), children_.end(), f);
        }
      private:
        id_type id_;
        activity_type activity_;
        const bool has_parent_;
        id_type parent_;
        children_t children_;
        mutable boost::recursive_mutex mutex_;
      };

      template <typename I, typename A>
      std::ostream & operator << (std::ostream & os, const descriptor<I,A> & d)
      {
        return d.operator<< (os);
      }
    }
  }
}

#endif
