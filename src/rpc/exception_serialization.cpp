// bernd.loerwald@itwm.fraunhofer.de

#include <rpc/exception_serialization.hpp>

#include <fhg/util/boost/serialization/blank.hpp>
#include <fhg/util/boost/serialization/error_code.hpp>
#include <fhg/util/boost/serialization/tuple.hpp>
#include <fhg/util/macros.hpp>

#include <boost/blank.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/variant/variant.hpp>

#include <future>
#include <list>
#include <regex>
#include <stdexcept>
#include <system_error>
#include <tuple>
#include <unordered_map>

namespace fhg
{
  namespace rpc
  {
    namespace exception
    {
      namespace
      {
        //! \note indentation broken on purpose: is equal to inheritance graph
        enum class std_exception_types
        {
          bad_alloc,
          bad_cast,
          bad_exception,
          bad_function_call,
          bad_typeid,
          bad_weak_ptr,
          logic_error,
            domain_error,
            future_error,
            invalid_argument,
            length_error,
            out_of_range,
          runtime_error,
            overflow_error,
            range_error,
            //! \todo bug in libstdcxx of gcc 4.8: undefined reference to
            //! `std::regex_error::regex_error(std::regex_constants::error_type)'
            // regex_error,
            system_error,
              ios_base__failure,
            underflow_error,
        };

        struct builtin_exception_data
        {
          //! \note Serialization
          builtin_exception_data() = default;

          [[noreturn]] void throw_with_nested() const;
          std::exception_ptr to_exception_ptr() const;

          static boost::optional<builtin_exception_data>
            from_exception_ptr (std::exception_ptr);

        private:
          using system_error_data_type =
            std::tuple<bool, std::string, std::error_code>;
          using data_type = boost::variant< boost::blank
                                          , std::string
                                          , system_error_data_type
                                          , std::error_code
                                          , std::regex_constants::error_type
                                          >;

          builtin_exception_data (std_exception_types type, data_type data)
            : _type (std::move (type))
            , _data (std::move (data))
          {}

          std_exception_types _type;
          data_type _data;

          friend class boost::serialization::access;
          template<class Archive>
            void serialize (Archive& ar, const unsigned int)
          {
            ar & _type;
            ar & _data;
          }
        };

        struct user_defined_exception_data
        {
          //! \note Serialization
          user_defined_exception_data() = default;

          [[noreturn]] void throw_with_nested
            (serialization_functions const&) const;
          std::exception_ptr to_exception_ptr
            (serialization_functions const&) const;

          static boost::optional<user_defined_exception_data>
            from_exception_ptr ( std::exception_ptr
                               , serialization_functions const&
                               , boost::optional<builtin_exception_data>
                               );

        private:
          user_defined_exception_data
              ( std::string function
              , std::string blob
              , boost::optional<builtin_exception_data> fallback
              )
            : _function (std::move (function))
            , _blob (std::move (blob))
            , _fallback (std::move (fallback))
          {}

          std::string _function;
          std::string _blob;
          boost::optional<builtin_exception_data> _fallback;

          friend class boost::serialization::access;
          template<class Archive>
            void serialize (Archive& ar, const unsigned int)
          {
            ar & _function;
            ar & _blob;
            ar & _fallback;
          }
        };

        struct nested_exception_data;
        struct aggregated_exception_data;

        using exception_data = boost::variant
          < builtin_exception_data
          , boost::recursive_wrapper<nested_exception_data>
          , boost::recursive_wrapper<aggregated_exception_data>
          , user_defined_exception_data
          >;

        struct nested_exception_data
        {
          exception_data _this;
          exception_data _nested;

          //! \note Serialization
          nested_exception_data() = default;

        private:
          friend class boost::serialization::access;
          template<class Archive>
            void serialize (Archive& ar, const unsigned int)
          {
            ar & _this;
            ar & _nested;
          }
        };

        struct aggregated_exception_data
        {
          std::string _typeid_name;
          std::string _data;

          std::exception_ptr to_exception_ptr
            ( aggregated_serialization_functions const&
            , serialization_functions const&
            ) const;
          static boost::optional<aggregated_exception_data>
            from_exception_ptr ( std::exception_ptr
                               , aggregated_serialization_functions const&
                               , serialization_functions const&
                               );

          //! \note Serialization
          aggregated_exception_data() = default;

        private:
          friend class boost::serialization::access;
          template<class Archive>
            void serialize (Archive& ar, const unsigned int)
          {
            ar & _typeid_name;
            ar & _data;
          }
        };

        [[noreturn]] void builtin_exception_data::throw_with_nested() const
        {
          switch (_type)
          {

#define WITH_NOTHING(type)                                                \
          case std_exception_types::type:                                 \
            {                                                             \
              std::throw_with_nested (std::type());                       \
            }

#define WITH_WHAT2(enum_value, type)                                      \
          case std_exception_types::enum_value:                           \
            {                                                             \
              std::throw_with_nested                                      \
                (std::type (boost::get<std::string const&> (_data)));     \
            }

#define WITH_WHAT(type) WITH_WHAT2 (type, type)

#define WITH_CODE(type)                                                   \
          case std_exception_types::type:                                 \
            {                                                             \
              std::throw_with_nested                                      \
                (std::type (boost::get<std::error_code const&> (_data))); \
            }

#define WITH_REGEX_CODE(type)                                             \
          case std_exception_types::type:                                 \
            {                                                             \
              std::throw_with_nested                                      \
                ( std::type                                               \
                  (boost::get<std::regex_constants::error_type> (_data))  \
                );                                                        \
            }

#define WITH_CODE_AND_MAYBE_WHAT2(enum_value, type)                       \
          case std_exception_types::enum_value:                           \
            {                                                             \
              system_error_data_type const& data                          \
                (boost::get<system_error_data_type const&> (_data));      \
              if (std::get<0> (data))                                     \
              {                                                           \
                std::throw_with_nested                                    \
                  (std::type (std::get<2> (data), std::get<1> (data)));   \
              }                                                           \
              else                                                        \
              {                                                           \
                std::throw_with_nested (std::type (std::get<2> (data)));  \
              }                                                           \
            }

#define WITH_CODE_AND_MAYBE_WHAT(type) WITH_CODE_AND_MAYBE_WHAT2 (type, type)

          WITH_NOTHING (bad_alloc)
          WITH_NOTHING (bad_cast)
          WITH_NOTHING (bad_exception)
          WITH_NOTHING (bad_function_call)
          WITH_NOTHING (bad_typeid)
          WITH_NOTHING (bad_weak_ptr)

          WITH_WHAT (logic_error)
          WITH_WHAT (domain_error)

          WITH_CODE (future_error)

          WITH_WHAT (invalid_argument)
          WITH_WHAT (length_error)
          WITH_WHAT (out_of_range)
          WITH_WHAT (runtime_error)
          WITH_WHAT (overflow_error)
          WITH_WHAT (range_error)

          //! \todo bug in libstdcxx of gcc 4.8: undefined reference to
          //! `std::regex_error::regex_error(std::regex_constants::error_type)'

          // case std_exception_types::regex_error:
          //   {
          //     std::throw_with_nested
          //       ( std::regex_error
          //         (boost::get<std::regex_constants::error_type> (_data))
          //       );
          //   }

          WITH_CODE_AND_MAYBE_WHAT (system_error)

          //! \todo Bug in stdlibcxx of gcc 4.8:
          //! std::ios_base::failure does not inherit from std::system_error
          WITH_WHAT2 (ios_base__failure, ios_base::failure)
          // WITH_CODE_AND_MAYBE_WHAT (ios_base__failure, ios_base::failure)

          WITH_WHAT (underflow_error)

#undef WITH_CODE_AND_MAYBE_WHAT
#undef WITH_CODE_AND_MAYBE_WHAT2
#undef WITH_REGEX_CODE
#undef WITH_CODE
#undef WITH_WHAT
#undef WITH_WHAT2
#undef WITH_NOTHING

          }

          UNREACHABLE();
        }

        std::exception_ptr builtin_exception_data::to_exception_ptr() const
        {
          switch (_type)
          {

#define WITH_NOTHING(type)                                                \
          case std_exception_types::type:                                 \
            {                                                             \
              return std::make_exception_ptr (std::type());               \
            }

#define WITH_WHAT2(enum_value, type)                                      \
          case std_exception_types::enum_value:                           \
            {                                                             \
              return std::make_exception_ptr                              \
                (std::type (boost::get<std::string const&> (_data)));     \
            }

#define WITH_WHAT(type) WITH_WHAT2 (type, type)

#define WITH_CODE(type)                                                   \
          case std_exception_types::type:                                 \
            {                                                             \
              return std::make_exception_ptr                              \
                (std::type (boost::get<std::error_code const&> (_data))); \
            }

#define WITH_REGEX_CODE(type)                                             \
          case std_exception_types::type:                                 \
            {                                                             \
              return std::make_exception_ptr                              \
                ( std::type                                               \
                  (boost::get<std::regex_constants::error_type> (_data))  \
                );                                                        \
            }

#define WITH_CODE_AND_MAYBE_WHAT2(enum_value, type)                       \
          case std_exception_types::enum_value:                           \
            {                                                             \
              system_error_data_type const& data                          \
                (boost::get<system_error_data_type const&> (_data));      \
              if (std::get<0> (data))                                     \
              {                                                           \
                return std::make_exception_ptr                            \
                  (std::type (std::get<2> (data), std::get<1> (data)));   \
              }                                                           \
              else                                                        \
              {                                                           \
                return std::make_exception_ptr                            \
                  (std::type (std::get<2> (data)));                       \
              }                                                           \
            }

#define WITH_CODE_AND_MAYBE_WHAT(type) WITH_CODE_AND_MAYBE_WHAT2 (type, type)

          WITH_NOTHING (bad_alloc)
          WITH_NOTHING (bad_cast)
          WITH_NOTHING (bad_exception)
          WITH_NOTHING (bad_function_call)
          WITH_NOTHING (bad_typeid)
          WITH_NOTHING (bad_weak_ptr)

          WITH_WHAT (logic_error)
          WITH_WHAT (domain_error)

          WITH_CODE (future_error)

          WITH_WHAT (invalid_argument)
          WITH_WHAT (length_error)
          WITH_WHAT (out_of_range)
          WITH_WHAT (runtime_error)
          WITH_WHAT (overflow_error)
          WITH_WHAT (range_error)

          //! \todo bug in libstdcxx of gcc 4.8: undefined reference to
          //! `std::regex_error::regex_error(std::regex_constants::error_type)'
          // WITH_REGEX_CODE (regex_error)

          WITH_CODE_AND_MAYBE_WHAT (system_error)

          //! \todo Bug in stdlibcxx of gcc 4.8:
          //! std::ios_base::failure does not inherit from std::system_error
          WITH_WHAT2 (ios_base__failure, ios_base::failure)
          // WITH_CODE_AND_MAYBE_WHAT (ios_base__failure, ios_base::failure)

          WITH_WHAT (underflow_error)

#undef WITH_CODE_AND_MAYBE_WHAT
#undef WITH_CODE_AND_MAYBE_WHAT2
#undef WITH_REGEX_CODE
#undef WITH_CODE
#undef WITH_WHAT
#undef WITH_WHAT2
#undef WITH_NOTHING

          }

          UNREACHABLE();
        }

        boost::optional<builtin_exception_data>
          builtin_exception_data::from_exception_ptr (std::exception_ptr ex_ptr)
        {
          try
          {
            std::rethrow_exception (ex_ptr);
          }

          //! \note depth-first to not catch more generic exceptions by accident
          //! \note std::exception itself can't be handled: pure abstract

#define WITH_NOTHING(type)                                                \
          catch (std::type const&)                                        \
          {                                                               \
            return builtin_exception_data                                 \
              (std_exception_types::type, {});                            \
          }

#define WITH_WHAT2(enum_value, type)                                      \
          catch (std::type const& ex)                                     \
          {                                                               \
            return builtin_exception_data                                 \
              (std_exception_types::enum_value, ex.what());               \
          }

#define WITH_WHAT(type) WITH_WHAT2 (type, type)

#define WITH_CODE(type)                                                   \
          catch (std::type const& ex)                                     \
          {                                                               \
            return builtin_exception_data                                 \
              (std_exception_types::type, ex.code());                     \
          }

#define WITH_REGEX_CODE(type) WITH_CODE(type)

          //! \todo what part of what() to transmit if it is non-default?
#define WITH_CODE_AND_MAYBE_WHAT2(enum_value, type)                       \
          catch (std::type const& ex)                                     \
          {                                                               \
            return builtin_exception_data                                 \
              ( std_exception_types::enum_value                           \
              , std::make_tuple                                           \
                ( std::type (ex.code()).what() != std::string (ex.what()) \
                , std::string (ex.what())                                 \
                , ex.code()                                               \
                )                                                         \
              );                                                          \
          }

#define WITH_CODE_AND_MAYBE_WHAT(type) WITH_CODE_AND_MAYBE_WHAT2 (type, type)

          WITH_WHAT (underflow_error)

          //! \todo Bug in stdlibcxx of gcc 4.8:
          //! std::ios_base::failure does not inherit from std::system_error
          WITH_WHAT2 (ios_base__failure, ios_base::failure)
          // WITH_CODE_AND_MAYBE_WHAT (ios_base__failure, ios_base::failure)

          WITH_CODE_AND_MAYBE_WHAT (system_error)

          //! \todo bug in libstdcxx of gcc 4.8: undefined reference to
          //! `std::regex_error::regex_error(std::regex_constants::error_type)'
          // WITH_REGEX_CODE (regex_error)

          WITH_WHAT (range_error)
          WITH_WHAT (overflow_error)
          WITH_WHAT (runtime_error)
          WITH_WHAT (out_of_range)
          WITH_WHAT (length_error)
          WITH_WHAT (invalid_argument)

          WITH_CODE (future_error)

          WITH_WHAT (domain_error)
          WITH_WHAT (logic_error)

          WITH_NOTHING (bad_weak_ptr)
          WITH_NOTHING (bad_typeid)
          WITH_NOTHING (bad_function_call)
          WITH_NOTHING (bad_exception)
          WITH_NOTHING (bad_cast)
          WITH_NOTHING (bad_alloc)

#undef WITH_CODE_AND_MAYBE_WHAT
#undef WITH_CODE_AND_MAYBE_WHAT2
#undef WITH_REGEX_CODE
#undef WITH_CODE
#undef WITH_WHAT
#undef WITH_WHAT2
#undef WITH_NOTHING

          catch (...)
          {}

          return boost::none;
        }

        [[noreturn]] void user_defined_exception_data::throw_with_nested
          (serialization_functions const& functions) const
        {
          serialization_functions::const_iterator const it
            (functions.find (_function));
          if (it != functions.end())
          {
            it->second.throw_with_nested (_blob);
            UNREACHABLE();
          }
          else if (_fallback)
          {
            _fallback->throw_with_nested();
            UNREACHABLE();
          }
          else
          {
            //! \note Neither deserialization available nor fallback given.
            std::terminate();
          }
        }
        std::exception_ptr user_defined_exception_data::to_exception_ptr
          (serialization_functions const& functions) const
        {
          serialization_functions::const_iterator const it
            (functions.find (_function));
          if (it != functions.end())
          {
            return it->second.to_ptr (_blob);
          }
          else if (_fallback)
          {
            return _fallback->to_exception_ptr();
          }
          else
          {
            //! \note Neither deserialization available nor fallback given.
            std::terminate();
          }
        }

        boost::optional<user_defined_exception_data>
          user_defined_exception_data::from_exception_ptr
            ( std::exception_ptr exception
            , serialization_functions const& funs
            , boost::optional<builtin_exception_data> fallback
            )
        {
          for (serialization_functions::value_type const& fun : funs)
          {
            boost::optional<std::string> exception_data
              (fun.second.from_ptr (exception));

            if (exception_data)
            {
              return user_defined_exception_data
                {fun.first, std::move (*exception_data), std::move (fallback)};
            }
          }

          return boost::none;
        }

        std::exception_ptr aggregated_exception_data::to_exception_ptr
          ( aggregated_serialization_functions const& aggregated_functions
          , serialization_functions const& functions
          ) const
        {
          aggregated_serialization_functions::const_iterator const it
            (aggregated_functions.find (_typeid_name));
          if (it != aggregated_functions.end())
          {
            return it->second.deserialize (_data, functions, aggregated_functions);
          }
          else
          {
            //! \note No deserialization available.
            std::terminate();
          }
        }
        boost::optional<aggregated_exception_data>
          aggregated_exception_data::from_exception_ptr
          ( std::exception_ptr exception
          , aggregated_serialization_functions const& aggregated_functions
          , serialization_functions const& functions
          )
        {
          for ( aggregated_serialization_functions::value_type const& fun
              : aggregated_functions
              )
          {
            boost::optional<std::string> exception_data
              (fun.second.serialize (exception, functions, aggregated_functions));

            if (exception_data)
            {
              return aggregated_exception_data
                {fun.first, std::move (*exception_data)};
            }
          }

          return boost::none;
        }

        exception_data serialize_exception_not_checking_for_nested
          ( std::exception_ptr exception
          , serialization_functions from_exception_ptr_functions
          , aggregated_serialization_functions const& aggregated_functions
          )
        {
          //! \note Always get builtin version as fallback if
          //! equivalent deserialization function is not registered.

          boost::optional<builtin_exception_data> serialized_builtin
            (builtin_exception_data::from_exception_ptr (exception));

          boost::optional<aggregated_exception_data> serialized_aggregated
            ( aggregated_exception_data::from_exception_ptr
              (exception, aggregated_functions, from_exception_ptr_functions)
            );

          boost::optional<user_defined_exception_data> serialized_user_defined
            ( user_defined_exception_data::from_exception_ptr
              (exception, from_exception_ptr_functions, serialized_builtin)
            );

          if (serialized_user_defined)
          {
            return std::move (*serialized_user_defined);
          }
          else if (serialized_aggregated)
          {
            return std::move (*serialized_aggregated);
          }
          else if (serialized_builtin)
          {
            return std::move (*serialized_builtin);
          }

          //! \note No handler registered and not inheriting from
          //! std::exception classes for implicit downcast.
          std::terminate();
        }

        exception_data serialize_exception_impl
          ( std::exception_ptr exception
          , serialization_functions const& from_exception_ptr_functions
          , aggregated_serialization_functions const& aggregated_functions
          )
        {
          try
          {
            std::rethrow_exception (exception);
          }
          catch (std::nested_exception const& nested_exception)
          {
            return nested_exception_data
              { serialize_exception_not_checking_for_nested
                (exception, from_exception_ptr_functions, aggregated_functions)
              , serialize_exception_impl ( nested_exception.nested_ptr()
                                         , from_exception_ptr_functions
                                         , aggregated_functions
                                         )
              };
          }
          catch (...)
          {
            //! \note ignore: only handle nested exceptions here
          }

          return serialize_exception_not_checking_for_nested
            (exception, from_exception_ptr_functions, aggregated_functions);
        }

        std::exception_ptr deserialize_exception_impl
          ( exception_data data
          , serialization_functions const& functions
          , aggregated_serialization_functions const& aggregated_functions
          )
        {
          struct data_visitor : public boost::static_visitor<std::exception_ptr>
          {
            std::exception_ptr operator() (builtin_exception_data data) const
            {
              return data.to_exception_ptr();
            }
            std::exception_ptr operator() (nested_exception_data data) const
            {
              try
              {
                try
                {
                  std::rethrow_exception
                    ( deserialize_exception_impl
                      (data._nested, _functions, _aggregated_functions)
                    );
                }
                catch (...)
                {
                  struct nested_visitor : public boost::static_visitor<void>
                  {
                    void operator() (builtin_exception_data data) const
                    {
                      data.throw_with_nested();
                    }
                    void operator() (nested_exception_data) const
                    {
                      //! \note nested can never be nested with a nested
                      //! function (at least not when a programmer is sane)
                      UNREACHABLE();
                    }
                    void operator() (user_defined_exception_data data) const
                    {
                      data.throw_with_nested (_functions);
                    }
                    void operator() (aggregated_exception_data) const
                    {
                      //! \note We will never std::throw_with_nested():
                      //! Nesting an exception within [an exception
                      //! with a set of exceptions] just does not make
                      //! any sense.
                      UNREACHABLE();
                    }

                    serialization_functions const& _functions;
                    nested_visitor (serialization_functions const& funs)
                      : _functions (funs)
                    {}
                  } const visitor {_functions};

                  boost::apply_visitor (visitor, data._this);

                  UNREACHABLE();
                }
              }
              catch (...)
              {
                return std::current_exception();
              }
            }
            std::exception_ptr operator() (user_defined_exception_data data) const
            {
              return data.to_exception_ptr (_functions);
            }
            std::exception_ptr operator() (aggregated_exception_data data) const
            {
              return data.to_exception_ptr (_aggregated_functions, _functions);
            }

            serialization_functions const& _functions;
            aggregated_serialization_functions const& _aggregated_functions;
            data_visitor ( serialization_functions const& funs
                         , aggregated_serialization_functions const& aggfuns
                         )
              : _functions (funs)
              , _aggregated_functions (aggfuns)
            {}
          } const visitor {functions, aggregated_functions};

          return boost::apply_visitor (visitor, data);
        }
      }

      std::string serialize
        ( std::exception_ptr exception
        , serialization_functions const& functions
        , aggregated_serialization_functions const& aggregated_functions
        )
      {
        std::ostringstream os;
        boost::archive::text_oarchive oa (os);

        exception_data const data
          (serialize_exception_impl (exception, functions, aggregated_functions));
        oa & data;

        return os.str();
      }

      std::exception_ptr deserialize
        ( std::string blob
        , serialization_functions const& functions
        , aggregated_serialization_functions const& aggregated_functions
        )
      {
        std::istringstream is (blob);
        boost::archive::text_iarchive ia (is);

        exception_data data;
        ia & data;

        return deserialize_exception_impl (data, functions, aggregated_functions);
      }
    }
  }
}
