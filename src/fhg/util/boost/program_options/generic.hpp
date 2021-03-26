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

#include <util-generic/program_options/separated_argument_list_parser.hpp>

#include <boost/optional.hpp>
#include <boost/program_options.hpp>

#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace fhg
{
  namespace util
  {
    namespace boost
    {
      using namespace ::boost;

      namespace program_options
      {
        class options;

        //! A container of values after parsing a command line.
        using parsed_values = ::boost::program_options::variables_map;

        //! A command line option of type \c T, to be used in \c
        //! options. Optionally, a \c Validator can be given.
        //! \see fhg::util::boost::program_options::validate()
        template<typename T, typename Validator = T>
          class option
        {
        public:
          //! Create an option with \a name and \a description, and no
          //! default value.
          option ( std::string const& name
                 , std::string const& description
                 );
          //! Create an option with \a name and \a description, and
          //! the default value created with \a args.
          template<typename... Args>
            option ( std::string const& name
                   , std::string const& description
                   , Args&&... args
                   );

          //! The name as given on construction.
          std::string const& name() const;
          //! The description as given on construction.
          std::string const& description() const;

          //! Check whether this option is contained in the given set
          //! of parsed values \a vm.
          bool is_contained_in (parsed_values const& vm) const;
          //! Retrieve this option from the given set of parsed values
          //! \a vm, throwing if it is not contained.
          T get_from (parsed_values const& vm) const;
          //! Retrieve this option from the given set of parsed values
          //! \a vm, or default to the given \a value if it is not
          //! contained.
          T get_from_or_value (parsed_values const& vm, T const& value) const;
          //! Retrieve this option from the given set of parsed values
          //! \a vm, or return \c boost::none if it is not
          //! contained. The value is implicitly casted to the
          //! requested type \c U, for easy use of validators.
          template<typename U = T>
            boost::optional<U> get (parsed_values const& vm) const;

          //! Ensure this options is not contained in the given set of
          //! parsed values \a vm (throw if it is), and insert it as
          //! if it was given with the value created by \a args.
          template<typename... Args>
            void set (parsed_values& vm, Args&&... args) const;

          //! Assuming this option is contained in the given set of
          //! parsed values \a vm, determine if the user specified it
          //! or whether it was implicitly defaulted.
          bool defaulted (parsed_values const& vm) const;

        private:
          friend class options;
          friend class embedded_command_line;

          ::boost::program_options::typed_value<Validator>* operator()() const;

          std::string const _name;
          std::string const _description;
          boost::optional<T> _default_value;
          boost::optional<std::string> _default_value_string;
        };

        //! A wrapper for `vector<string>` options to allow them being
        //! given in a less verbose manner, to be used in \c options.
        //!
        //! Assuming the option name `blorbs`, a user may specify the
        //! values `rubbel`, `--robbel kurt` and `ribbel` with either
        //!
        //!   --blorbs rubbel --blorbs="--robbel kurt" --blorbs ribbel
        //!
        //! or using this wrapper and `open` of `BLORBS[[` and `close
        //! of `]]BLORBS` they could just
        //!
        //!   BLORBS[[ rubbel --robbel kurt ribbel ]]BLORBS
        //!
        //! which especially helps with the ugliness of values having
        //! leading dashes as seen with `--robbel` above, which is
        //! also where the name comes from.
        class embedded_command_line : public option<std::vector<std::string>>
        {
        public:
          //! Create an embedded option with \a name and \a
          //! description, which on the command line begins after the
          //! token \a open and ends with the token \a close.
          //! \note The default value implicitly is the empty vector.
          embedded_command_line ( std::string const& name
                                , std::string const& description
                                , std::string const& open
                                , std::string const& close
                                );

        private:
          std::string const _open;
          std::string const _close;

          friend class options;
        };

        //! Custom exception thrown when `-h/--help` is among the command-line
        //! options.
        class HelpException : public std::runtime_error
        {
        public:
          explicit HelpException (const std::string&);
        };

        //! Custom exception thrown when `-v/--version` is among the
        //! command-line options.
        class VersionException : public std::runtime_error
        {
        public:
          explicit VersionException (const std::string&);
        };

        //! A group of command line options, both the description and
        //! after parsing also the values. Hierarchies are possible.
        class options
        {
        public:
          //! Create a group that in `--help` is named \a header.
          options (std::string const& header);

          //! Create a group that in `--help` is named \a header
          //! and uses total \a line_length when turned into a string.
          options (std::string const& header, unsigned line_length);

          //! Create a group that in `--help` is named \a header and
          //! and uses total \a line_length and \a min_description_length
          //! when turned into a string.
          options ( std::string const& header
                  , unsigned line_length
                  , unsigned min_description_length
                  );

          //! Parse the given \a args and validate against all options
          //! added beforehand, throwing on unknown arguments or
          //! errors in how they are specified.
          //! \note \a args may be `int argc, char const* argv` or a
          //! `vector<string> args`.
          template<typename... Args>
            parsed_values store_and_notify (Args&&... args) const;

          //! Add a given \a option as an optional parameter.
          //! \note This does not require the option to have a default
          //! value, but that requires manual handling when extracted.
          template<typename T, typename Validator>
            options& add (option<T, Validator> const& option);
          //! Add a given \a option as a required parameter.
          //! \note This implies the option can always be retrieved
          //! after storing, even if no default value is given.
          template<typename T, typename Validator>
            options& require (option<T, Validator> const& option);
          //! Add a `vector<string>` option as described by \a cl as
          //! an optional parameter.
          //! \note As there always is a default value (the empty
          //! vector), it can safely be retrieved without checking.
          //! \note There may be multiple embedded options, but their
          //! opening markers are required to be distinct.
          options& embed (embedded_command_line const& cl);
          //! Mark the given \a option as positional, i.e. the option
          //! any left over parameters are added to as value.
          // \todo Should this not also need to also explicitly
          // `add()` the option?!
          template<typename T, typename Validator>
            options& positional (option<T, Validator> const& option);

          //! Merge the options described in \a other into this set of
          //! options. This includes positionals, embeddeds and
          //! version, but not the constructor-given header.
          options& add (options const& other);
          //! Merge the raw Boost.ProgramOptions options description
          //! \a other into this set of options.
          options& add
            (::boost::program_options::options_description const& other);

          //! If a \a version is specified, the command line parser
          //! supports `--version` and `-v`, which if specified lets
          //! \c store_and_notify() throw the \a version given.
          //! \note May be called multiple times, but only with the
          //! same value.
          options& version (std::string const& version);

        private:
          template<typename T, typename Validator>
            options& add
              ( option<T, Validator> const& option
              , ::boost::program_options::typed_value<Validator>* value
              );

          ::boost::program_options::options_description _options_description;
          ::boost::program_options::positional_options_description _positional;
          std::map<std::string, std::pair<std::string, std::string>>
            _embedded_command_lines;
          boost::optional<std::string> _version;
        };
      }
    }
  }
}

#include <fhg/util/boost/program_options/generic.ipp>
