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
        typedef T rank_type;
        typedef rank_type value_type;
        typedef rank_type argument_type;

        typedef boost::unordered_set<value_type> exclude_set_type;
        typedef std::vector<value_type> rank_list_type;

        explicit
        preference_t (const bool _mandatory = false)
          : mandatory_(_mandatory)
        { }

        virtual ~preference_t () {}

        yes_no_may can (const rank_type rank) const
        {
          if (is_mandatory())
          {
            if (is_wanted (rank)) return yes_no_may::YES;
            return yes_no_may::NO;
          }
          else
          {
            if (is_wanted (rank)) return yes_no_may::YES;
            if (is_excluded (rank)) return yes_no_may::NO;
            return yes_no_may::MAY;
          }
        }

        bool operator () (const rank_type rank) const
        {
          return can(rank);
        }

        virtual bool is_mandatory (void) const
        {
          return mandatory_;
        }

        bool is_wanted (const rank_type rank) const
        {
          return ( std::find ( ranks_.begin()
                             , ranks_.end()
                             , rank
                             ) != ranks_.end()
                 );
        }

        preference_t & want (const rank_type rank)
        {
          ranks_.push_back (rank);
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

        bool is_excluded (rank_type rank) const
        {
          return excluded_ranks_.find(rank) != excluded_ranks_.end();
        }

        preference_t & cant (const rank_type rank)
        {
          excluded_ranks_.insert (rank);
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

        preference_t & operator+= (rank_type const & r)
        {
          return want (r);
        }

        preference_t & operator-= (rank_type const & r)
        {
          return cant (r);
        }

        rank_list_type & ranks(void)
        {
          return ranks_;
        }

        rank_list_type const & ranks(void) const
        {
          return ranks_;
        }

        exclude_set_type & exclusion (void)
        {
          return excluded_ranks_;
        }

        exclude_set_type const & exclusion (void) const
        {
          return excluded_ranks_;
        }

        bool empty (void) const
        {
          return ranks_.empty();
        }

      private:
        bool mandatory_;
        rank_list_type ranks_;
        exclude_set_type excluded_ranks_;
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

        os << "{ranks, ";
        os << fhg::util::show (p.ranks().begin(), p.ranks().end());
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
