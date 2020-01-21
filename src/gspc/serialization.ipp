#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/device/back_inserter.hpp>
#include <boost/iostreams/filtering_stream.hpp>

namespace gspc
{
  template<typename T>
    std::vector<char> bytes_save (T const& x)
  {
    std::vector<char> data;
    {
      boost::iostreams::filtering_ostream zos
        (boost::iostreams::back_inserter (data));
      boost::archive::binary_oarchive oa (zos);

      oa & x;
    }
    return data;
  }

  template<typename T>
    T bytes_load (std::vector<char> const& data)
  {
    boost::iostreams::filtering_istream zis
      (boost::iostreams::array_source (data.data(), data.size()));
    boost::archive::binary_iarchive ia (zis);

    T x;
    ia & x;
    return x;
  }
}
