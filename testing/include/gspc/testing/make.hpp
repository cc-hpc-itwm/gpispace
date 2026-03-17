// Copyright (C) 2014-2016,2021-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/drts/drts.hpp>

#include <gspc/util/ostream/modifier.hpp>
#include <gspc/util/temporary_path.hpp>

#include <filesystem>
#include <fmt/core.h>
#include <list>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>

namespace gspc::testing
{
  namespace option
  {
    struct generic : public gspc::util::ostream::modifier
    {
      generic (std::string const& key, char const* value);
      generic (std::string const& key, std::string const&);
      generic (std::string const& key, std::filesystem::path const&);
      std::ostream& operator() (std::ostream& os) const override;

    private:
      std::string const _key;
      std::string const _value;
    };
    struct options : public gspc::util::ostream::modifier
    {
      template<typename Option, typename... Args>
        options& add (Args&&... args)
      {
        _options.emplace_back (new Option (std::forward<Args> (args)...));

        return *this;
      }
      std::ostream& operator() (std::ostream& os) const override;

    private:
      std::list<std::unique_ptr<generic>> _options;
    };

    struct include : public generic
    {
      include (std::filesystem::path const& path)
        : generic ("search-path", path)
      {}
    };

    namespace gen
    {
      struct cxx_flag : public generic
      {
        cxx_flag (std::string flag)
          : generic ("gen-cxxflags", flag)
        {}
      };

      struct ld_flag : public generic
      {
        ld_flag (std::string flag)
          : generic ("gen-ldflags", flag)
        {}
      };

      struct include : public cxx_flag
      {
        include (std::filesystem::path const& path)
          : cxx_flag {fmt::format ("'-I {}'", path.string())}
        {}
      };

      struct link : public ld_flag
      {
        link (std::filesystem::path const& path)
          : ld_flag (path.string())
        {}
      };

      struct library_path : public ld_flag
      {
        library_path (std::filesystem::path const& path)
          : ld_flag {fmt::format ("'-L {}'", path.string())}
        {}
      };
    }
  }

  class make_net;
  class make_net_lib_install;

  class make
  {
  public:
    std::filesystem::path const& pnet() const
    {
      return _pnet;
    }

  private:
    friend class make_net;
    friend class make_net_lib_install;

    make ( gspc::installation const& installation
         , std::string const& main
         , std::filesystem::path const& source_directory
         , std::optional<std::filesystem::path> const& lib_destdir
           = std::nullopt
         , option::options const& = option::options()
         );

    make (make const&) = delete;
    make& operator= (make const&) = delete;
    make (make&&) = delete;
    make& operator= (make&&) = delete;
    ~make() = default;

    std::string const _main;
    gspc::util::temporary_path const _build_directory;
    std::filesystem::path const _pnet;
  };

  class make_net : public make
  {
  public:
    make_net ( gspc::installation const& installation
             , std::string const& main
             , std::filesystem::path const& source_directory
             )
      : make ( installation
             , main
             , source_directory
             )
    {}
  };

  class make_net_lib_install : public make
  {
  public:
    make_net_lib_install
      ( gspc::installation const& installation
      , std::string const& main
      , std::filesystem::path const& source_directory
      , std::filesystem::path const& lib_destdir
      , option::options const& options = option::options()
      )
        : make ( installation
               , main
               , source_directory
               , lib_destdir
               , options
               )
    {}
  };
}
