#pragma once

#include <pnete/plugin/plugin_base.hpp>

extern "C"
{
  fhg::pnete::plugin::plugin_base* fhg_pnete_create_plugin (QObject* parent);
}
