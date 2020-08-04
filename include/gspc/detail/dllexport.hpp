#pragma once

//! A function or type marked with this macro is exported into the
//! public binary API.
#define GSPC_DLLEXPORT __attribute__ ((visibility ("default")))
