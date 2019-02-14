#include <gspc/stencil_cache/types.hpp>

#include <list>
#include <vector>

namespace test
{
  namespace gspc
  {
    namespace stencil_cache
    {
      namespace workflow
      {
        struct Neighborhood
        {
          using Coordinate = ::gspc::stencil_cache::Coordinate;

          Neighborhood (Coordinate, Coordinate, Coordinate);

          std::list<Coordinate> operator() (Coordinate) const;

          Neighborhood (std::vector<char> const&);
          std::vector<char> data() const;

        private:
          Coordinate X;
          Coordinate Y;
          Coordinate R;
        };
      }
    }
  }
}
