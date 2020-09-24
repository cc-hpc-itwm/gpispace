namespace iml
{
  template<typename Archive>
    void gaspi_segment_description::serialize (Archive& ar, unsigned int)
  {
    ar & _communication_buffer_size;
    ar & _communication_buffer_count;
  }

  template<typename Archive>
    void beegfs_segment_description::serialize (Archive& ar, unsigned int)
  {
    // Inlined copy of util-generic/serialization/boost/filesystem/path.hpp
    // to avoid dependency in public API.
    boost::filesystem::path::string_type path_string;
    if (Archive::is_saving::value)
    {
      path_string = _path.string();
    }
    ar & BOOST_SERIALIZATION_NVP (path_string);
    if (Archive::is_loading::value)
    {
      _path = path_string;
    }
  }
}
