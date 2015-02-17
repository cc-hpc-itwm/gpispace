#pragma once

#include <boost/utility.hpp>

#include <vector>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      class buffer_t : boost::noncopyable
      {
      public:
        explicit buffer_t (std::vector<char>::size_type sz);

        inline char *data () { return &m_data[0]; }
        inline std::vector<char>::size_type size () const { return m_data.size(); }
        inline std::vector<char>::size_type used () const { return m_used; }
        inline void used (std::vector<char>::size_type u) { m_used = u; }

      private:
        std::vector<char> m_data;
        std::vector<char>::size_type m_used;
      };
    }
  }
}
