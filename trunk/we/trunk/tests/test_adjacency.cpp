// basic usage of adjacency.hpp, mirko.rahn@itwm.fraunhofer.de

#include <we/container/adjacency.hpp>
#include <we/container/adjacency.ipp>

#include <limits>

#include <iostream>

typedef unsigned short row_t;
typedef unsigned int col_t;
typedef char adj_t;
// static const adj_t invalid (std::numeric_limits<adj_t>::max());
static const adj_t invalid ('.');

using std::cout;
using std::endl;

int
main ()
{
  adjacency::table<row_t,col_t,adj_t> t (invalid, 1, 1);

  t.set_adjacent (0, 23, 'a');
  t.set_adjacent (0, 24, 'a');
  t.set_adjacent (0, 25, 'b');
  t.set_adjacent (0, 26, 'a');

  t.set_adjacent (1, 24, 'b');
  t.set_adjacent (1, 25, 'a');
  t.set_adjacent (1, 26, 'b');
  t.set_adjacent (1, 27, 'b');

  cout << t.get_adjacent (0,22) << endl;
  cout << t.get_adjacent (0,23) << endl;
  cout << t.get_adjacent (0,24) << endl;
  cout << t.get_adjacent (0,25) << endl;
  cout << t.get_adjacent (0,26) << endl;
  cout << t.get_adjacent (0,27) << endl;
  cout << t.get_adjacent (0,28) << endl;

  cout << t.get_adjacent (1,22) << endl;
  cout << t.get_adjacent (1,23) << endl;
  cout << t.get_adjacent (1,24) << endl;
  cout << t.get_adjacent (1,25) << endl;
  cout << t.get_adjacent (1,26) << endl;
  cout << t.get_adjacent (1,27) << endl;
  cout << t.get_adjacent (1,28) << endl;

  cout << t.get_adjacent (333,444) << endl;

  cout << "const row " << 0 << ":" << endl;
  for ( adjacency::const_it<col_t,adj_t> it (t.row_const_it (0))
      ; it.has_more()
      ; ++it
      )
    cout << " " << *it << " by " << it() << endl;

  cout << "const col " << 25 << ":" << endl;
  for ( adjacency::const_it<row_t,adj_t> it (t.col_const_it (25))
      ; it.has_more()
      ; ++it
      )
    cout << " " << *it << " by " << it() << endl;

  cout << "const col " << 3141 << ":" << endl;
  for ( adjacency::const_it<row_t,adj_t> it (t.col_const_it (3141))
      ; it.has_more()
      ; ++it
      )
    cout << " " << *it << " by " << it() << endl;

  return 0;
}
