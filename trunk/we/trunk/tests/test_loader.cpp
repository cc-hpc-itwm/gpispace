#include <iostream>

#include <we/loader/loader.hpp>

#include "answer.hpp"

int main (int, char **)
{
  we::loader::loader loader;

  loader.load ( "answer", "./libanswer.so" );
  loader.load ( "question", "./libquestion.so" );

  std::cerr << loader << std::endl;

  we::loader::input_t inp;
  we::loader::output_t out;

  loader["answer"] ("answer", inp, out);

  return EXIT_SUCCESS;
}

