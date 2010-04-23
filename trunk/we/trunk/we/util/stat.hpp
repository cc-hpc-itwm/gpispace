// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_UTIL_STAT_HPP
#define _WE_UTIL_STAT_HPP

#include <map>
#include <sys/time.h>
#include <iostream>
#include <iomanip>

namespace statistic
{
  static inline double current_time()
  {
    struct timeval tv;

    gettimeofday (&tv, NULL);

    return (double(tv.tv_sec) + double (tv.tv_usec) * 1E-6);
  }

  template<typename T>
  struct by_first
  {
    bool operator () (const T & a, const T & b)
    {
      return a < b;
    }
  };

  template<typename T>
  struct by_second
  {
    bool operator () (const T & a, const T & b)
    {
      return a.second < b.second;
    }
  };

  template<typename T>
  struct by_ratio
  {
    bool operator () (const T & a, const T & b)
    {
      return a.second / a.first > b.second / b.first;
    }
  };

  typedef unsigned long cnt_t;
  typedef double time_t;
  typedef std::pair<cnt_t,time_t> elem_t;

  using std::cout;
  using std::endl;

  struct elem_out_t
  {
  private:
    cnt_t csum;
    time_t tsum;

  public:
    elem_out_t () : csum (0), tsum (0) {}

    void operator () (const elem_t & x)
    {
      const cnt_t c (x.first);
      const time_t t (x.second);

      tsum += t;
      csum += c;
      
      cout << " " << std::fixed << std::setprecision(5) << std::setw(8) << t;
      cout << " " << std::fixed << std::setprecision(5) << std::setw(8) << tsum;
      cout << " " << std::setw(8) << c;
      cout << " " << std::setw(8) << csum;
      cout << " " << std::fixed << std::setprecision(5) << std::setw(8) << (1000 * t / c);
    }
  };

  template<typename T>
  struct loud
  {
  private:
    typedef std::vector<T> key_t;
    typedef std::map<key_t,elem_t> stat_t;

    stat_t f, r;

    typedef std::vector<key_t> key_vec_t;
    typedef std::map<elem_t, key_vec_t, by_first<elem_t> > by_cnt_t;
    typedef std::map<elem_t, key_vec_t, by_second<elem_t> > by_time_t;
    typedef std::map<elem_t, key_vec_t, by_ratio<elem_t> > by_ratio_t;

    template<typename MAP>
    void out_by ( const MAP & m
                , const std::string & descr
                , const std::string & msg
                ) const
    {
      cout << endl << "### " << descr << " by " << msg << ":" << endl;
     
      elem_out_t elem_out;

      for ( typename MAP::const_iterator pos (m.begin())
          ; pos != m.end()
          ; ++pos
          )
        {
          for ( typename key_vec_t::const_iterator kvec (pos->second.begin())
              ; kvec != pos->second.end()
              ; ++kvec
              )
            {
              elem_out (pos->first);

              for ( typename key_t::const_iterator key (kvec->begin())
                  ; key != kvec->end()
                  ; ++key
                  )
                cout << ((key != kvec->begin()) ? "," : "") << " " << *key;

              cout << endl;
            }
        }
    };

    void out (const stat_t & s, const std::string & descr = "STAT") const
    {
      by_cnt_t by_cnt;
      by_time_t by_time;
      by_ratio_t by_ratio;

      {
        cout << endl << "### " << descr << ":" << endl;

        elem_out_t elem_out;

        for ( typename stat_t::const_iterator pos (s.begin())
            ; pos != s.end()
            ; ++pos
            )
          {
            by_cnt[pos->second].push_back (pos->first);
            by_time[pos->second].push_back (pos->first);
            by_ratio[pos->second].push_back (pos->first);

            elem_out (pos->second);

            for ( typename key_t::const_iterator key (pos->first.begin())
                ; key != pos->first.end()
                ; ++key
                )
              cout << ((key != pos->first.begin()) ? "," : "") << " " << *key;

            cout << endl;
          }
      }

      out_by (by_cnt, descr, "cnt");
      out_by (by_time, descr, "time");
      out_by (by_ratio, descr, "ratio");
    }

    void start (const key_t & key, stat_t & s)
    {
      typename stat_t::iterator pos (s.find (key));

      if (pos == s.end())
        {
          elem_t & e (s[key]);

          e.first = 1;
          e.second = -current_time();
        }
      else
        {
          elem_t & e (pos->second);

          ++e.first;
          e.second -= current_time();
        }
    }

    void stop (const key_t & key, stat_t & s)
    {
      typename stat_t::iterator pos (s.find (key));

      if (pos == s.end())
        throw std::runtime_error ("stop before start!?");

      elem_t & e (pos->second);

      e.second += current_time();
    }

  public:
    void out (const std::string & msg) const
    {
      out (f, msg + " STAT by first level");
      out (r, msg + " STAT by second level");
    }

    void start (const T & t1, const T & t2)
    {
      { key_t v; v.push_back (t1); v.push_back (t2); start (v, f); }
      { key_t v; v.push_back (t2); v.push_back (t1); start (v, r); }
    }

    void stop (const T & t1, const T & t2)
    {
      { key_t v; v.push_back (t1); v.push_back (t2); stop (v, f); }
      { key_t v; v.push_back (t2); v.push_back (t1); stop (v, r); }
    }
  };

  template<typename T>
  struct muted
  {
  public:
    inline void start (const T &, const T &) {}
    inline void stop (const T &, const T &) {}
    inline void out (const std::string &) {}
  };
}

#endif
