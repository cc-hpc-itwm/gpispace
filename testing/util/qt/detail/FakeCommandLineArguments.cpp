#include <gspc/testing/util/qt/detail/FakeCommandLineArguments.hpp>

#include <gspc/util/warning.hpp>





        namespace gspc::util::qt::testing::detail
        {
          FakeCommandLineArguments::FakeCommandLineArguments()
          {
            argv_data.emplace_back ('t');
            argv_data.emplace_back ('e');
            argv_data.emplace_back ('s');
            argv_data.emplace_back ('t');
            argv_data.emplace_back ('\0');

            argv.emplace_back (argv_data.data());

            argc = gspc::util::suppress_warning::sign_conversion<int>
              ( gspc::util::suppress_warning::shorten_64_to_32_with_check<unsigned int>
                  ( argv.size(), "As per insertion above, size == 1ul.")
              , "As per insertion and cast above, size == 1ul."
              );
          }
        }
