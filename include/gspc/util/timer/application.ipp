#include <utility>



    namespace gspc::util::timer
    {
      template<typename Duration, typename Clock>
        application<Duration, Clock>::application ( std::string description
                                                  , std::ostream& os
                                                  )
          : ostream::echo (os)
          , sections<Duration, Clock> (std::move (description), *this)
      {}
    }
