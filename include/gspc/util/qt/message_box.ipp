#include <gspc/util/mp/exactly_one_is.hpp>
#include <gspc/util/mp/none_is.hpp>
#include <gspc/util/unreachable.hpp>

#include <functional>
#include <type_traits>

using gspc::util::mp::exactly_one_is;
using gspc::util::mp::none_is;

    namespace gspc::util::qt
    {
      namespace detail
      {
        template<QMessageBox::StandardButton, typename Ret, typename Tag>
          struct action_t : public std::function<Ret()>
        {
          using std::function<Ret()>::function;
        };

        template<typename Ret>
          Ret call_action (int)
        {
          FHG_UTIL_UNREACHABLE
            ("QMessageBox is supposed to only return buttons that we gave it.");
        }
        template < typename Ret
                 , QMessageBox::StandardButton ButtonsHead
                 , typename TagHead
                 , typename... Tail
                 >
          Ret call_action ( int result
                          , action_t<ButtonsHead, Ret, TagHead> head
                          , Tail... tail
                          )
        {
          return result == ButtonsHead
            ? head()
            : call_action<Ret> (result, tail...);
        }

        constexpr QMessageBox::StandardButton default_button()
        {
          return QMessageBox::NoButton;
        }
        //! \note the const* for action_t works around action_t
        //! itselfnot being a trivial type (due to the destructor) and
        //! thus not being usable as a temporary/argument in a
        //! constexpr context.
        template < QMessageBox::StandardButton Button
                 , typename Ret
                 , typename Tag
                 , typename... Tail
                 >
          constexpr QMessageBox::StandardButton default_button
            (action_t<Button, Ret, Tag> const*, Tail... tail)
        {
          return std::is_same<Tag, default_button_tag>{} ? Button
            : default_button (tail...);
        }

        template<typename... Tags>
          constexpr bool zero_or_one_default_buttons()
        {
          return exactly_one_is<default_button_tag, Tags...>{}
            || none_is<default_button_tag, Tags...>{};
        }
      }

      template < typename Ret
               , QMessageBox::StandardButton... Buttons
               , typename... Tags
               >
        Ret message_box ( QMessageBox::Icon icon
                        , QWidget* parent
                        , QString const& title
                        , QString const& text
                        , detail::action_t<Buttons, Ret, Tags>... actions
                        )
      {
        static_assert
          ( detail::zero_or_one_default_buttons<Tags...>()
          , "A message box can have exactly zero or one default button."
          );

        QMessageBox box (icon, title, text, {Buttons...}, parent);
        box.setDefaultButton (detail::default_button (&actions...));

        return call_action (box.exec(), actions...);
      }

      template<QMessageBox::StandardButton Button, typename Fun>
        detail::button_t<Button, return_type<Fun>> button (Fun fun)
      {
        return fun;
      }
      template<QMessageBox::StandardButton Button, typename Fun>
        detail::default_button_t<Button, return_type<Fun>> default_button (Fun fun)
      {
        return fun;
      }
    }
