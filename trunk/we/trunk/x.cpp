// basic usage of multirel.hpp, mirko.rahn@itwm.fraunhofer.de

#include <multirel.hpp>

#include <iostream>

using std::cout;
using std::endl;

template<typename L, typename R>
static void print_rel (const multirel::multirel<L,R> & rel)
{
  for ( typename multirel::traits<L,R>::const_it it (rel.begin())
      ; it != rel.end()
      ; ++it
      )
    cout << it->left << " <-> " << it->right << endl;
}

template<typename L, typename R>
static void print_rel_left_of (const multirel::multirel<L,R> & rel, const R & r)
{
  cout << r << " =>";

  for ( typename multirel::right_const_it<L,R> it (rel.left_of (r))
      ; it.has_more()
      ; ++it
      )
    cout << " " << *it;

  cout << endl;
}

template<typename L, typename R>
static void print_rel_right_of (const multirel::multirel<L,R> & rel, const L & l)
{
  cout << l << " =>";

  for ( typename multirel::left_const_it<L,R> it (rel.right_of (l))
      ; it.has_more()
      ; ++it
      )
    cout << " " << *it;

  cout << endl;
}

int
main ()
{
  multirel::multirel<char,int> r;

  r.add ('a',3);
  r.add ('a',3);
  r.add ('a',3);
  r.add ('b',3);
  r.add ('b',4);
  r.add ('b',4);
  r.add ('a',5);
  r.add ('c',6);

  print_rel (r);
  print_rel_left_of<char,int> (r, 3);
  print_rel_right_of<char,int> (r, 'b');

  cout << r.contains_left ('a') << endl;
  cout << r.contains_left ('d') << endl;
  cout << r.contains_right (4) << endl;

  r.delete_one ('a',3);
  r.delete_all ('b',4);

  print_rel (r);

  cout << r.contains_right (4) << endl;

  return 0;
} 
