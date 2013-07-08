#include <algorithm>
#include <list>
#include <iostream>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>

#include <sys/time.h>

namespace
{
  class interval
  {
  public:
    interval (int begin, int end)
      : _begin (begin)
      , _end (end)
    {
      if (end < begin)
      {
        throw std::runtime_error
          ((boost::format ("he, nein, [%1%..%2%)??") % begin % end).str());
      }
    }
    int begin() const
    {
      return _begin;
    }
    int end() const
    {
      return _end;
    }
    int set_begin (int begin)
    {
      return _begin = begin;
    }
    int set_end (int end)
    {
      return _end = end;
    }
    int intersects (const interval& x) const
    {
      return end() >= x.begin() && x.end() >= begin();
    }
    bool operator== (const interval& other) const
    {
      return other.begin() == begin() && other.end() == end();
    }
  private:
    int _begin;
    int _end;
  };

  std::ostream& operator<< (std::ostream& os, const interval& i)
  {
    return os << "[" << i.begin() << ".." << i.end() << ")";
  }

  class silist // sorted interval list
  {
  public:
    void insert (const interval& i)
    {
      _.insert (std::upper_bound ( _.begin()
                                 , _.end()
                                 , i
                                 , boost::bind (&interval::begin, _1)
                                 < boost::bind (&interval::begin, _2)
                                 )
               , i
               );
    }
    const std::CONTAINER<interval>& elements() const
    {
      return _;
    }
  protected:
    std::CONTAINER<interval> _;
  };

  class smilist : public silist // sorted merged interval list
  {
  public:
    void insert (interval i)
    {
      std::CONTAINER<interval>::iterator inter
        (std::find_if ( _.begin()
                      , _.end()
                      , boost::bind (&interval::intersects, i, _1)
                      )
        );

      intersect_from (inter, i);

      silist::insert (i); // a second binary search
    }
  protected:
    void intersect_from (std::CONTAINER<interval>::iterator& inter, interval& i)
    {
      while (inter != _.end() && inter->intersects (i))
      {
        i.set_begin (std::min (i.begin(), inter->begin()));
        i.set_end (std::max (i.end(), inter->end()));
        inter = _.erase (inter);
      }
    }
  };

  class fsmilist : public smilist // fast sorted merged interval list
  {
  public:
    void insert (interval i)
    {
      std::CONTAINER<interval>::iterator inter
        (std::lower_bound ( _.begin()
                          , _.end()
                          , i
                          , boost::bind (&interval::end, _1)
                          < boost::bind (&interval::begin, _2)
                          )
        );

      intersect_from (inter, i);

      _.insert (inter, i); // no second search needed!
    }
  };

  class printer
  {
  public:
    printer (const std::string& descr, const std::CONTAINER<interval>& elements)
      : _descr (descr)
      , _elements (elements)
    {}
    std::ostream& operator() (std::ostream& os) const
    {
      bool first (true);
      os << _descr << " [" << _elements.size() << "]: {";
      BOOST_FOREACH (const interval& i, _elements)
      {
        if (first)
        {
          first = false;
        }
        else
        {
          os << " ";
        }
        os << i;
      }
      return os << "}";
    }
  private:
    const std::string& _descr;
    const std::CONTAINER<interval>& _elements;
  };

  std::ostream& operator<< (std::ostream& os, const printer& p)
  {
    return p (os);
  }
  std::ostream& operator<< (std::ostream& os, const silist& l)
  {
    return os << printer ("silist", l.elements());
  }
  std::ostream& operator<< (std::ostream& os, const smilist& l)
  {
    return os << printer ("smilist", l.elements());
  }
  std::ostream& operator<< (std::ostream& os, const fsmilist& l)
  {
    return os << printer ("fsmilist", l.elements());
  }

  template <class ForwardIterator>
  bool is_sorted (ForwardIterator first, ForwardIterator last)
  {
    if (first==last) return true;
    ForwardIterator next = first;
    while (++next!=last) {
      if (next->begin()<first->begin())
        return false;
      ++first;
    }
    return true;
  }

  double current_time()
  {
    struct timeval tv;

    gettimeofday (&tv, NULL);

    return (double(tv.tv_sec) + double (tv.tv_usec) * 1E-6);
  }

  template<typename L>
  void timed_insert (const std::string& descr, L& l, const int N)
  {
    srand (3141);
    int b (0);
    double t (-current_time());
    for (int k (0); k < N; ++k)
    {
      int e (b + rand() % 10);
      l.insert (interval (b, e));
      b += rand() % 5;
    }
    t += current_time();
    std::cout << descr
              << " [" << N << " -> " << l.elements().size() << "] "
              << t << " sec(s)"
              << std::endl;
  }
}

int main ()
{
  {
    std::CONTAINER<interval> l;

    l.push_back (interval (5,8));
    l.push_back (interval (0,2));
    l.push_back (interval (2,3));
    l.push_back (interval (3,4));
    l.push_back (interval (1,3));
    l.push_back (interval (6,7));
    l.push_back (interval (5,7));
    l.push_back (interval (6,9));
    l.push_back (interval (10,11));

    silist sil;
    smilist smil;
    fsmilist fsmil;

    BOOST_FOREACH (const interval& i, l)
    {
      sil.insert (i);
      smil.insert (i);
      fsmil.insert (i);
    }

    std::cout << sil << std::endl;
    std::cout << smil << std::endl;
    std::cout << fsmil << std::endl;

    if (!is_sorted (sil.elements().begin(), sil.elements().end()))
    {
      throw std::runtime_error ("silist not sorted");
    }
    if (!is_sorted (smil.elements().begin(), smil.elements().end()))
    {
      throw std::runtime_error ("smilist not sorted");
    }
    if (!is_sorted (fsmil.elements().begin(), fsmil.elements().end()))
    {
      throw std::runtime_error ("fsmilist not sorted");
    }
  }

  {
    smilist smil;
    fsmilist fsmil;
    timed_insert ("smilist", smil, INSERTS);
    timed_insert ("fsmilist", fsmil, INSERTS);
    std::cout << "equal: "
              << std::boolalpha << (smil.elements() == fsmil.elements())
              << std::endl;
  }
}

/*
rahn@brank:~$ clang++ -DCONTAINER=vector -D INSERTS=10000 t.cpp -I/usr && ./a.out
silist [9]: {[0..2) [1..3) [2..3) [3..4) [5..8) [5..7) [6..7) [6..9) [10..11)}
smilist [3]: {[0..4) [5..9) [10..11)}
fsmilist [3]: {[0..4) [5..9) [10..11)}
smilist [10000 -> 621] 0.12373 sec(s)
fsmilist [10000 -> 621] 0.00777507 sec(s)
equal: true
=> 15.9x
rahn@brank:~$ clang++ -DCONTAINER=vector -D INSERTS=100000 t.cpp -I/usr && ./a.out
silist [9]: {[0..2) [1..3) [2..3) [3..4) [5..8) [5..7) [6..7) [6..9) [10..11)}
smilist [3]: {[0..4) [5..9) [10..11)}
fsmilist [3]: {[0..4) [5..9) [10..11)}
smilist [100000 -> 6676] 12.0356 sec(s)
fsmilist [100000 -> 6676] 0.0999351 sec(s)
equal: true
=> 120x
rahn@brank:~$ clang++ -O3 -DCONTAINER=vector -D INSERTS=100000 t.cpp -I/usr && ./a.out
silist [9]: {[0..2) [1..3) [2..3) [3..4) [5..8) [5..7) [6..7) [6..9) [10..11)}
smilist [3]: {[0..4) [5..9) [10..11)}
fsmilist [3]: {[0..4) [5..9) [10..11)}
smilist [100000 -> 6676] 0.823446 sec(s)
fsmilist [100000 -> 6676] 0.00486517 sec(s)
equal: true
=> 170x
=> 5 millisec to merge 100000 intervals (into 6676)
rahn@brank:~$ clang++ -O3 -DCONTAINER=list -D INSERTS=100000 t.cpp -I/usr && ./a.out
silist [9]: {[0..2) [1..3) [2..3) [3..4) [5..8) [5..7) [6..7) [6..9) [10..11)}
smilist [3]: {[0..4) [5..9) [10..11)}
fsmilist [3]: {[0..4) [5..9) [10..11)}
smilist [100000 -> 6676] 2.38909 sec(s)
fsmilist [100000 -> 6676] 1.45788 sec(s)
equal: true
=> list is much slower
 */
