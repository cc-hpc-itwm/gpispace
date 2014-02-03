// alexander.petry@itwm.fraunhofer.de

#ifndef FHGKERNEL_START_AND_RUN_FHGKERNEL_HPP
#define FHGKERNEL_START_AND_RUN_FHGKERNEL_HPP

#include <fhg/plugin/core/kernel.hpp> // search_path_t

#include <fhglog/Logger.hpp> // Logger::ptr_t

int setup_and_run_fhgkernel ( bool daemonize
                            , bool keep_going
                            , std::vector<std::string> mods_to_load
                            , std::vector<std::string> config_vars
                            , std::string pidfile
                            , std::string kernel_name
                            , fhg::core::kernel_t::search_path_t search_path
                            , fhg::log::Logger::ptr_t logger
                            );

#endif
