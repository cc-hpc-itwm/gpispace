#pragma once

#include <mmgr/word.h>

#include <stdexcept>
#include <string>
#include <vector>

namespace gspc
{
  namespace mmgr
  {
    namespace exception
    {
      struct heap_empty : public std::logic_error
      {
        heap_empty (std::string const&);
      };
    }

    struct heap
    {
      void insert (Offset_t offset);
      std::vector<Offset_t>::size_type size() const;
      Offset_t min() const;
      void delete_min();

    private:
      std::vector<Offset_t> _;
    };
  }
}
