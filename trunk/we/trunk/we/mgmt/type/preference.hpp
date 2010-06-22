#ifndef WE_MGMT_TYPE_PREFERENCE_HPP
#define WE_MGMT_TYPE_PREFERENCE_HPP 1

#include <vector>
#include <ostream>
#include <boost/unordered_set.hpp>
#include <we/util/show.hpp>

namespace we
{
  namespace mgmt
  {
    namespace pref
    {
      struct mandatory {};
      struct preferred {};
      struct yes_no_maybe
      {
        yes_no_maybe ()
          : value_ (0)
        { }

        // no - 0
        // yes - 1
        // maybe - 2
        explicit
        yes_no_maybe (const int i)
          : value_ (i)
        { }

        bool yes (void) const
        {
          return value_ == 1;
        }

        bool no (void) const
        {
          return value_ == 0;
        }

        bool maybe (void) const
        {
          return value_ == 2;
        }

        void set (int v)
        {
          value_ = v;
        }

      private:
        int value_;
      };

      inline
      std::ostream & operator << (std::ostream & os, const yes_no_maybe & ynm)
      {
        if (ynm.yes())
          os << "yes";
        if (ynm.no())
          os << "no";
        if (ynm.maybe())
          os << "maybe";

        return os;
      }
      inline
      std::istream & operator >> (std::istream & is, yes_no_maybe & ynm)
      {
        std::string s;
        is >> s;
        if (s == "yes") ynm.set(1);
        if (s == "no") ynm.set(0);
        if (s == "maybe") ynm.set(2);
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

        preference_t ()
          : mandatory_(false)
        {}

        explicit
        preference_t (const bool _mandatory)
          : mandatory_(_mandatory)
        {}

        yes_no_maybe can (const rank_type rank) const
        {
          if (mandatory_)
          {
            if (is_wanted (rank)) return yes_no_maybe(1);
            return yes_no_maybe(0);
          }
          else
          {
            if (is_wanted (rank)) return yes_no_maybe(1);
            if (is_excluded (rank)) return yes_no_maybe(0);
            return yes_no_maybe(2);
          }
        }

        bool operator () (const rank_type rank) const
        {
          const yes_no_maybe ynm (can(rank));
          if (mandatory_)
            return ynm.yes();
          else
            return ynm.yes() || ynm.maybe();
        }

        bool is_mandatory (void) const
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

        /*
        template <typename InputIterator>
        preference_t & want (InputIterator first, InputIterator last)
        {
          while (first != last)
          {
            want (*first);
            ++first;
          }
          return *this;
        }
        */

        bool is_excluded (rank_type rank) const
        {
          return excluded_ranks_.find(rank) != excluded_ranks_.end();
        }

        preference_t & cant (const rank_type rank)
        {
          excluded_ranks_.insert (rank);
          return *this;
        }

        template <typename InputIterator>
        preference_t & cant (InputIterator first, InputIterator last)
        {
          while (first != last)
          {
            cant (*first);
            ++first;
          }
          return *this;
        }

        preference_t & operator+= (const rank_type r)
        {
          return want (r);
        }

        preference_t & operator-= (const rank_type r)
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
      inline
      std::ostream & operator << (std::ostream & os, const preference_t<T> & p)
      {
        os << "{pref, ";

        if (p.is_mandatory())
          os << "{must, ";
        else
          os << "{want, ";
        os << ::util::show (p.ranks().begin(), p.ranks().end());
        os << "}, ";

        os << "{cant, ";
        os << ::util::show (p.exclusion().begin(), p.exclusion().end());
        os << "}";
        os << "}";

        return os;
      }
    }
  }
}

#endif
