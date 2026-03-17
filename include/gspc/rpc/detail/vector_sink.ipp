

    namespace gspc::rpc::detail
    {
      inline vector_sink::vector_sink (std::vector<char>& vector)
        : _vector (vector)
      {}

      inline std::streamsize vector_sink::write
        (char_type const* s, std::streamsize n)
      {
        _vector.insert (_vector.end(), s, s + n);
        return n;
      }
    }
