
#include <process.hpp>
#include <iostream>
#include <cstdlib>

#include <stdexcept>

#include <require.hpp>

int
main ()
{
  process::file_const_buffer file (NULL, 0, "%redefinition%");

  process::file_const_buffer_list files_input;

  files_input.push_back (file);
  files_input.push_back (file);

  try
    {
      process::execute ( std::string()
                       , process::const_buffer (NULL,0)
                       , process::buffer (NULL,0)
                       , files_input
                       , process::file_buffer_list ()
                       );
    }
  catch (const std::runtime_error & e)
    {
      std::cout << "catched exception:" << std::endl << e.what() << std::endl;

      REQUIRE (  std::string (e.what())
              == std::string ("redefinition of key: %redefinition%")
              );
    }

  std::cout << "SUCCESS" << std::endl;

  return EXIT_SUCCESS;
}
