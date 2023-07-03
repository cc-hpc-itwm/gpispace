// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/syscall.hpp>

#include <QtCore/QModelIndex>

#include <boost/serialization/split_free.hpp>

//! \note This serializes a pointer to the NOT COPIED model, thus
//! shall only be used within the same process (or the pointer will
//! point to the abyss and will probably crash soon after). To try
//! helping to avoid this issue, there is also the current PID
//! serialized and checked. (Yes, terribly bad luck will break it even
//! with this check.)

namespace boost
{
  namespace serialization
  {
    template<typename Archive>
      void load (Archive& ar, QModelIndex& index, unsigned int const)
    {
      pid_t expected_pid (fhg::util::syscall::getpid());
      pid_t pid;
      ar & pid;
      if (pid != expected_pid)
      {
        throw std::logic_error
          ("tried loading QModelIndex serialized in other process");
      }

      int row;
      int column;
      std::uintptr_t model_raw;
      QAbstractItemModel const* model;
      bool has_parent;
      QModelIndex parent;

      ar & row;
      ar & column;
      ar & model_raw;
      model = reinterpret_cast<QAbstractItemModel const*> (model_raw);
      ar & has_parent;
      if (has_parent)
      {
        ar & parent;
      }

      index = model->index (row, column, parent);
    }

    template<typename Archive>
      void save (Archive& ar, QModelIndex const& index, unsigned int const)
    {
      pid_t pid (fhg::util::syscall::getpid());
      ar & pid;

      int row (index.row());
      int column (index.column());
      QAbstractItemModel const* model (index.model());
      bool has_parent (index.parent() != QModelIndex());
      QModelIndex parent (index.parent());

      ar & row;
      ar & column;
      std::uintptr_t model_raw (reinterpret_cast<std::uintptr_t> (model));
      ar & model_raw;
      ar & has_parent;
      if (has_parent)
      {
        ar & parent;
      }
    }

    template<class Archive>
      void serialize (Archive& ar, QModelIndex& index, unsigned int const version)
    {
      ::boost::serialization::split_free (ar, index, version);
    }
  }
}
