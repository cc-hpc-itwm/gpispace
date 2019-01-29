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

        void Neighborhood::operator()
          (Coordinate coordinate, std::list<Coordinate>& neighbors) const
        {
          auto const px (coordinate / Y);
          auto const py (coordinate % Y);

          neighbors.clear();

          for (long x {std::max (0L, px - R + 1L)}; x < std::min (X, px + R); ++x)
          for (long y {std::max (0L, py - R + 1L)}; y < std::min (Y, py + R); ++y)
          {
            neighbors.emplace_back (y + Y * x);
          }
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
