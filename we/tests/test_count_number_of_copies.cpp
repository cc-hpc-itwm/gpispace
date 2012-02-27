// count how often objects are copied, mirko.rahn@itwm.fraunhofer.de

#include <we/net.hpp>

#include <cstdlib>

#include <iostream>
#include <sstream>

#include <map>
#include <string>

#include <boost/unordered_map.hpp>

using std::cout;
using std::endl;

typedef boost::unordered_map<std::string, unsigned long> cntmap_t;

static cntmap_t cntmap;

static void print_cnt ()
{
  cout << "*** CNTMAP:" << endl;

  for (cntmap_t::const_iterator it (cntmap.begin()); it != cntmap.end(); ++it)
    cout << it->first << " => " << it->second << endl;
}

class copy_counted
{
private:
  std::string descr_;
  unsigned long copy_count_;
public:
  void out (std::string msg)
  {
    cout << msg << ": " << copy_count_ << endl;

    ++cntmap[msg];
  }

  copy_counted (std::string descr)
    : descr_(descr)
    , copy_count_(0)
  {
    out ("cons '" + descr_ + "'");

    ++cntmap["CONS"];
  }

  ~copy_counted ()
  {
    out ("decons '" + descr_ + "'");

    ++cntmap["DECONS"];
  }

  copy_counted (const copy_counted & cc)
  {
    copy_count_ = cc.copy_count_ + 1;
    descr_ = "copy of " + cc.descr_;
    out ("copy '" + cc.descr_ + "'");

    ++cntmap["COPY"];
  }

  unsigned long copy_count (void) const
  {
    return copy_count_;
  }
};

class internal_place_t : public copy_counted
{
public:
  internal_place_t () : copy_counted("internal_place_t") {}

  bool operator == (const internal_place_t & other) const
  {
    cout << "internal_place: this.copy_count " << copy_count()
         << " ==? other.x " << other.copy_count()
         << endl;

    return (copy_count() == other.copy_count());
  }

  bool operator < (const internal_place_t & other) const
  {
    cout << "internal_place: this.copy_count " << copy_count()
         << " <? other.x " << other.copy_count()
         << endl;

    return (copy_count() < other.copy_count());
  }
};

static inline std::size_t hash_value (const internal_place_t & p)
{
  return p.copy_count();
}

static std::ostream & operator << (std::ostream & s, const internal_place_t & p)
{
  return s << "internal_place_t: copy count = " << p.copy_count();
}

class ptr_place_t : public copy_counted
{
private:
  internal_place_t * p_;
public:
  ptr_place_t () : copy_counted ("ptr_place_t"), p_(new internal_place_t()) {}
  ~ptr_place_t () { if (copy_count()==0) delete p_; }

  const internal_place_t * p (void) const { return p_; }

  bool operator == (const ptr_place_t & other) const
  {
    cout << "ptr_place_t: *this.p_ " << *p()
         << " ==? *other.p_ " << *(other.p())
         << endl;

    return (*(p()) == *(other.p()));
  }

  bool operator < (const ptr_place_t & other) const
  {
    cout << "ptr_place_t: *this.p_ " << *p()
         << " <? *other.p_ " << *(other.p())
         << endl;

    return (*(p()) < *(other.p()));
  }
};

static inline std::size_t hash_value (const ptr_place_t & p)
{
  return hash_value(*(p.p()));
}

static std::ostream & operator << (std::ostream & s, const ptr_place_t & p)
{
  return s << *(p.p());
}

// use internal directly
// typedef internal_place_t place_t;

// indirect
typedef ptr_place_t place_t;

struct transition_t
{
public:
  std::string name;

  template<typename T>
  bool condition (const T & ) const { return true; }
};

inline std::size_t hash_value (const transition_t & t)
{
  boost::hash<std::string> h;

  return h (t.name);
}

inline bool operator == (const transition_t & x, const transition_t & y)
{
  return x.name == y.name;
}

typedef std::string edge_t;
typedef std::string token_t;

typedef petri_net::net<place_t, transition_t, edge_t, token_t> pnet_t;

static void print (const pnet_t & n)
{
  cout << "*** places [" << endl;

  for (pnet_t::place_const_it pit (n.places()); pit.has_more(); ++pit)
    cout << *pit << " => " << n.get_place(*pit) << endl;

  cout << "]" << endl;
}

static void add (pnet_t & n, const place_t & p)
{
  cout << "add " << p << " => ";

  try
    {
      cout << n.add_place (p);
    }
  catch (bijection::exception::already_there)
    {
      cout << "ALREADY_THERE";
    }

  cout << endl;
}

static void del (pnet_t & n, const place_t & p)
{
  cout << "del " << p << " => ";

  try
    {
      cout << n.delete_place (n.get_place_id (p));
    }
  catch (bijection::exception::no_such)
    {
      cout << "NO SUCH";
    }

  cout << endl;
}

static void del_pid (pnet_t & n, const petri_net::pid_t & pid)
{
  cout << "del_pid " << pid << " => ";

  try
    {
      cout << n.delete_place (pid);
    }
  catch (bijection::exception::no_such)
    {
      cout << "NO SUCH";
    }

  cout << endl;
}

static void mod (pnet_t & n, const petri_net::pid_t & pid, const place_t & p)
{
  cout << "modify " << pid << " -> " << p << " => ";

  try
    {
      cout << n.modify_place (pid, p);
    }
  catch (bijection::exception::already_there)
    {
      cout << "ALREADY THERE";
    }
  catch (bijection::exception::no_such)
    {
      cout << "NO SUCH";
    }

  cout << endl;
}

static void rep (pnet_t & n, const petri_net::pid_t & pid, const place_t & p)
{
  cout << "replace " << pid << " -> " << p << " => ";

  try
    {
      cout << n.replace_place (pid, p);
    }
  catch (bijection::exception::already_there)
    {
      cout << "ALREADY THERE";
    }
  catch (bijection::exception::no_such)
    {
      cout << "NO SUCH";
    }

  cout << endl;
}

static void cons_and_delete (void)
{
  cout << endl << "**** CONSTRUCT AND DELETE ****" << endl;

  pnet_t n;

  place_t p;

  add (n, p);
  add (n, p);

  print (n);

  del (n, p);

  print (n);

  for (pnet_t::place_const_it pit (n.places()); pit.has_more(); ++pit)
    del_pid (n, *pit);

  print (n);
}

static void modify (void)
{
  cout << endl << "**** MODIFY ****" << endl;

  cntmap.clear();

  pnet_t n;

  place_t p;

  add (n, p);
  add (n, p);

  print (n);

  mod (n, 0, p);

  print (n);

  mod (n, 1, p);

  print (n);
}

static void replace (void)
{
  cout << endl << "**** REPLACE ****" << endl;

  pnet_t n;

  place_t p;

  add (n, p);
  add (n, p);

  print (n);

  rep (n, 0, p);

  print (n);

  rep (n, 1, p);

  print (n);
}

int
main ()
{
  cntmap.clear(); cons_and_delete(); print_cnt();

  cntmap.clear(); modify(); print_cnt();

  cntmap.clear(); replace(); print_cnt();

  {
    cout << "**** std::map[] ****" << endl;

    place_t p;

    typedef std::map<place_t,int> map_t;

    map_t m;

    cout << "std::map[]" << endl; m[p] = 0;
    cout << "std::map[]" << endl; m[p] = 1;

    for (map_t::const_iterator it (m.begin()); it != m.end(); ++it)
      cout << "## " << it->first << " -> " << it->second << endl;
  }

  {
    cout << "**** std::map.insert ****" << endl;

    place_t p;

    typedef std::map<place_t,int> map_t;

    map_t m;

    cout << "std::map.insert" << endl; m.insert (std::pair<place_t,int>(p, 0));
    cout << "std::map.insert" << endl; m.insert (std::pair<place_t,int>(p, 1));

    for (map_t::const_iterator it (m.begin()); it != m.end(); ++it)
      cout << "## " << it->first << " -> " << it->second << endl;
  }

  {
    cout << "**** boost::unordered_map[] ****" << endl;

    place_t p;

    typedef boost::unordered_map<place_t,int> map_t;

    map_t m;

    cout << "boost::unordered_map[]" << endl; m[p] = 0;
    cout << "boost::unordered_map[]" << endl; m[p] = 1;

    for (map_t::const_iterator it (m.begin()); it != m.end(); ++it)
      cout << "## " << it->first << " -> " << it->second << endl;
  }

  {
    cout << "**** boost::unordered_map.insert ****" << endl;

    place_t p;

    typedef boost::unordered_map<place_t,int> map_t;

    map_t m;

    cout << "boost::unordered_map.insert" << endl; m.insert (std::pair<place_t,int>(p, 0));
    cout << "boost::unordered_map.insert" << endl; m.insert (std::pair<place_t,int>(p, 1));

    for (map_t::const_iterator it (m.begin()); it != m.end(); ++it)
      cout << "## " << it->first << " -> " << it->second << endl;
  }

  return EXIT_SUCCESS;
}
