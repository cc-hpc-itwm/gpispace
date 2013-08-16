#include <iostream>

#include <we/loader/loader.hpp>

#include "answer.hpp"

int main (int ac, char **argv)
{
  we::loader::loader loader;

  loader.append_search_path (".");

  for (int i = 1; i < ac; ++i)
  {
    loader.append_search_path (argv[i]);
  }

  expr::eval::context out;

  loader["answer"].call ("answer", expr::eval::context(), out);

  loader["question"].call ("question", expr::eval::context(), out);
  loader["answer"].call ("answer", expr::eval::context(), out);

  return EXIT_SUCCESS;
}
