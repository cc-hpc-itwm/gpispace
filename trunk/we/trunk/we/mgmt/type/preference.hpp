#ifndef WE_MGMT_TYPE_PREFERENCE_HPP
#define WE_MGMT_TYPE_PREFERENCE_HPP 1

#include <vector>
#include <ostream>
#include <boost/unordered_set.hpp>
#include <fhg/util/show.hpp>

namespace we
{
  namespace mgmt
  {
    namespace pref
    {
      struct yes_no_may
      {
        enum YNM
        {
          NO = 0
        , YES
        , MAY
        };

        yes_no_may ()
          : value_ (NO)
        { }

        // no - 0
        // yes - 1
        // may - 2
        yes_no_may (const YNM v)
          : value_ (v)
        { }

        bool yes (void) const
        {
          return value_ == YES;
        }

        bool no (void) const
        {
          return value_ == NO;
        }

        bool may (void) const
        {
          return value_ == MAY;
        }

        void set (const YNM v)
        {
          value_ = v;
        }

        operator bool ()
        {
          return yes() || may();
        }

      private:
        YNM value_;
      };

      inline
      std::ostream & operator << (std::ostream & os, const yes_no_may & ynm)
      {
        if (ynm.yes())
          os << "yes";
        if (ynm.no())
          os << "no";
        if (ynm.may())
          os << "may";

        return os;
      }

      inline
      std::istream & operator >> (std::istream & is, yes_no_may & ynm)
      {
        std::string s;
        is >> s;
        if (s == "yes") ynm.set(yes_no_may::YES);
        if (s == "no") ynm.set(yes_no_may::NO);
        if (s == "may") ynm.set(yes_no_may::MAY);
        return is;
      }

      template <typename T>
      struct preference_t
      {
        typedef T value_type;
        typedef value_type argument_type;

        typedef boost::unordered_set<value_type> exclude_set_type;
        typedef std::vector<value_type> value_list_type;

        explicit
        preference_t (const bool _mandatory = false)
          : mandatory_(_mandatory)
        { }

        virtual ~preference_t () {}

        yes_no_may can (const value_type val) const
        {
          if (is_mandatory())
          {
            if (is_wanted (val)) return yes_no_may::YES;
            return yes_no_may::NO;
          }
          else
          {
            if (is_wanted (val)) return yes_no_may::YES;
            if (is_excluded (val)) return yes_no_may::NO;
            return yes_no_may::MAY;
          }
        }

        bool operator () (const value_type val) const
        {
          return can(val);
        }

        virtual bool is_mandatory (void) const
        {
          return mandatory_;
        }

        bool is_wanted (const value_type val) const
        {
          return ( std::find ( values_.begin()
                             , values_.end()
                             , val
                             ) != values_.end()
                 );
        }

        preference_t & want (const value_type val)
        {
          values_.push_back (val);
          return *this;
        }

        template <typename ForwardIterator>
        preference_t & want (ForwardIterator first, ForwardIterator last)
        {
          while (first != last)
          {
            want (*first);
            ++first;
          }
          return *this;
        }

        bool is_excluded (value_type val) const
        {
          return excluded_values_.find(val) != excluded_values_.end();
        }

        preference_t & cant (const value_type val)
        {
          excluded_values_.insert (val);
          return *this;
        }

        template <typename ForwardIterator>
        preference_t & cant (ForwardIterator first, ForwardIterator last)
        {
          while (first != last)
          {
            cant (*first);
            ++first;
          }
          return *this;
        }

        preference_t & operator+= (value_type const & r)
        {
          return want (r);
        }

        preference_t & operator-= (value_type const & r)
        {
          return cant (r);
        }

        value_list_type & values(void)
        {
          return values_;
        }

        value_list_type const & values(void) const
        {
          return values_;
        }

        exclude_set_type & exclusion (void)
        {
          return excluded_values_;
        }

        exclude_set_type const & exclusion (void) const
        {
          return excluded_values_;
        }

        bool empty (void) const
        {
          return values_.empty();
        }

      private:
        bool mandatory_;
        value_list_type values_;
        exclude_set_type excluded_values_;
      };

      template <typename T>
      preference_t<T> make_mandatory ()
      {
        return preference_t<T> (true);
      }

      template <typename T>
      preference_t<T> make_preferred ()
      {
        return preference_t<T> (false);
      }

      template <typename T>
      inline
      std::ostream & operator << (std::ostream & os, const preference_t<T> & p)
      {
        os << "{pref, ";

        if (p.is_mandatory())
          os << "must";
        else
          os << "want";
        os << ", ";

        os << "{values, ";
        os << fhg::util::show (p.values().begin(), p.values().end());
        os << "}";

        if (! p.is_mandatory())
        {
          os << ", ";
          os << "{cant, ";
          os << fhg::util::show (p.exclusion().begin(), p.exclusion().end());
          os << "}";
        }

        os << "}";

        return os;
      }
    }
  }
}

#endif
