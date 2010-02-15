// basic usage of multirel.hpp, mirko.rahn@itwm.fraunhofer.de

#include <we/container/multirel.hpp>

#include <iostream>
#include <sstream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>

#include <boost/serialization/nvp.hpp>

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
  typename multirel::right_const_it<L,R> it (rel.left_of (r));

  cout << r << " [#" << it.size() << "]" << " =>";

  for (; it.has_more(); ++it)
    cout << " " << *it;

  cout << endl;
}

template<typename L, typename R>
static void print_rel_right_of (const multirel::multirel<L,R> & rel, const L & l)
{
  typename multirel::left_const_it<L,R> it (rel.right_of (l));

  cout << l << " [#" << it.size() << "]" << " =>";

  for (; it.has_more(); ++it)
    cout << " " << *it;

  cout << endl;
}

int
main ()
{
  typedef multirel::multirel<char,int> multirel_t;

  multirel_t r;

  r.add ('a',3);
  r.add ('a',3);
  r.add ('a',3);
  r.add ('b',3);
  r.add ('b',4);
  r.add ('b',4);
  r.add ('a',5);
  r.add ('c',6);
  r.add ('c',7);

  print_rel (r);

  std::ostringstream oss_text;
  std::ostringstream oss_xml;
  std::ostringstream oss_binary;

  cout << "dumping..." << endl;

  {
    boost::archive::text_oarchive oa_text(oss_text, boost::archive::no_header);
    boost::archive::xml_oarchive oa_xml(oss_xml);
    boost::archive::binary_oarchive oa_binary(oss_binary, boost::archive::no_header);

    oa_text << BOOST_SERIALIZATION_NVP(r);
    oa_xml << BOOST_SERIALIZATION_NVP(r);
    oa_binary << BOOST_SERIALIZATION_NVP(r);
  }

  cout << "dump_text [" << oss_text.str().length() << "]:" << endl;
  cout << oss_text.str() << endl;
  cout << "dump_xml [" << oss_xml.str().length() << "]:" << endl;
  cout << oss_xml.str() << endl;
  cout << "dump_binary [" << oss_binary.str().length() << "]:" << endl;
  cout << oss_binary.str() << endl;

  print_rel_left_of<char,int> (r, 3);
  print_rel_right_of<char,int> (r, 'b');

  cout << r.contains_left ('a') << endl;
  cout << r.contains_left ('d') << endl;
  cout << r.contains_right (4) << endl;

  r.delete_one ('a',3);
  r.delete_all ('b',4);

  print_rel (r);

  cout << r.contains_right (4) << endl;

  r.delete_right (3);

  print_rel (r);

  cout << r.contains_left ('c') << endl;

  r.delete_left ('c');

  print_rel (r);

  {
    cout << "restore from dump_text..." << endl;

    std::istringstream iss(oss_text.str());
    boost::archive::text_iarchive ia(iss, boost::archive::no_header);

    ia >> BOOST_SERIALIZATION_NVP(r);

    print_rel (r);
  }

  {
    cout << "restore from dump_xml..." << endl;

    std::istringstream iss(oss_xml.str());
    boost::archive::xml_iarchive ia(iss);

    ia >> BOOST_SERIALIZATION_NVP(r);

    print_rel (r);
  }

  {
    cout << "restore from dump_binary..." << endl;

    std::istringstream iss(oss_binary.str());
    boost::archive::binary_iarchive ia(iss, boost::archive::no_header);

    ia >> BOOST_SERIALIZATION_NVP(r);

    print_rel (r);
  }

  return 0;
}
