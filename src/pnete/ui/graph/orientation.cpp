// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/graph/orientation.hpp>

#include <stdexcept>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        namespace port
        {
          namespace orientation
          {
            namespace detail
            {
              namespace exception
              {
                class generic : public std::runtime_error
                {
                public:
                  explicit generic (const std::string& msg)
                    : std::runtime_error ("parse error: orientation: " + msg)
                  {}
                };

                class expect : public generic
                {
                public:
                  explicit expect ( const std::string::const_iterator& begin
                                  , const std::string::const_iterator& end
                                  )
                    : generic ("expected: " + std::string (begin, end))
                  {}
                  explicit expect (const std::string& msg)
                    : generic ("expected: " + msg)
                  {}
                };
              }

              static void require ( const std::string& what
                                  , std::string::const_iterator& pos
                                  , const std::string::const_iterator& end
                                  )
              {
                std::string::const_iterator what_pos (what.begin());
                const std::string::const_iterator& what_end (what.end());

                while (what_pos != what_end)
                  {
                    if (pos == end || *what_pos != *pos)
                      {
                        throw exception::expect (what_pos, what_end);
                      }
                    else
                      {
                        ++pos; ++what_pos;
                      }
                  }

                if (pos != end)
                  {
                    throw exception::generic ("input to long");
                  }
              }
            }

            std::string show (const type& o)
            {
              switch (o)
                {
                case NORTH: return "NORTH";
                case EAST: return "EAST";
                case SOUTH: return "SOUTH";
                case WEST: return "WEST";
                default:
                  throw std::runtime_error ("STRANGE: Unknown orientation");
                }
            }
            type read (const std::string& inp)
            {
              std::string::const_iterator pos (inp.begin());
              const std::string::const_iterator& end (inp.end());

              if (pos != end)
                {
                  switch (*pos)
                    {
                    case 'N': detail::require ("ORTH", ++pos, end); return NORTH;
                    case 'E': detail::require ("AST", ++pos, end); return EAST;
                    case 'S': detail::require ("OUTH", ++pos, end); return SOUTH;
                    case 'W': detail::require ("EST", ++pos, end); return WEST;
                    }
                }

              throw detail::exception::expect ("NORTH, EAST, SOUTH or WEST");
            }

            type invert (const type& o)
            {
              switch (o)
              {
              case NORTH: return SOUTH;
              case EAST: return WEST;
              case SOUTH: return NORTH;
              case WEST: return EAST;
              default: throw std::runtime_error ("STRANGE: Unknown orientation");
              }
            }
          }
        }
      }
    }
  }
}
