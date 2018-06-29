namespace fhg
{
  namespace logging
  {
    template<typename Receiver>
      file_sink<Receiver>::file_sink
        ( typename Receiver::endpoint_t const& emitter
        , boost::filesystem::path const& target
        , std::function<void (std::ostream&, message const&)> formatter
        , boost::optional<std::size_t> flush_interval
        )
      : basic_file_sink
          (target, std::move (formatter), std::move (flush_interval))
      , _receiver
          ( emitter
          , [&] (message const& m) { return dispatch_append (m); }
          )
    {}
  }
}
