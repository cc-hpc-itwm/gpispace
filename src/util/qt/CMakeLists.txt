# This file is part of GPI-Space.
# Copyright (C) 2020 Fraunhofer ITWM
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

extended_add_library (NAME fhg_util_qt
  SOURCES "dual_list_selector.cpp"
          "mvc/alphanum_sort_proxy.cpp"
          "mvc/delegating_header_view.cpp"
          "mvc/filter_ignoring_branch_nodes_proxy.cpp"
          "mvc/fixed_proxy_models.cpp"
          "mvc/flat_to_tree_proxy.cpp"
          "mvc/multiplicated_column_proxy.cpp"
          "mvc/section_index.cpp"
          "no_undoredo_lineedit.cpp"
          "restricted_tree_column_sorter.cpp"
          "scoped_disconnect.cpp"
          "treeview_with_delete.cpp"
  MOC "dual_list_selector.hpp"
      "mvc/alphanum_sort_proxy.hpp"
      "mvc/delegating_header_view.hpp"
      "mvc/filter_ignoring_branch_nodes_proxy.hpp"
      "mvc/fixed_proxy_models.hpp"
      "mvc/flat_to_tree_proxy.hpp"
      "mvc/multiplicated_column_proxy.hpp"
      "no_undoredo_lineedit.hpp"
      "restricted_tree_column_sorter.hpp"
      "treeview_with_delete.hpp"
  LIBRARIES Qt5::Gui
            Qt5::Core
            Qt5::Widgets
            Util::Qt
)
