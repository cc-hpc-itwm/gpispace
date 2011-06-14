#ifndef FHG_PLUGIN_PLUGIN_HPP
#define FHG_PLUGIN_PLUGIN_HPP 1

namespace fhg
{
  class plugin
  {
  public:
    virtual ~plugin () {}

    virtual std::string const & info () const = 0;
    virtual std::string const & type () const = 0;

    virtual void on_load () = 0;
    virtual void on_unload () = 0;
  };
}

#endif
