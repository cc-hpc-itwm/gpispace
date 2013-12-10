#include <we/mgmt/bits/signal.hpp>
#include <iostream>

static int counter = 0;
static std::string text;

static void count (void)
{
  std::cerr << "counting" << std::endl;

  counter ++;
}

static void m (int id, const std::string & s)
{
  std::cerr << "got signal: " << id << ": " << s << std::endl;
  text += s;
}

int main ()
{
  we::mgmt::util::signal<> sig_cnt;
  sig_cnt.connect ( &count );

  sig_cnt();
  sig_cnt();
  if (2 != counter)
  {
    return EXIT_FAILURE;
  }

  we::mgmt::util::signal<void (int, const std::string &)> sig_sig;
  text = "";

  sig_sig.connect ( &m );
  sig_sig ( 1, "Hallo " );
  sig_sig ( 2, "Welt!" );

  if (text != "Hallo Welt!")
  {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
