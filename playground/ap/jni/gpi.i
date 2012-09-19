/* example.i */

%module gpi
%{
#include "gpi.h"
%}
%include "stdint.i"
%include "buffers.i"

%apply void* BUFF {void *buffer}

%include "gpi.h"
