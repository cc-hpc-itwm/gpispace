
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>

#include <net.hpp>

using std::cout;
using std::endl;

static unsigned long cons (0);
static unsigned long decons (0);
static unsigned long copy (0);

class place_t
{
private:
  int x;
public:
  void out (std::string msg) { cout << msg << ": " << x << endl; }

  place_t () : x(0) { out ("place_t()"); ++cons; }
  place_t (const place_t & p) { x = p.x + 1; out ("copy place_t()"); ++copy; }
  ~place_t () { out ("~place_t()"); ++decons; }

  const int get (void) const { return x; }

  bool operator == (const place_t & other) const 
  {
    cout << "compare this.x == " << x
         << " with other.x == " << other.x
         << endl;

    return (x == other.x);
  }
};

static inline const std::size_t hash_value (const place_t & p)
{
  return p.get();
}

std::ostream & operator << (std::ostream & s, const place_t & p)
{
  return s << "place_t: copy count = " << p.get();
}

typedef std::string transition_t;
typedef std::string edge_t;
typedef std::string token_t;

typedef net<place_t, transition_t, edge_t, token_t> pnet_t;

static void print (const pnet_t & n)
{
  cout << "*** places [" << endl;

  for (pnet_t::place_const_it pit (n.places()); pit.has_more(); ++pit)
    cout << *pit << " => " << n.place(*pit) << endl;

  cout << "]" << endl;
}

int
main ()
{
  {
    cout << endl << "**** CONSTRUCTING AND DELETING ****" << endl;

    pnet_t n;

    place_t p;

    cout << "add_place => " << n.add_place (p) << endl;
    cout << "add_place => " << n.add_place (p) << endl;

    print (n);

    try
      {
        cout << "del_place => " << n.delete_place (p) << endl;
      }
    catch (no_such)
      {
        cout << "NOT THERE!" << endl;
      }

    print (n);

    for (pnet_t::place_const_it pit (n.places()); pit.has_more(); ++pit)
      n.delete_place (*pit);

    print (n);
  }

  cout << "#" << endl;
  cout << "# cons = " << cons << endl;
  cout << "# decons = " << decons << endl;
  cout << "# copy = " << copy << endl;

  {
    cout << endl << "**** MODIFYING ****" << endl;

    pnet_t n;

    place_t p;

    cout << "add_place => " << n.add_place (p) << endl;
    cout << "add_place => " << n.add_place (p) << endl;

    print (n);

    cout << "** MODIFY 0" << endl;

    try
      {
        n.modify_place (0, p);
      }
    catch (already_there)
      {
        cout << "MODIFY: NO SUCCESS" << endl;
      }

    print (n);

    cout << "** MODIFY 1" << endl;

    try
      {
        n.modify_place (1, p);
      }
    catch (already_there)
      {
        cout << "MODIFY: NO SUCCESS" << endl;
      }

    print (n);
  }

  {
    cout << endl << "**** REPLACING ****" << endl;

    pnet_t n;

    place_t p;

    cout << "add_place => " << n.add_place (p) << endl;
    cout << "add_place => " << n.add_place (p) << endl;

    print (n);

    cout << "** REPLACE 0" << endl;

    try
      {
        n.replace_place (0, p);
      }
    catch (already_there)
      {
        cout << "REPLACE: NO SUCCESS" << endl;
      }

    print (n);

    cout << "** REPLACE 1" << endl;

    try
      {
        n.replace_place (1, p);
      }
    catch (already_there)
      {
        cout << "REPLACE: NO SUCCESS" << endl;
      }

    print (n);
  }

  return EXIT_SUCCESS;
}
