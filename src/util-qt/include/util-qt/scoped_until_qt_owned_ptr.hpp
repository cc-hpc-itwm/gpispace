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

#include <QtCore/QPointer>

#include <memory>
#include <type_traits>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      template<typename T>
        class scoped_until_qt_owned_ptr
      {
      public:
        template<typename... Args>
          scoped_until_qt_owned_ptr (Args&&... args)
            : _scoped (std::make_unique<T> (std::forward<Args> (args)...))
            , _qt_owned (nullptr)
        {}

        template<typename U>
          scoped_until_qt_owned_ptr (std::unique_ptr<U> scoped)
            : _scoped (std::move (scoped))
            , _qt_owned (nullptr)
        {}

        template<typename U>
          scoped_until_qt_owned_ptr (scoped_until_qt_owned_ptr<U>&& other)
            : _scoped (std::move (other._scoped))
            , _qt_owned (other._qt_owned)
        {
          other._qt_owned = nullptr;
        }

        T* release()
        {
          return _qt_owned = _scoped.release();
        }

        T* get()
        {
          return _scoped ? _scoped.get() : static_cast<T*> (_qt_owned);
        }
        T& operator*() { return get(); }
        T* operator->() { return get(); }
        T const* get() const
        {
          return _scoped ? _scoped.get() : static_cast<T const*> (_qt_owned);
        }
        T const& operator*() const { return get(); }
        T const* operator->() const { return get(); }

      private:
        template<typename> friend class scoped_until_qt_owned_ptr;

        std::unique_ptr<T> _scoped;
        //! \note Use QPointer to detect if Qt deleted the object in
        //! between if possible
        typename std::conditional < std::is_base_of<QObject, T>::value
                                  , QPointer<T>
                                  , T*
                                  >::type _qt_owned;
      };
    }
  }
}
