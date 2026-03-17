#include <gspc/util/callable_signature.hpp>
#include <gspc/util/cxx17/void_t.hpp>
#include <gspc/util/print_container.hpp>
#include <gspc/util/print_exception.hpp>
#include <gspc/util/unreachable.hpp>

#include <algorithm>
#include <exception>
#include <stdexcept>
#include <utility>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <gspc/util/exception_printer.formatter.hpp>

    namespace gspc::testing
    {
      namespace detail
      {
        template<typename Wrapping, typename Wrapped>
          struct nested_exception
        {
          Wrapping _wrapping;
          Wrapped _wrapped;

          nested_exception (Wrapping wrapping, Wrapped wrapped)
            : _wrapping (std::move (wrapping))
            , _wrapped (std::move (wrapped))
          {}
        };

        template<typename Ex>
          void throw_ (Ex const& ex)
        {
          throw ex;
        }

        template<typename Wrapping, typename Wrapped>
          void throw_ (nested_exception<Wrapping, Wrapped> const& ex)
        {
          try
          {
            throw_ (ex._wrapped);
          }
          catch (...)
          {
            std::throw_with_nested (ex._wrapping);
          }
        }

        template<typename Ex>
          std::string to_string (Ex const& ex)
        {
          try
          {
            throw_ (ex);
          }
          catch (...)
          {
            return gspc::util::current_exception_printer (": ").string();
          }

          FHG_UTIL_UNREACHABLE ("throw_ always throws");
        }

        template<typename Ex, typename = void>
          struct require_equal
        {
          void operator() (Ex const& lhs, Ex const& rhs) const
          {
            if (std::string (lhs.what()) != rhs.what())
            {
              throw std::logic_error
                ( fmt::format
                  ( "'{}' != '{}'"
                  , to_string (lhs)
                  , to_string (rhs)
                  )
                );
            }
          }
        };

        template<typename Ex>
          struct require_equal<Ex, gspc::util::cxx17::void_t<decltype (std::declval<Ex>() == std::declval<Ex>())> >
        {
          void operator() (Ex const& lhs, Ex const& rhs) const
          {
            if (!(lhs == rhs))
            {
              throw std::logic_error
                ( fmt::format
                  ( "'{}' != '{}'"
                  , to_string (lhs)
                  , to_string (rhs)
                  )
                );
            }
          }
        };


        template<typename ToCatch, typename Wrapping, typename Wrapped>
          void require_equal_wrapped_impl
            ( nested_exception<Wrapping, Wrapped> const& lhs
            , Wrapping const& rhs
            )
        {
          require_equal<Wrapping>() (lhs._wrapping, rhs);

          try
          {
            std::rethrow_if_nested (rhs);
          }
          catch (ToCatch const& wrapped_rhs)
          {
            require_equal<Wrapped>() (lhs._wrapped, wrapped_rhs);

            return;
          }
          catch (std::exception const& ex)
          {
            throw std::logic_error
              ( fmt::format
                ( "nested exception of wrong type: got {}, expected {}"
                , typeid (ex).name()
                , typeid (ToCatch).name()
                )
              );
          }
          catch (...)
          {
            throw std::logic_error
              ( fmt::format
                ( "nested exception of wrong type: got unknown, expected {}"
                , typeid (ToCatch).name()
                )
              );
          }

          throw std::logic_error
            ( fmt::format
              ( "missing nested exception of type {}"
              , typeid (ToCatch).name()
              )
            );
        }

        template<typename Wrapping, typename Wrapped>
          struct require_equal<nested_exception<Wrapping, Wrapped>, void>
        {
          void operator() ( nested_exception<Wrapping, Wrapped> const& lhs
                          , Wrapping const& rhs
                          ) const
          {
            require_equal_wrapped_impl<Wrapped, Wrapping, Wrapped>
              (lhs, rhs);
          }
        };

        template<typename Wrapping, typename WrappedWrapping, typename WrappedWrapped>
          struct require_equal<nested_exception<Wrapping, nested_exception<WrappedWrapping, WrappedWrapped>>, void>
        {
          using nested = nested_exception<WrappedWrapping, WrappedWrapped>;
          void operator() ( nested_exception<Wrapping, nested> const& lhs
                          , Wrapping const& rhs
                          ) const
          {
            require_equal_wrapped_impl<WrappedWrapping, Wrapping, nested>
              (lhs, rhs);
          }
        };

        template< typename ToCatch
                , typename Exception
                , typename Fun
                , typename Check
                >
          void require_exception_impl ( Fun&& fun
                                      , Check&& check
                                      , std::string const& expected_string
                                      )
        {
          try
          {
            try
            {
              fun();
            }
            catch (ToCatch const& actual)
            {
              check (actual);

              return;
            }
            catch (std::exception const& ex)
            {
              std::throw_with_nested
                ( std::logic_error ( "got exception of wrong type "
                                   + std::string (typeid (ex).name())
                                   )
                );
            }
            catch (...)
            {
              std::throw_with_nested
                (std::logic_error ("got exception of wrong type"));
            }

            throw std::logic_error ("got no exception at all");
          }
          catch (...)
          {
            throw std::logic_error
              ( fmt::format
                ( "missing exception '{}' of type {}: {}"
                , expected_string
                , typeid (Exception).name()
                , gspc::util::current_exception_printer (": ")
                )
              );
          }
        }
      }

      template<typename Wrapping, typename Wrapped>
        detail::nested_exception<Wrapping, Wrapped> make_nested
          (Wrapping wrapping, Wrapped wrapped)
      {
        return {wrapping, wrapped};
      }


      template<typename ThrowIfMismatch, typename Exception, typename Fun>
        void require_exception ( Fun&& fun
                               , Exception const& expected
                               , ThrowIfMismatch throw_if_mismatch
                               )
      {
        static_assert
          ( gspc::util::is_callable < ThrowIfMismatch
                        , void (Exception const&, Exception const&)
                        >{}
          , "require_exception requires given ThrowIfMismatch to throw, rather "
            "than return a result"
          );

        detail::require_exception_impl<Exception, Exception>
          ( std::forward<Fun> (fun)
          , [&expected, &throw_if_mismatch] (Exception const& catched)
            {
              throw_if_mismatch (expected, catched);
            }
          , detail::to_string (expected)
          );
      }
      template<typename Exception, typename Fun>
        void require_exception ( Fun&& fun
                               , Exception const& expected
                               )
      {
        return require_exception ( std::forward<Fun> (fun)
                                 , expected
                                 , detail::require_equal<Exception>()
                                 );
      }

      template<typename ThrowIfMismatch, typename Wrapping, typename Wrapped, typename Fun>
        void require_exception
          ( Fun&& fun
          , detail::nested_exception<Wrapping, Wrapped> const& expected
          , ThrowIfMismatch throw_if_mismatch
          )
      {
        detail::require_exception_impl<Wrapping, decltype (expected)>
          ( std::forward<Fun> (fun)
          , [&expected, &throw_if_mismatch] (Wrapping const& catched)
            {
              throw_if_mismatch (expected, catched);
            }
          , detail::to_string (expected)
          );
      }
      template<typename Wrapping, typename Wrapped, typename Fun>
        void require_exception
          ( Fun&& fun
          , detail::nested_exception<Wrapping, Wrapped> const& expected
          )
      {
        return require_exception
          ( std::forward<Fun> (fun)
          , expected
          , detail::require_equal<detail::nested_exception<Wrapping, Wrapped>>()
          );
      }

      template<typename Exception, typename Fun>
        void require_exception_with_message_in
          ( Fun&& fun
          , std::list<std::string> const& whats
          )
      {
        std::string const joined_whats
          (gspc::util::print_container ("{'", "', '", "'}", whats).string());

        detail::require_exception_impl<Exception, Exception>
          ( std::forward<Fun> (fun)
          , [&whats, &joined_whats] (Exception const& catched)
            {
              std::string const what (catched.what());

              if (std::find (whats.begin(), whats.end(), what) == whats.end())
              {
                throw std::logic_error
                  ( fmt::format
                    ( "mismatching message: '{}' != one of {}"
                    , what
                    , joined_whats
                    )
                  );
              }
            }
          , joined_whats
          );
      }

      template<typename Exception, typename Fun>
        void require_exception_with_message ( Fun&& fun
                                            , std::string const& what
                                            )
      {
        require_exception_with_message_in<Exception, Fun>
          (std::forward<Fun> (fun), {what});
      }
    }

