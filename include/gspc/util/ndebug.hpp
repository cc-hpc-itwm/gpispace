#pragma once

#ifndef NDEBUG
#define IFNDEF_NDEBUG(x...) x
#define IFDEF_NDEBUG(x...)
#else
#define IFNDEF_NDEBUG(x...)
#define IFDEF_NDEBUG(x...) x
#endif
