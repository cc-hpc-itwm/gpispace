#ifndef KDM_COMPLEX_KDM_HPP
#define KDM_COMPLEX_KDM_HPP 1

namespace kdm
{
  static ::value::type initialize (const std::string & filename, long & wait)
  {
    ::value::structured_t config;

    std::cout << "initialize: use file " << filename << std::endl;

    std::ifstream file (filename.c_str());

    if (!file)
      throw std::runtime_error ("Lurcks, config file not good!?");

    while (!file.eof())
      {
        std::string s;
        file >> s;
        long v;
        file >> v;
        if (s.size())
          config[s] = v;
      }

    std::cout << "initialize: got config " << config << std::endl;

    wait = ::value::get_literal_value<long>
      (::value::get_field ("SUBVOLUMES_PER_OFFSET", config))
      ;

    std::cout << "initialize: wait = " << wait << std::endl;

    return config;
  }

  static void loadTT (const ::value::type & v)
  {
    std::cout << "loadTT: got config " << v << std::endl;
  }

  static void finalize (const ::value::type & v)
  {
    std::cout << "finalize: got config " << v << std::endl;
  }

  static ::value::type load ( const ::value::type & config
                            , const ::value::type & bunch
                            , const long & store
                            )
  {
    std::cout << "load: got config " << config << std::endl;
    std::cout << "load: got bunch " << bunch << std::endl;
    std::cout << "load: got store " << store << std::endl;

    ::value::structured_t loaded_bunch;

    loaded_bunch["bunch"] = bunch;
    loaded_bunch["store"] = store;
    loaded_bunch["seen"] = literal::type (bitsetofint::type());
    loaded_bunch["wait"] = ::value::get_literal_value<long>
      (::value::get_field ("SUBVOLUMES_PER_OFFSET", config))
      ;

    std::cout << "load: loaded_bunch " << loaded_bunch << std::endl;

    return loaded_bunch;
  }

  static void write (const ::value::type & config, const ::value::type & volume)
  {
    std::cout << "write: got config " << config << std::endl;
    std::cout << "write: got volume " << volume << std::endl;
  }

  static ::value::type
  process ( const ::value::type & config
          , const ::value::type & volume
          )
  {
    std::cout << "process: got config " << config << std::endl;
    std::cout << "process: got volume " << volume << std::endl;

    ::value::type volume_processed (volume);

    const ::value::type buffer0 (::value::get_field ("buffer0", volume));
    const bool assigned0
      (::value::get_literal_value<bool>(::value::get_field ("assigned", buffer0)));
    const bool filled0
      (::value::get_literal_value<bool>(::value::get_field ("filled", buffer0)));

    const ::value::type buffer1 (::value::get_field ("buffer1", volume));
    const bool assigned1
      (::value::get_literal_value<bool>(::value::get_field ("assigned", buffer1)));
    const bool filled1
      (::value::get_literal_value<bool>(::value::get_field ("filled", buffer1)));

    // die hier implementierte Logik ist noch nicht optimal: es wird
    // einfach nur jeder bufffer geladen, der assigned aber nicht
    // gefüllt ist und die buffer, die geladen sind, werden verarbeitet.

    const long wait
      (::value::get_literal_value<long>(::value::get_field ("wait", volume)));

    ::value::field("assigned", ::value::field("buffer0", volume_processed)) = assigned0 && !filled0;
    ::value::field("filled", ::value::field("buffer0", volume_processed)) = assigned0 && !filled0;
    ::value::field("free", ::value::field("buffer0", volume_processed)) = assigned0 && !filled0;
    ::value::field("assigned", ::value::field("buffer1", volume_processed)) = assigned1 && !filled1;
    ::value::field("filled", ::value::field("buffer1", volume_processed)) = assigned1 && !filled1;
    ::value::field("free", ::value::field("buffer1", volume_processed)) = assigned1 && !filled1;
    ::value::field("wait", volume_processed) = wait - ((filled0) ? 1 : 0) - ((filled1) ? 1 : 0);

    return volume_processed;
  }
}

namespace module
{
  template <typename ModuleCall, typename Context, typename OutputList>
  void eval (const ModuleCall & mf, const Context & ctxt, OutputList & output)
  {
    if (mf.module() == "kdm")
      {

        if (mf.function() == "loadTT")
          {
            const value::type & v (ctxt.value("config"));
            kdm::loadTT (v);
            output.push_back (std::make_pair (control(), "trigger"));
          }

        else if (mf.function() == "initialize")
          {
            const std::string & filename
              (::value::get_literal_value<std::string> (ctxt.value("config_file")));
            long wait (0);
            const value::type config (kdm::initialize (filename, wait));
            output.push_back (std::make_pair (config, "config"));
            output.push_back (std::make_pair (literal::type(wait), "wait"));
            output.push_back (std::make_pair (control(), "trigger"));
            bitsetofint::type bs;
            bs.ins (0);
            output.push_back (std::make_pair (literal::type(bs), "wanted"));
          }

        else if (mf.function() == "finalize")
          {
            const value::type & v (ctxt.value("config"));
            kdm::finalize (v);
            output.push_back (std::make_pair (control(), "done"));
          }

        else if (mf.function() == "load")
          {
            const value::type & config (ctxt.value("config"));
            const value::type & bunch (ctxt.value("bunch"));
            const long & store
              (::value::get_literal_value<long> (ctxt.value("empty_store")));
            const value::type loaded_bunch (kdm::load (config, bunch, store));
            output.push_back (std::make_pair (loaded_bunch, "loaded_bunch"));
          }

        else if (mf.function() == "write")
          {
            const value::type & config (ctxt.value("config"));
            const value::type & volume (ctxt.value("volume"));
            kdm::write (config, volume);
            output.push_back (std::make_pair (volume, "volume"));
          }

        else if (mf.function() == "process")
          {
            const value::type & config (ctxt.value("config"));
            const value::type & volume (ctxt.value("volume"));
            const value::type volume_processed (kdm::process (config, volume));

            output.push_back ( std::make_pair ( volume_processed
                                              , "volume_processed"
                                              )
                             );
          }
      }
  }
}

#endif
