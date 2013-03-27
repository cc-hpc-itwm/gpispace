// mirko.rahn@itwm.fraunhofer.de

#include <xml/parse/util/mk_fstream.hpp>

#include <xml/parse/error.hpp>

#include <fhg/util/filesystem.hpp>

namespace xml
{
  namespace parse
  {
    namespace util
    {
      std::ofstream & mk_fstream ( std::ofstream & stream
                                 , const state::type & state
                                 , const boost::filesystem::path & file
                                 )
      {
        boost::filesystem::path path (file);

        path.remove_filename();

        if (!fhg::util::mkdirs (path))
          {
            throw error::could_not_create_directory (path);
          }

        if (boost::filesystem::exists (file))
          {
            state.warn (warning::overwrite_file (file));
          }

        stream.open (file.string().c_str());

        if (!stream.good())
          {
            throw error::could_not_open_file (file);
          }

        return stream;
      }

      check_no_change_fstream::check_no_change_fstream
      ( const state::type& state
      , const boost::filesystem::path& file
      )
        : _state (state)
        , _file (file)
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
        if (boost::filesystem::is_regular_file (_file))
          {
            std::ifstream ifs (_file.string().c_str());

            if (!ifs.good())
              {
                throw error::could_not_open_file (_file);
              }

            std::stringstream sstr;
            ifs >> std::noskipws >> sstr.rdbuf();

            if (_oss.str() == sstr.str())
              {
                return;
              }

            if (!_state.force_overwrite_file())
              {
                throw error::file_already_there (_file);
              }

            if (_state.do_file_backup())
              {
                const boost::filesystem::path backup
                  (_file.string() + _state.backup_extension());

                _state.warn (warning::backup_file (_file, backup));

                boost::filesystem::copy_file
                  ( _file
                  , backup
                  , boost::filesystem::copy_option::overwrite_if_exists
                  );
              }

            _state.warn (warning::overwrite_file (_file));
          }

        write();
      }
      void check_no_change_fstream::write() const
      {
        boost::filesystem::path path (_file);

        path.remove_filename();

        if (!fhg::util::mkdirs (path))
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
