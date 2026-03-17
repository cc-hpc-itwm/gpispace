#include <fmt/format.h>
#include <boost/lexical_cast.hpp>

#include <cassert>
#include <stdexcept>




      namespace gspc::util::boost::program_options
      {
        template<typename T, typename Validator>
          option<T, Validator>::option ( std::string const& name
                                       , std::string const& description
                                       )
            : _name (name)
            , _description (description)
            , _default_value (std::nullopt)
            , _default_value_string (std::nullopt)
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
              ( fmt::format ("missing key '{}' in variables map", name())
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
            std::optional<U> option<T, Validator>::get
              (parsed_values const& vm) const
        {
          return is_contained_in (vm)
            ? std::make_optional<U> (get_from (vm))
            : std::nullopt
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
              ( fmt::format
                ( "Failed to set option '{}' to '{}': Found value '{}'"
                , name()
                , (T (std::forward<Args> (args)...))
                , get_from (vm)
                )
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
              , fmt::format
                ( "{}\nSpecify also as '{} options... {}'"
                , description
                , open
                , close
                )
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
                ( gspc::util::boost::program_options::separated_argument_list_parser
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
            throw VersionException (_version.value());
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
              ( fmt::format ("Duplicate marker '{}'", cl._open)
              );
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
                ( fmt::format ("Duplicate marker '{}'", s.first)
                );
            }
          }

          if (!!other._version)
          {
            version (other._version.value());
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
          if (!!_version && _version.value() != version)
          {
            throw std::invalid_argument
              ( fmt::format
                ( "Different versions: '{}' vs '{}'"
                , _version.value()
                , version
                )
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
