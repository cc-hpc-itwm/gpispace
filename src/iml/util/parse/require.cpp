#include <iml/util/parse/require.hpp>

#include <iml/util/parse/error.hpp>

namespace fhg
{
  namespace iml
  {
    namespace util
    {
      namespace parse
      {
        namespace require
        {
          void require (position& pos, const char& what)
          {
            if (pos.end() || *pos != what)
            {
              throw error::expected (std::string (1, what), pos);
            }

            ++pos;
          }
          void require (position& pos, const std::string& what)
          {
            std::string::const_iterator what_pos (what.begin());
            const std::string::const_iterator what_end (what.end());

            while (what_pos != what_end)
            {
              if (pos.end() || *pos != *what_pos)
              {
                throw error::expected (std::string (what_pos, what_end), pos);
              }
              else
              {
                ++pos;
                ++what_pos;
              }
            }
          }

          std::string identifier (position& pos)
          {
            std::string id;

            if (pos.end() || !(isalpha (*pos) || *pos == '_'))
            {
              throw error::expected ("identifier [a-zA-Z_][a-zA-Z_0-9]*", pos);
            }

            id.push_back (*pos); ++pos;

            while (!pos.end() && (isalpha (*pos) || *pos == '_' || isdigit (*pos)))
            {
              id.push_back (*pos); ++pos;
            }

            return id;
          }
        }
      }
    }
  }
}
