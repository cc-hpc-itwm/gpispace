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

  expr::eval::context inp;
  expr::eval::context out;

  loader["answer"] ("answer", inp, out);

  loader["question"] ("question", inp, out);
  loader["answer"] ("answer", inp, out);

  return EXIT_SUCCESS;
}
