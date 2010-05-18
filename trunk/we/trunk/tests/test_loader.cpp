#include <iostream>

#include <we/loader/loader.hpp>

#include "answer.hpp"

int main (int ac, char **argv)
{
  we::loader::loader loader;

  for (int i = 1; i < ac; ++i)
  {
    loader.append_search_path (argv[i]);
  }

  loader.load ( "answer" );
  loader.load ( "question" );

  std::cerr << loader << std::endl;

  we::loader::input_t inp;
  we::loader::output_t out;

  loader["answer"] ("answer", inp, out);

  loader.unload ("answer");
  std::cerr << loader << std::endl;

  loader["question"] ("question", inp, out);
  loader["answer"] ("answer", inp, out);
  std::cerr << loader << std::endl;

  return EXIT_SUCCESS;
}

