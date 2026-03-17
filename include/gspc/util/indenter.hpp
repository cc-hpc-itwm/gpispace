// Copyright (C) 2013,2015,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/util/ostream/modifier.hpp>

#include <iosfwd>


  namespace gspc::util
  {
    class indenter : public gspc::util::ostream::modifier
    {
    public:
      indenter (unsigned int = 0);
      indenter& operator++();
      indenter operator++ (int);
      indenter& operator--();
      indenter operator-- (int);
      std::ostream& operator() (std::ostream&) const override;

    private:
      unsigned int _depth;
    };

    class deeper : public gspc::util::ostream::modifier
    {
    public:
      deeper (indenter&);
      std::ostream& operator() (std::ostream&) const override;
    private:
      indenter& _indenter;
    };
  }
