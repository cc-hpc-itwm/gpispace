#ifndef CFG_HPP
#define CFG_HPP 1

#include <string>
#include <fhglog/fhglog.hpp>
#include <sdpa/modules/assert.hpp>

#include <fvm-pc/pc.hpp>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
  fvmAllocHandle_t velocity_field;
  fvmAllocHandle_t input_data;
  fvmAllocHandle_t output_volume;
  int nx, ny, nt, nz;
} cfg_t;

int c_read_config(const std::string &config_file, cfg_t *);

#ifdef __cplusplus
}
#endif


#endif
