
// measurements bimap vs. homegrown bijection, mirko.rahn@itwm.fraunhofer.de

#include <cstdlib>
#include <iostream>
#include <string>

#include <bijection.hpp>

typedef std::string T;
typedef bijection::bijection<T> bijection_t;

using std::cout;
using std::endl;

static void ins (bijection_t & bi, const T & x)
{
  cout << "insert " << x << " => ";

  try
    {
      cout << bi.insert (x);
    }
  catch (bijection::already_there)
    {
      cout << "ALREADY THERE";
    }

  cout << endl;
}

static void get_id (bijection_t & bi, const T & x)
{
  cout << "get_id " << x << " => ";

  try
    {
      cout << bi.get_id (x);
    }
  catch (bijection::no_such)
    {
      cout << "NO SUCH";
    }
  
  cout << endl;
}

static void get_elem (bijection_t & bi, const bijection::id_t & id)
{
  cout << "get_elem " << id << " => ";

  try
    {
      cout << bi.get_elem (id);
    }
  catch (bijection::no_such)
    {
      cout << "NO SUCH";
    }

  cout << endl;
}

static void erase (bijection_t & bi, const T & x)
{
  cout << "erase " << x << " => ";
  
  try
    {
      bi.erase (x); cout << "done";
    }
  catch (bijection::no_such)
    {
      cout << "NO SUCH";
    }

  cout << endl;
}

int
main ()
{
  bijection_t bi;

  ins (bi, "A");
  ins (bi, "B");
  ins (bi, "C");

  get_id (bi, "A");
  get_id (bi, "B");
  get_id (bi, "C");
  get_id (bi, "D");

  get_elem (bi, 0);
  get_elem (bi, 1);
  get_elem (bi, 2);
  get_elem (bi, 3);

  cout << bi;

  ins (bi, "A"); get_id (bi, "A");
  ins (bi, "A"); get_id (bi, "A");

  cout << bi;

  for (bijection_t::const_it it (bi.begin()); it != bi.end(); ++it)
    cout << it->first << " <=> " << it->second << std::endl;

  erase (bi, "B");
  erase (bi, "D");

  cout << bi;

  return EXIT_SUCCESS;
}
