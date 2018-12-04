#include <drts/cache_management/error.hpp>

#include <boost/format.hpp>

#include <string>

namespace drts
{
  namespace cache
  {
    namespace error
    {
      buffer_larger_than_cache_size
        ::buffer_larger_than_cache_size
        ( unsigned long buffer_size
        , unsigned long cache_size
        )
          : generic ( boost::format
                      ( "Buffer size is larger than the cache size: ! (%1% > %2%)")
                      % buffer_size
                      % cache_size
                    )
        {}

      insufficient_space_available_in_cache
          ::insufficient_space_available_in_cache
          ( unsigned long cache_size
          , unsigned long free
          , unsigned long gap_begin
          , unsigned long gap_end
          )
            : generic ( boost::format
                        ( "The cached memory buffers do not fit in cache (cache_size=%1%, offset_free=%2%, gap_begin=%3%, gap_end=%4%)")
                        % cache_size
                        % free
                        % gap_begin
                        % gap_end
                      )
          {}

      buffer_not_in_cache
      ::buffer_not_in_cache(std::string dataid)
          : generic ( boost::format( "The required buffer with id=%1% is not cached")
                    % dataid
                    )
      {}
    }
  }
}
