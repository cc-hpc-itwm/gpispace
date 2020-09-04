# Fundamentals

## scoped_until_qt_owned_ptr

Scoped resource management helper for heap allocated but not yet Qt
owned objects with explicit release semantics for safety and
clarity. Automatically constructs object on the heap in the ctor.

```c++
scoped_until_qt_owned_ptr<QHBoxLayout> layout; // 1, 6
scoped_until_qt_owned_ptr<QLabel> label;       // 2, 5
layout->addWidget (label.release());           // 3
label->connect (&QLabel::somesignal, ...);     // 4
throw 0;
```

1. heap allocate `QHBoxLayout* layout` with no parent
2. heap allocate `QLabel* label` with no parent
3. move ownership of `*label` to `*layout` making the dtor of `label`
   a no-operation
4. connect a signal of `*label` using the now non-owning pointer
5. **not** free `*label`
6. free `*layout`, which then using the Qt ownership model frees
   `*label` since it is a child object

Passing ownership to Qt without releasing is not detected. This also
applies to passing a `parent` pointer in construction,
e.g. `scoped_until_qt_owned_ptr<QLabel> label (layout.get())`.

## overload

`qOverload` (requires C++14) and `QOverload` for pre Qt 5.7: Select
overloaded function by argument type, without ugly manual casting.

```c++
QOverload<int>::of (&QComboBox::currentIndexChanged)
qOverload<int> (&QComboBox::currentIndexChanged)
```
is equivalent to
```c++
static_cast<void (QComboBox::*) (int)>
  (&QComboBox::currentIndexChanged)>
```

## scoped_signal_block

RAII wrapper for `QObject::blockSignals()`: Block signals of a given
object until end of scope. Correctly handles nested blocking.

## variant

Helpers for sane usage of `QVariant` where not avoidable.

### stores

Check if the given variant contains the `T`, without conversion.

### value

Get the value of type `T` from the variant, or throw if a different
type is contained.

### optional

Get the value of type `T` from the variant, or `boost::none` if a
different type is contained.

### collect

Get a list of all values of type `T` in the variant, recursing into
stored `QVariantList`s. Non-matching types are ignored.

## forward_decl

Forward declare a Qt symbol correctly (read: in the correct
namespace), avoiding includes. Since it can forward-declare types and
functions, the full declaration is required. Has to be used in the top
level namespace.

```c++
FHG_UTIL_QT_FORWARD_DECL (class QObject)
FHG_UTIL_QT_FORWARD_DECL (bool qFuzzyIsNull (float))
```

## do_after_event_loop

Call a function a single time, after event processing for this event
loop round finished.

# Testing

## Application, CoreApplication

A QApplication/QCoreApplication that can be used in a `$DISPLAY`-less
unit test environment. Shall be inside a test body.

# Widgets

## message_box

Opens a `QMessageBox` with the given icon, parent, title and text as
well as the actions listed and executes the user selected action.

Buttons and their connected actions, are created using `button()` and
`default_button()` wrapper functions, to associate a
`QMessageBox::StandardButton` with them and to ensure there is either
one or no default button. Actions may have return values, but the
return value must be consistent for all actions.

```c++
bool const result
  ( message_box
      ( QMessageBox::Question
      , nullptr
      , "question"
      , "yes or no?"
      , button<QMessageBox::Yes> ([] { return true; })
      , default_button<QMessageBox::No> ([] { return false; })
      )
  );
```

## style

Helpers for saner interaction with QSS stylesheets, mostly for
emulating CSS style classes using properties.

### set_css_class_with_value, remove_css_class, set_css_class_enabled

Set or unset a dynamic property `style-$class` on the given
`widget`. Then ensure the widget is repolished. The `_enabled`
overload shall be used for class-existence based styling only. The
`_with_value` overload can be used to dispatch based on the value
additionally.

Given

```c++
set_css_class_with_value (label1, "valued", "foo");
set_css_class_with_value (label2, "valued", "bar");
set_css_class_enabled (widget, "is_special");
set_css_class_enabled (label2, "is_special");
```

and

```
* { color: yellow; }
*[style-is_special] { border: 1px solid red; }
QLabel[style-valued=foo] { color: green; }
QLabel[style-valued=bar] { color: blue; }
```

- `label1` is colored green
- `label2` is colored blue with a red border
- `widget` is colored yellow with a red border

### repolish

Ensure that a `QWidget` is polished with the current style, after
changing the style in a way that Qt does not detect, e.g. setting
properties.

## scoped_cursor_override

RAII wrapper for `QApplication`'s override cursor. Overrides the
currently used cursor. Correctly handles nested overriding.

## widget/file_line_edit

A `QLineEdit` with an accompanying button to open a `QFileDialog`. No
verification of the entered data happens other than that done by
`QFileDialog`, but due to the nature of the control, that may never
even be happening. This means that

```c++
file_line_edit edit (QFileDialog::ExistingFile);
// ...
assert (boost::filesystem::exists (edit.value()));
```

may very well fail the assertion.

## widget/mini_button

A `QToolButton` with a given `QAction` triggered on press. The
`action` also defines the icon, text, tooltip etc. A convenience ctor
is provided to only specify the icon. The action can be queried with
`defaultAction()`.

```c++
mini_button button (QStyle::SP_LineEditClearButton);
connect ( button->defaultAction(), &QAction::triggered
        , lineedit, &QLineEdit::clear
        );
```

# Events

## event_filter

Easily install an event handler functions on an object for given
QEvent types, without deriving and overloading the filtered object's
type.

```c++
add_event_filter < QEvent::WhatsThisClicked
                 , QEvent::Close
                 >
  ( object
  , [] (QWhatsThisClickedEvent* event)
    {
      event->reject();
    }
  , [object] (QCloseEvent*)
    {
      object->deleteLater();
    }
  );
```

# Drag & Drop

## make_drop_target

Let a widget be a drop target for `QMimeData`. Can either specify
mimetype string or a callback to check if a `QMimeData` will be
accepted.

# QPainter

## painter_state_saver

A scoped helper for `QPainter::save()` and `QPainter::restore()`.

# Serialization

## qdatastream_enum

Include to allow printing any C++ enum (as per `std::is_enum`) to
`QDataStream` streams, using their underyling type.

## qbytearray_archive

A Boost.Serialization archive to and from `QByteArray`, e.g. for use
in `QMimeData`.

```c++
BoostSerializableObject object;

qbytearray_oarchive oa;
oa << object;
QByteArray const ba (oa.get());

qbytearray_iarchive ia (ba);
ia >> object;
```

## serialization/QList, serialization/QVector, serialization/QPointF

Boost.Serialization implementation for QLists, QVector and QPointF.

## serialization/QModelIndex

Boost.Serialization implementation for QModelIndex, which shall only
ever be used within the same process, e.g. when dragging between
multiple views inside the same UI. It contains raw, non-owning
pointers to models, which **will crash** if used across processes.
