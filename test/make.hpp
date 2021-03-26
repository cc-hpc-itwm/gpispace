// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <drts/drts.hpp>

#include <util-generic/ostream/modifier.hpp>
#include <util-generic/temporary_path.hpp>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

#include <list>
#include <memory>
#include <string>

namespace test
{
  namespace option
  {
    struct generic : public fhg::util::ostream::modifier
    {
      generic (std::string const& key, char const* const value);
      generic (std::string const& key, std::string const&);
      generic (std::string const& key, boost::format const&);
      generic (std::string const& key, boost::filesystem::path const&);
      virtual std::ostream& operator() (std::ostream& os) const override;

    private:
      std::string const _key;
      std::string const _value;
    };
    struct options : public fhg::util::ostream::modifier
    {
      template<typename Option, typename... Args>
        options& add (Args&&... args)
      {
        _options.emplace_back (new Option (std::forward<Args> (args)...));

        return *this;
      }
      virtual std::ostream& operator() (std::ostream& os) const override;

    private:
      std::list<std::unique_ptr<generic>> _options;
    };

    struct include : public generic
    {
      include (boost::filesystem::path const& path)
        : generic ("search-path", path)
      {}
    };

    namespace gen
    {
      struct cxx_flag : public generic
      {
        template<typename T>
          cxx_flag (T x)
            : generic ("gen-cxxflags", std::forward<T> (x))
        {}
      };

      struct ld_flag : public generic
      {
        template<typename T>
          ld_flag (T x)
            : generic ("gen-ldflags", std::forward<T> (x))
        {}
      };

      struct include : public cxx_flag
      {
        include (boost::filesystem::path const&);
      };

      struct link : public ld_flag
      {
        link (boost::filesystem::path const&);
      };

      struct library_path : public ld_flag
      {
        library_path (boost::filesystem::path const&);
      };
    }
  }

  class make_net;
  class make_net_lib_install;

  class make
  {
  public:
    boost::filesystem::path const& pnet() const
    {
      return _pnet;
    }

  private:
    friend class make_net;
    friend class make_net_lib_install;

    make ( gspc::installation const& installation
         , std::string const& main
         , boost::filesystem::path const& source_directory
         , boost::optional<boost::filesystem::path> const& lib_destdir
           = boost::none
         , option::options const& = option::options()
         );

    make (make const&) = delete;
    make& operator= (make const&) = delete;
    make (make&&) = delete;
    make& operator= (make&&) = delete;

    std::string const _main;
    fhg::util::temporary_path const _build_directory;
    boost::filesystem::path const _pnet;
  };

  class make_net : public make
  {
  public:
    make_net ( gspc::installation const& installation
             , std::string const& main
             , boost::filesystem::path const& source_directory
             )
      : make (installation, main, source_directory)
    {}
  };

  class make_net_lib_install : public make
  {
  public:
    make_net_lib_install
      ( gspc::installation const& installation
      , std::string const& main
      , boost::filesystem::path const& source_directory
      , boost::filesystem::path const& lib_destdir
      , option::options const& options = option::options()
      )
        : make (installation, main, source_directory, lib_destdir, options)
    {}
  };
}
