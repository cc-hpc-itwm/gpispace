#ifndef FHG_PLUGIN_DESCRIPTOR_HPP
#define FHG_PLUGIN_DESCRIPTOR_HPP 1

struct fhg_plugin_descriptor_t
{
  const char *magic;
  const char *name;
  const char *description;
  const char *author;
  const char *version;
  const char *tstamp;
  const char *license;
  const char *depends;
  const char *featurekey;
  const char *buildrev;
  const char *compiler;
  // >= version 1.1
  const char *provides;
};

#endif
