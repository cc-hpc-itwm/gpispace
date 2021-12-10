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

#include <xml/parse/util/mk_fstream.hpp>

#include <xml/parse/error.hpp>

#include <boost/filesystem.hpp>

namespace xml
{
  namespace parse
  {
    namespace util
    {
      check_no_change_fstream::check_no_change_fstream
      ( state::type const& state
      , ::boost::filesystem::path const& file
      , std::function<bool (std::string const&, std::string const&)> equal
      )
        : _state (state)
        , _file (file)
        , _equal (equal)
        , _oss ()
      {}
      check_no_change_fstream::~check_no_change_fstream()
      {
        commit();
      }
      check_no_change_fstream&
      check_no_change_fstream::operator << (std::ostream& (*f)(std::ostream&))
      {
        f (_oss); return *this;
      }
      void check_no_change_fstream::commit() const
      {
        if (::boost::filesystem::is_regular_file (_file))
          {
            std::ifstream ifs (_file.string().c_str());

            if (!ifs.good())
              {
                throw error::could_not_open_file (_file);
              }

            std::stringstream sstr;
            ifs >> std::noskipws >> sstr.rdbuf();

            if (_equal (_oss.str(), sstr.str()))
              {
                return;
              }

            if (!_state.force_overwrite_file())
              {
                throw error::file_already_there (_file);
              }

            if (_state.do_file_backup())
              {
                const ::boost::filesystem::path backup
                  (_file.string() + _state.backup_extension());

                _state.warn (warning::backup_file (_file, backup));

                ::boost::filesystem::copy_file
                  ( _file
                  , backup
                  , ::boost::filesystem::copy_option::overwrite_if_exists
                  );
              }

            _state.warn (warning::overwrite_file (_file));
          }

        write();
      }
      void check_no_change_fstream::write() const
      {
        ::boost::filesystem::path path (_file);

        path.remove_filename();

        ::boost::filesystem::create_directories (path);

        if (not ::boost::filesystem::is_directory (path))
          {
            throw error::could_not_create_directory (path);
          }

        std::ofstream stream (_file.string().c_str());

        if (!stream.good())
          {
            throw error::could_not_open_file (_file);
          }

        stream << _oss.str();
      }
    }
  }
}
