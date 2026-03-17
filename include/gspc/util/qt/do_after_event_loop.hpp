#pragma once

#include <functional>



    namespace gspc::util::qt
    {
      //! Execute the given function after one Qt event loop cycle.
      void do_after_event_loop (std::function<void()>);
    }
