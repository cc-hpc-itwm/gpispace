// mirko.rahn@itwm.fraunhofer.de

#include <we/type/property.hpp>

#include <fhg/util/join.hpp>

#include <boost/optional.hpp>

#include <iostream>

namespace prop = we::type::property;

static prop::type p;

void set (const std::string & path, const std::string & val)
{
  std::cout << "# set " << path << std::endl;

  const boost::optional<prop::mapped_type> old (p.set (path, val));

  std::cout << p
            << "overwritten " << (old ? "true" : "false")
            << std::endl
    ;
}

void get (const std::string & path)
{
  std::cout << "# get " << path << " => " << std::endl;

  try
    {
      std::cout << p.get (path) << std::endl;
    }
  catch (const prop::exception::missing_binding & m)
    {
      std::cout << "MISSING: " << m.what() << std::endl;
    }
}

void get_val (const std::string & path)
{
  std::cout << "# get_val " << path << " => " << std::endl;

  try
    {
      std::cout << p.get_val (path) << std::endl;
    }
  catch (const prop::exception::missing_binding & m)
    {
      std::cout << "MISSING: " << m.what() << std::endl;
    }
  catch (const prop::exception::not_a_val & m)
    {
      std::cout << "NOT A VALUE: " << m.what() << std::endl;
    }
}

void get_maybe_val (const std::string & path)
{
  std::cout << "# get_maybe_val " << path
            << " => " << p.get_maybe_val (path)
            << std::endl
    ;
}

void get_with_default (const std::string & path, const std::string & dflt)
{
  std::cout << "# get_with_default " << path
            << " => " << p.get_with_default (path, dflt)
            << std::endl
    ;
}

void all_get (const std::string & path)
{
  std::cout << "###" << std::endl;
  get (path);
  get_val (path);
  get_maybe_val (path);
  get_with_default (path, "<default>");
}

void del (const std::string & path)
{
  std::cout << "# del " << path << std::endl;

  p.del (path);

  std::cout << p << std::endl;
}

int
main ()
{
  set ("A.A.A", "value_of (A.A.A)");
  all_get ("A.A.A");
  all_get ("A.A.B");
  all_get ("A.A.B");
  all_get ("A.A");
  all_get ("A");
  all_get ("A.A");
  set ("A.A", "value_of (A.A)");
  set ("A", "value_of (A)");
  set ("A.A.A", "value_of (A.A.A)");
  set ("A.A.B", "value_of (A.A.B)");
  set ("A.A.C", "value_of (A.A.C)");
  set ("A.B.A", "value_of (A.B.A)");
  set ("A.B.B", "value_of (A.B.B)");
  set ("A.C.A", "value_of (A.C.A)");
  set ("A.C.A", "value_of (A.C.A)");
  set ("B.A.A", "value_of (B.A.A)");
  del ("A.B");
  del ("A.A.A");
  all_get ("B.A.A");
  all_get ("B.A");
  set ("A.A.B.A", "value_of (A.A.B.A)");

  std::cout << "# visit all leafs:" << std::endl;

  prop::traverse::stack_type stack (prop::traverse::dfs (p));

  while (!stack.empty())
    {
      const prop::traverse::pair_type elem (stack.top());
      const prop::path_type path (elem.first);
      const prop::value_type value (elem.second);

      std::cout << fhg::util::join (path, ".") << " => " << value
                << std::endl
        ;

      stack.pop();
    }

  return EXIT_SUCCESS;
}
