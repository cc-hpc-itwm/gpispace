// Copyright (C) 2011,2013-2015,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/xml/parse/util/mk_fstream.hpp>

#include <gspc/xml/parse/error.hpp>

#include <filesystem>

#include <exception>
#include <utility>



    namespace gspc::xml::parse::util
    {
      check_no_change_fstream::check_no_change_fstream
      ( state::type const& state
      , std::filesystem::path const& file
      , std::function<bool (std::string const&, std::string const&)> equal
      )
        : _state (state)
        , _file (file)
        , _equal (equal)
        , _oss ()
      {}
      check_no_change_fstream::~check_no_change_fstream()
      try
      {
        commit();
      }
      catch (...)
      {
        std::ignore = std::current_exception();
      }
      check_no_change_fstream&
      check_no_change_fstream::operator << (std::ostream& (*f)(std::ostream&))
      {
        f (_oss); return *this;
      }
      void check_no_change_fstream::commit() const
      {
        if (std::filesystem::is_regular_file (_file))
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
                const std::filesystem::path backup
                  (_file.string() + _state.backup_extension());

                _state.warn (warning::backup_file (_file, backup));

                std::filesystem::copy
                  ( _file
                  , backup
                  , std::filesystem::copy_options::overwrite_existing
                  );
              }

            _state.warn (warning::overwrite_file (_file));
          }

        write();
      }
      void check_no_change_fstream::write() const
      {
        std::filesystem::path path (_file);

        path.remove_filename();

        std::filesystem::create_directories (path);

        if (not std::filesystem::is_directory (path))
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
