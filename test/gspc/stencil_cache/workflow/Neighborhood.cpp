#include <test/gspc/stencil_cache/workflow/Neighborhood.hpp>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/filtering_stream.hpp>

#include <algorithm>
#include <utility>

namespace test
{
  namespace gspc
  {
    namespace stencil_cache
    {
      namespace workflow
      {
        Neighborhood::Neighborhood (Coordinate x, Coordinate y, Coordinate r)
          : X (std::move (x))
          , Y (std::move (y))
          , R (std::move (r))
        {}

        std::list<Neighborhood::Coordinate> Neighborhood::operator()
          (Coordinate coordinate) const
        {
          std::list<Coordinate> neighbors;

          auto const px (coordinate / Y);
          auto const py (coordinate % Y);

          for (long x {std::max (0L, px - R)}; x < std::min (X, px + R + 1L); ++x)
          for (long y {std::max (0L, py - R)}; y < std::min (Y, py + R + 1L); ++y)
          {
            neighbors.emplace_back (y + Y * x);
          }

          return neighbors;
        }

        namespace
        {
          using IArchive = boost::archive::binary_iarchive;
          using OArchive = boost::archive::binary_oarchive;

          boost::archive::archive_flags flags()
          {
            return boost::archive::archive_flags::no_header;
          }
        }

        Neighborhood::Neighborhood (std::vector<char> const& data)
        {
          boost::iostreams::filtering_istream zin;
          zin.push (boost::iostreams::array_source (data.data(), data.size()));
          IArchive ia {zin, flags()};
          ia >> X;
          ia >> Y;
          ia >> R;
        }

        std::vector<char> Neighborhood::data() const
        {
          std::vector<char> data;
          {
            boost::iostreams::filtering_ostream zos;
            zos.push (boost::iostreams::back_inserter (data));
            OArchive oa {zos, flags()};
            oa << X;
            oa << Y;
            oa << R;
          }
          return data;
        }
      }
    }
  }
}

using N = test::gspc::stencil_cache::workflow::Neighborhood;

GSPC_STENCIL_CACHE_CALLBACK (std::shared_ptr<void>, init)
  (std::vector<char> const& data)
{
  return std::make_shared<N> (data);
}

GSPC_STENCIL_CACHE_CALLBACK
  (std::list<gspc::stencil_cache::Coordinate>, neighbors)
  ( void* neighborhood
  , gspc::stencil_cache::Stencil coordinate
  )
{
  return static_cast<N*> (neighborhood)->operator() (coordinate);
}
