// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_PNETE_PLUGIN_PLUGIN_API_HPP
#define FHG_PNETE_PLUGIN_PLUGIN_API_HPP

#include <pnete/plugin/plugin_base.hpp>

extern "C"
{
  fhg::pnete::plugin::plugin_base* fhg_pnete_create_plugin (QObject* parent);
}

#endif
