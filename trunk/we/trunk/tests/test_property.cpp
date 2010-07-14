// mirko.rahn@itwm.fraunhofer.de

#include <we/type/property.hpp>

#include <iostream>

namespace prop = we::type::property;

static prop::type p;

void set (const std::string & path, const std::string & val)
{
  std::cout << "# set " << path << std::endl;

  p.set (path, val);

  std::cout << p << std::endl;
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
  get ("A.A.A");
  get ("A.A.B");
  get_val ("A.A.B");
  get_val ("A.A");
  get ("A");
  get ("A.A");
  set ("A.A", "value_of (A.A)");
  set ("A", "value_of (A)");
  set ("A.A.A", "value_of (A.A.A)");
  set ("A.A.B", "value_of (A.A.B)");
  set ("A.A.C", "value_of (A.A.C)");
  set ("A.B.A", "value_of (A.B.A)");
  set ("A.B.B", "value_of (A.B.B)");
  set ("A.C.A", "value_of (A.C.A)");
  set ("B.A.A", "value_of (B.A.A)");
  del ("A.B");
  del ("A.A.A");
  get_val ("B.A.A");
  get_val ("B.A");

  return EXIT_SUCCESS;
}
