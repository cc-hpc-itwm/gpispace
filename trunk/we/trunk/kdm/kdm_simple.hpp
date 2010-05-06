#ifndef KDM_SIMPLE_KDM_HPP
#define KDM_SIMPLE_KDM_HPP 1

#include <fstream>

namespace kdm
{
  static value::type initialize (const std::string & filename, long & wait)
  {
    value::structured_t config;

    std::cout << "initialize: use file " << filename << std::endl;

    std::ifstream file (filename.c_str());

    if (!file)
      throw std::runtime_error ("Lurcks, config file not good");

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

    wait = value::get_literal_value<long> (value::get_field ("OFFSETS", config))
         * value::get_literal_value<long> (value::get_field ("SUBVOLUMES_PER_OFFSET", config))
      ;

    std::cout << "initialize: wait = " << wait << std::endl;

    return config;
  }

  static void loadTT (const value::type & v)
  {
    std::cout << "loadTT: got config " << v << std::endl;
  }

  static void finalize (const value::type & v)
  {
    std::cout << "finalize: got config " << v << std::endl;
  }

  static void load (const value::type & config, const value::type & bunch)
  {
    std::cout << "load: got config " << config << std::endl;
    std::cout << "load: got bunch " << bunch << std::endl;
  }
  static void write (const value::type & config, const value::type & volume)
  {
    std::cout << "write: got config " << config << std::endl;
    std::cout << "write: got volume " << volume << std::endl;
  }
  static void
  process ( const value::type & config
          , const value::type & bunch
          , long & wait
          )
  {
    std::cout << "process: got config " << config << std::endl;
    std::cout << "process: got bunch " << bunch << std::endl;
    std::cout << "process: got wait " << wait << std::endl;

    --wait;
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
              (value::get_literal_value<std::string> (ctxt.value("config_file")));
            long wait (0);
            const value::type & config (kdm::initialize (filename, wait));
            output.push_back (std::make_pair (config, "config"));
            output.push_back (std::make_pair (literal::type(wait), "wait"));
            output.push_back (std::make_pair (control(), "trigger"));
          }
        else if (mf.function() == "finalize")
          {
            const value::type & v (ctxt.value("config"));
            kdm::finalize (v);
            output.push_back (std::make_pair (control(), "trigger"));
          }
        else if (mf.function() == "load")
          {
            const value::type & config (ctxt.value("config"));
            const value::type & bunch (ctxt.value("bunch"));
            kdm::load (config, bunch);
            output.push_back (std::make_pair (bunch, "bunch"));
          }
        else if (mf.function() == "write")
          {
            const value::type & config (ctxt.value("config"));
            const value::type & volume (ctxt.value("volume"));
            kdm::write (config, volume);
            output.push_back (std::make_pair (control(), "done"));
          }
        else if (mf.function() == "process")
          {
            const value::type & config (ctxt.value("config"));
            const value::type & bunch (ctxt.value("bunch"));
            long wait (value::get_literal_value<long> (ctxt.value("wait")));
            kdm::process (config, bunch, wait);
            output.push_back (std::make_pair (wait, "wait"));
          }
      }
  }
}

#endif
