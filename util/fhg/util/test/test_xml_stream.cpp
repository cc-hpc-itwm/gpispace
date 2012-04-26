
#include <fhg/util/xml.hpp>

int main ()
{
  fhg::util::xml::xmlstream s (std::cout);

  s.open ("first");

  s.open ("second");

  s.attr ("key", "val");
  s.attr ("maybe_key", boost::optional<std::string>("Just val"));
  s.attr ("maybe_key", boost::none);

  s.close ();

  s.open ("content");
  s.content (23);
  s.close();

  s.open ("more");
  s.open ("deeper");
  s.attr ("key", 23);
  s.close();
  s.close ();

  s.close();

  return EXIT_SUCCESS;
}
