#pragma once

//! Change the value of the given variable in the current scope and
//! make sure the variable is set to it's original value at scope
//! exit.
//! usage:
//! - FHG_UTIL_IN_THIS_SCOPE (variable) = value;
//! - FHG_UTIL_IN_THIS_SCOPE (variable) (value);
//! - FHG_UTIL_IN_THIS_SCOPE (variable) (value1, value2);
#define FHG_UTIL_IN_THIS_SCOPE(variable) \
  FHG_UTIL_IN_THIS_SCOPE_IMPL (variable)

#include <gspc/util/in_this_scope.ipp>
