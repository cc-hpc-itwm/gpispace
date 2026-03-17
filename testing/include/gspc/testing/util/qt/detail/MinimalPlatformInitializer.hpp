#pragma once





        namespace gspc::util::qt::testing::detail
        {
          //! Initialize the platform plugin global state to minimal.
          struct MinimalPlatformInitializer
          {
            MinimalPlatformInitializer();
            ~MinimalPlatformInitializer();

            MinimalPlatformInitializer (MinimalPlatformInitializer const&) = delete;
            MinimalPlatformInitializer (MinimalPlatformInitializer&&) = delete;
            MinimalPlatformInitializer& operator= (MinimalPlatformInitializer const&) = delete;
            MinimalPlatformInitializer& operator= (MinimalPlatformInitializer&&) = delete;

          private:
            char const* _previous;
          };
        }
