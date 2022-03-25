// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <util-generic/callable_signature.hpp>
#include <util-qt/forward_decl.hpp>

#include <QtCore/QEvent>
#include <QtCore/QObject>

FHG_UTIL_QT_FORWARD_DECL (class QActionEvent)
FHG_UTIL_QT_FORWARD_DECL (class QChildEvent)
FHG_UTIL_QT_FORWARD_DECL (class QClipboardEvent)
FHG_UTIL_QT_FORWARD_DECL (class QCloseEvent)
FHG_UTIL_QT_FORWARD_DECL (class QContextMenuEvent)
FHG_UTIL_QT_FORWARD_DECL (class QDragEnterEvent)
FHG_UTIL_QT_FORWARD_DECL (class QDragLeaveEvent)
FHG_UTIL_QT_FORWARD_DECL (class QDragMoveEvent)
FHG_UTIL_QT_FORWARD_DECL (class QDropEvent)
FHG_UTIL_QT_FORWARD_DECL (class QDynamicPropertyChangeEvent)
FHG_UTIL_QT_FORWARD_DECL (class QFileOpenEvent)
FHG_UTIL_QT_FORWARD_DECL (class QFocusEvent)
FHG_UTIL_QT_FORWARD_DECL (class QGestureEvent)
FHG_UTIL_QT_FORWARD_DECL (class QGraphicsSceneContextMenuEvent)
FHG_UTIL_QT_FORWARD_DECL (class QGraphicsSceneDragDropEvent)
FHG_UTIL_QT_FORWARD_DECL (class QGraphicsSceneHoverEvent)
FHG_UTIL_QT_FORWARD_DECL (class QGraphicsSceneMouseEvent)
FHG_UTIL_QT_FORWARD_DECL (class QGraphicsSceneMoveEvent)
FHG_UTIL_QT_FORWARD_DECL (class QGraphicsSceneResizeEvent)
FHG_UTIL_QT_FORWARD_DECL (class QGraphicsSceneWheelEvent)
FHG_UTIL_QT_FORWARD_DECL (class QHelpEvent)
FHG_UTIL_QT_FORWARD_DECL (class QHideEvent)
FHG_UTIL_QT_FORWARD_DECL (class QHoverEvent)
FHG_UTIL_QT_FORWARD_DECL (class QIconDragEvent)
FHG_UTIL_QT_FORWARD_DECL (class QInputMethodEvent)
FHG_UTIL_QT_FORWARD_DECL (class QKeyEvent)
FHG_UTIL_QT_FORWARD_DECL (class QMouseEvent)
FHG_UTIL_QT_FORWARD_DECL (class QMoveEvent)
FHG_UTIL_QT_FORWARD_DECL (class QPaintEvent)
FHG_UTIL_QT_FORWARD_DECL (class QResizeEvent)
FHG_UTIL_QT_FORWARD_DECL (class QShortcutEvent)
FHG_UTIL_QT_FORWARD_DECL (class QShowEvent)
FHG_UTIL_QT_FORWARD_DECL (class QStatusTipEvent)
FHG_UTIL_QT_FORWARD_DECL (class QTabletEvent)
FHG_UTIL_QT_FORWARD_DECL (class QTimerEvent)
FHG_UTIL_QT_FORWARD_DECL (class QToolBarChangeEvent)
FHG_UTIL_QT_FORWARD_DECL (class QTouchEvent)
FHG_UTIL_QT_FORWARD_DECL (class QUpdateLaterEvent)
FHG_UTIL_QT_FORWARD_DECL (class QWhatsThisClickedEvent)
FHG_UTIL_QT_FORWARD_DECL (class QWheelEvent)
FHG_UTIL_QT_FORWARD_DECL (class QWindowStateChangeEvent)

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace
      {
        template<QEvent::Type> struct event_type_for_enum_value;

#define MAKE_EVENT_TYPE_FOR_ENUM_VALUE(EnumValue, EventType)      \
        template<> struct event_type_for_enum_value<EnumValue>    \
        {                                                         \
          using type = EventType;                                 \
        }

        //! \todo Qt5: Regenerate list and forward decls above
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::ActionAdded, QActionEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::ActionChanged, QActionEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::ActionRemoved, QActionEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::ActivationChange, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::ApplicationActivate, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::ApplicationDeactivate, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::ApplicationFontChange, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::ApplicationLayoutDirectionChange, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::ApplicationPaletteChange, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::ApplicationWindowIconChange, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::ChildAdded, QChildEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::ChildPolished, QChildEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::ChildRemoved, QChildEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::Clipboard, QClipboardEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::Close, QCloseEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::ContentsRectChange, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::ContextMenu, QContextMenuEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::CursorChange, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::DeferredDelete, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::DragEnter, QDragEnterEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::DragLeave, QDragLeaveEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::DragMove, QDragMoveEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::Drop, QDropEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::DynamicPropertyChange, QDynamicPropertyChangeEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::EnabledChange, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::Enter, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::EnterWhatsThisMode, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::FileOpen, QFileOpenEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::FocusIn, QFocusEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::FocusOut, QFocusEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::FontChange, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::Gesture, QGestureEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::GestureOverride, QGestureEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::GrabKeyboard, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::GrabMouse, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::GraphicsSceneContextMenu, QGraphicsSceneContextMenuEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::GraphicsSceneDragEnter, QGraphicsSceneDragDropEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::GraphicsSceneDragLeave, QGraphicsSceneDragDropEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::GraphicsSceneDragMove, QGraphicsSceneDragDropEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::GraphicsSceneDrop, QGraphicsSceneDragDropEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::GraphicsSceneHelp, QHelpEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::GraphicsSceneHoverEnter, QGraphicsSceneHoverEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::GraphicsSceneHoverLeave, QGraphicsSceneHoverEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::GraphicsSceneHoverMove, QGraphicsSceneHoverEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::GraphicsSceneMouseDoubleClick, QGraphicsSceneMouseEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::GraphicsSceneMouseMove, QGraphicsSceneMouseEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::GraphicsSceneMousePress, QGraphicsSceneMouseEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::GraphicsSceneMouseRelease, QGraphicsSceneMouseEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::GraphicsSceneMove, QGraphicsSceneMoveEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::GraphicsSceneResize, QGraphicsSceneResizeEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::GraphicsSceneWheel, QGraphicsSceneWheelEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::Hide, QHideEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::HideToParent, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::HoverEnter, QHoverEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::HoverLeave, QHoverEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::HoverMove, QHoverEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::IconDrag, QIconDragEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::IconTextChange, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::InputMethod, QInputMethodEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::KeyPress, QKeyEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::KeyRelease, QKeyEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::KeyboardLayoutChange, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::LanguageChange, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::LayoutDirectionChange, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::LayoutRequest, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::Leave, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::LeaveWhatsThisMode, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::LocaleChange, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::MouseButtonDblClick, QMouseEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::MouseButtonPress, QMouseEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::MouseButtonRelease, QMouseEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::MouseMove, QMouseEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::MouseTrackingChange, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::Move, QMoveEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::OkRequest, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::Paint, QPaintEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::PaletteChange, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::ParentAboutToChange, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::ParentChange, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::Polish, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::PolishRequest, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::QueryWhatsThis, QHelpEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::Resize, QResizeEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::Shortcut, QShortcutEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::ShortcutOverride, QKeyEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::Show, QShowEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::ShowToParent, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::SockAct, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::StatusTip, QStatusTipEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::StyleChange, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::TabletEnterProximity, QTabletEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::TabletLeaveProximity, QTabletEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::TabletMove, QTabletEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::TabletPress, QTabletEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::TabletRelease, QTabletEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::Timer, QTimerEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::ToolBarChange, QToolBarChangeEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::ToolTip, QHelpEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::ToolTipChange, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::TouchBegin, QTouchEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::TouchEnd, QTouchEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::TouchUpdate, QTouchEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::UngrabKeyboard, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::UngrabMouse, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::UpdateLater, QUpdateLaterEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::UpdateRequest, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::WhatsThis, QHelpEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::WhatsThisClicked, QWhatsThisClickedEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::Wheel, QWheelEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::WindowActivate, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::WindowBlocked, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::WindowDeactivate, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::WindowIconChange, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::WindowStateChange, QWindowStateChangeEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::WindowTitleChange, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::WindowUnblocked, QEvent);
        MAKE_EVENT_TYPE_FOR_ENUM_VALUE (QEvent::ZOrderChange, QEvent);

#undef MAKE_EVENT_TYPE_FOR_ENUM_VALUE

        template<QEvent::Type EnumValue>
          struct event_filter : public QObject
        {
          using event_type = typename event_type_for_enum_value<EnumValue>::type;
          event_filter (std::function<bool (event_type*)> filter, QObject* parent)
            : QObject (parent)
            , _filter (filter)
          {}
          virtual bool eventFilter (QObject*, QEvent* event) override
          {
            if (event->type() != EnumValue)
            {
              return false;
            }

            return _filter (static_cast<event_type*> (event));
          }

          std::function<bool (event_type*)> _filter;
        };

        template<typename Expected, typename Got>
          struct expect_same
        {
          static_assert (std::is_same<Expected, Got>::value, "type mismatch");
        };
      }

      template<QEvent::Type EnumValue, typename Filter>
        void add_event_filter (QObject* object, Filter&& filter)
      {
        expect_same < bool (typename event_type_for_enum_value<EnumValue>::type*)
                    , callable_signature<Filter>
                    >();
        object->installEventFilter (new event_filter<EnumValue> (filter, object));
      }

      template < QEvent::Type EnumValue0, QEvent::Type... EnumValues
               , typename Filter0, typename... Filters
               >
        void add_event_filter (QObject* object, Filter0&& filter, Filters&&... filters)
      {
        add_event_filter<EnumValue0> (object, std::forward<Filter0> (filter));
        add_event_filter<EnumValues...> (object, std::forward<Filters> (filters)...);
      }
    }
  }
}
