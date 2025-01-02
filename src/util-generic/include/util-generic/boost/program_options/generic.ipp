// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <cassert>
#include <stdexcept>

namespace fhg
{
  namespace util
  {
    namespace boost
    {
      namespace program_options
      {
        template<typename T, typename Validator>
          option<T, Validator>::option ( std::string const& name
                                       , std::string const& description
                                       )
            : _name (name)
            , _description (description)
            , _default_value (::boost::none)
            , _default_value_string (::boost::none)
        {}
        template<typename T, typename Validator>
          template<typename... Args>
            option<T, Validator>::option ( std::string const& name
                                         , std::string const& description
                                         , Args&&... args
                                         )
              : option (name, description)
        {
          _default_value.emplace (std::forward<Args> (args)...);
          _default_value_string.emplace
            (::boost::lexical_cast<std::string> (*_default_value));
        }

        template<typename T, typename Validator>
          std::string const& option<T, Validator>::name() const
        {
          return _name;
        }
        template<typename T, typename Validator>
          std::string const& option<T, Validator>::description() const
        {
          return _description;
        }

        template<typename T, typename Validator>
          bool option<T, Validator>::is_contained_in
            (parsed_values const& vm) const
        {
          return vm.count (name());
        }
        template<typename T, typename Validator>
          T option<T, Validator>::get_from (parsed_values const& vm) const
        {
          if (!is_contained_in (vm))
          {
            throw std::logic_error
              (( ::boost::format ("missing key '%1%' in variables map")
               % name()
               ).str()
              );
          }

          // \note static_cast to be backward compatible in option validation
          return static_cast<T> (vm.at (name()).template as<Validator>());
        }
        template<typename T, typename Validator>
          T option<T, Validator>::get_from_or_value
            (parsed_values const& vm, T const& value) const
        {
          return is_contained_in (vm) ? get_from (vm) : value;
        }
        template<typename T, typename Validator>
          template<typename U>
            ::boost::optional<U> option<T, Validator>::get
              (parsed_values const& vm) const
        {
          return is_contained_in (vm)
            ? ::boost::make_optional<U> (get_from (vm))
            : ::boost::none
            ;
        }
        template<typename T, typename Validator>
          template<typename... Args>
            void option<T, Validator>::set
              (parsed_values& vm, Args&&... args) const
        {
          if (is_contained_in (vm))
          {
            throw std::logic_error
              (( ::boost::format
                   ("Failed to set option '%1%' to '%2%': Found value '%3%'")
               % name()
               % (T (std::forward<Args> (args)...))
               % get_from (vm)
               ).str()
              );
          }

          vm.insert ( std::make_pair
                        ( name()
                        , ::boost::program_options::variable_value
                            (T (std::forward<Args> (args)...), false)
                        )
                    );
        }
        template<typename T, typename Validator>
          bool option<T, Validator>::defaulted (parsed_values const& vm) const
        {
          return vm.at (name()).defaulted();
        }

        template<typename T, typename Validator>
          ::boost::program_options::typed_value<Validator>*
            option<T, Validator>::operator()() const
        {
          // \note The option description takes ownership of the
          // `value`, leading to `option` globals breaking when used
          // twice. As that's a pattern we have, delay creation of the
          // `value` until we pass it to Boost, and only have a set of
          // constructor arguments as members.
          auto value (::boost::program_options::value<Validator>());
          if (_default_value)
          {
            assert (_default_value_string);
            value->default_value (*_default_value, *_default_value_string);
          }
          return value;
        }

        inline embedded_command_line::embedded_command_line
            ( std::string const& name
            , std::string const& description
            , std::string const& open
            , std::string const& close
            )
          : option<std::vector<std::string>>
              ( name
              , ( ::boost::format ("%1%\nSpecify also as '%2% options... %3%'")
                % description
                % open
                % close
                ).str()
              )
          , _open (open)
          , _close (close)
        {
          _default_value.emplace();
          _default_value_string.emplace();
        }

        inline HelpException::HelpException (const std::string& options_description)
          : std::runtime_error (options_description)
        {}

        inline VersionException::VersionException (const std::string& version)
          : std::runtime_error (version)
        {}

        inline options::options (std::string const& header)
          : _options_description (header)
        {}

        inline options::options
          (std::string const& header, unsigned line_length)
          : _options_description (header, line_length)
        {}

        inline options::options
          ( std::string const& header
          , unsigned line_length
          , unsigned min_description_length
          )
          : _options_description (header, line_length, min_description_length)
        {}

        template<typename... Args>
          parsed_values options::store_and_notify (Args&&... args) const
        {
          ::boost::program_options::options_description common ("Common");
          common.add_options() ("help,h", "show the options description");

          if (!!_version)
          {
            common.add_options() ("version,v", "show the program version");
          }

          ::boost::program_options::options_description
            options_description (_options_description);
          options_description.add (common);

          parsed_values vm;
          ::boost::program_options::store
            ( ::boost::program_options::command_line_parser
                (std::forward<Args> (args)...)
            . options (options_description)
            . positional (_positional)
            . style
                ( ::boost::program_options::command_line_style::default_style
                ^ ::boost::program_options::command_line_style::allow_guessing
                )
            . extra_style_parser
                ( fhg::util::boost::program_options::separated_argument_list_parser
                    (_embedded_command_lines)
                )
            . run()
            , vm
            );

          if (vm.count ("help"))
          {
            throw HelpException
              (::boost::lexical_cast<std::string> (options_description));
          }

          if (vm.count ("version"))
          {
            throw VersionException (_version.get());
          }

          vm.notify();

          return vm;
        }

        template<typename T, typename Validator>
          options& options::add (option<T, Validator> const& option)
        {
          return add (option, option());
        }
        template<typename T, typename Validator>
          options& options::require (option<T, Validator> const& option)
        {
          return add (option, option()->required());
        }
        inline options& options::embed (embedded_command_line const& cl)
        {
          if ( !_embedded_command_lines
             . emplace (cl._open, std::make_pair (cl._close, cl.name()))
             . second
             )
          {
            throw std::invalid_argument
              ((::boost::format ("Duplicate marker '%1%'") % cl._open).str());
          }

          require (cl);

          return *this;
        }
        template<typename T, typename Validator>
          options& options::positional (option<T, Validator> const& option)
        {
          if (_positional.max_total_count() > 0)
          {
            throw std::logic_error ("more than one positional parameter");
          }

          _positional.add (option.name().c_str(), -1);

          return *this;
        }

        inline options& options::add (options const& other)
        {
          _options_description.add (other._options_description);

          if (other._positional.max_total_count() > 0)
          {
            if (_positional.max_total_count() > 0)
            {
              throw std::logic_error ("more than one positional parameter");
            }

            _positional = other._positional;
          }

          for (auto const& s : other._embedded_command_lines)
          {
            if (!_embedded_command_lines.emplace (s).second)
            {
              throw std::invalid_argument
                ((::boost::format ("Duplicate marker '%1%'") % s.first).str());
            }
          }

          if (!!other._version)
          {
            version (other._version.get());
          }

          return *this;
        }
        inline options& options::add
          (::boost::program_options::options_description const& other)
        {
          _options_description.add (other);

          return *this;
        }

        inline options& options::version (std::string const& version)
        {
          if (!!_version && _version.get() != version)
          {
            throw std::invalid_argument
              ( ( ::boost::format ("Different versions: '%1%' vs '%2%'")
                % _version.get()
                % version
                ).str()
              );
          }

          _version = version;

          return *this;
        }

        template<typename T, typename Validator>
          options& options::add
            ( option<T, Validator> const& option
            , ::boost::program_options::typed_value<Validator>* value
            )
        {
          _options_description.add_options()
            (option.name().c_str(), value, option.description().c_str());

          return *this;
        }
      }
    }
  }
}
