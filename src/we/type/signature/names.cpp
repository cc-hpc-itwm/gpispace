// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <we/type/signature/names.hpp>
#include <we/type/signature/traverse.hpp>

namespace pnet
{
  namespace type
  {
    namespace signature
    {
      namespace
      {
        class get_names_rec
        {
        public:
          get_names_rec (std::unordered_set<std::string>& names)
            : _names (names)
          {}
          void _struct (std::pair<std::string, structure_type> const& s) const
          {
            for (field_type const& f : s.second)
            {
              traverse (*this, f);
            }
          }
          void _field (std::pair<std::string, std::string> const& f) const
          {
            _names.insert (f.second);
          }
          void _field_struct
            (std::pair<std::string, structure_type> const& s) const
          {
            traverse (*this, s);
          }

        private:
          std::unordered_set<std::string>& _names;
        };

        class get_names : public ::boost::static_visitor<>
        {
        public:
          get_names (std::unordered_set<std::string>& names)
            : _names (names)
          {}
          void operator() (std::string const& tname) const
          {
            _names.insert (tname);
          }
          void operator() (structured_type const& s) const
          {
            traverse (get_names_rec (_names), s);
          }

        private:
          std::unordered_set<std::string>& _names;
        };
      }

      std::unordered_set<std::string> names (signature_type const& signature)
      {
        std::unordered_set<std::string> names;

        ::boost::apply_visitor (get_names (names), signature);

        return names;
      }
    }
  }
}
