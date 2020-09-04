// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <boost/utility.hpp>

#include <condition_variable>
#include <list>
#include <mutex>

namespace fhg
{
  namespace thread
  {
    template<typename T>
      class queue : public boost::noncopyable
    {
    public:
      typedef std::list<T> container_type;
      typedef typename container_type::size_type size_type;

      T get()
      {
        std::unique_lock<std::mutex> lock (m_mtx);
        m_get_cond.wait (lock, [this] { return !m_container.empty(); });

        T t (std::move (m_container.front()));
        m_container.pop_front();
        return t;
      }

      template<class... Args> void put (Args&&... args)
      {
        std::unique_lock<std::mutex> const _ (m_mtx);
        m_container.emplace_back (std::forward<Args> (args)...);
        m_get_cond.notify_one();
      }

      template<typename FwdIt> void put_many (FwdIt begin, FwdIt end)
      {
        std::unique_lock<std::mutex> const _ (m_mtx);
        m_container.insert (m_container.end(), begin, end);
        m_get_cond.notify_all();
      }

      void INDICATES_A_RACE_clear()
      {
        std::unique_lock<std::mutex> const _ (m_mtx);
        m_container.clear();
      }
    private:
      mutable std::mutex m_mtx;
      std::condition_variable m_get_cond;

      container_type m_container;
    };
  }
}
