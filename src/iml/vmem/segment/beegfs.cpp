// Copyright (C) 2015,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/iml/vmem/segment/beegfs.hpp>

#include <gspc/util/read_lines.hpp>
#include <gspc/util/syscall/directory.hpp>

#include <beegfs/beegfs.h>

#include <filesystem>

#include <string>





        namespace gspc::iml::vmem::segment::beegfs
        {
          namespace
          {
            std::filesystem::path config_file (int fd)
            {
              char* raw_path (nullptr);
              if (!beegfs_getRuntimeConfigFile (fd, &raw_path))
              {
                throw std::runtime_error
                  ( "unable to get config file: "
                  + std::string (strerror (errno))
                  );
              }
              std::filesystem::path path (raw_path);
              free (raw_path);
              return path;
            }
            bool tuneUseGlobalFileLocks (int fd)
            {
              for ( std::string const& line
                  : gspc::util::read_lines (beegfs::config_file (fd))
                  )
              {
                if (line == "tuneUseGlobalFileLocks = 1")
                {
                  return true;
                }
                else if (line == "tuneUseGlobalFileLocks = 0")
                {
                  return false;
                }
              }
              throw std::runtime_error
                ("'tuneUseGlobalFileLocks = [01]' not found in BeeGFS config");
            }

            void check_requirements (int fd)
            {
              if (!beegfs_testIsBeeGFS (fd))
              {
                throw std::runtime_error ("not a BeeGFS mountpoint");
              }
              if (!tuneUseGlobalFileLocks (fd))
              {
                throw std::runtime_error
                  ("'tuneUseGlobalFileLocks' required but disabled");
              }
            }
          }

          void check_requirements (std::filesystem::path const& path)
          try
          {
            gspc::util::syscall::directory const directory (path);
            check_requirements (directory.fd());
          }
          catch (...)
          {
            std::throw_with_nested (requirements_not_met (path));
          }
        }
