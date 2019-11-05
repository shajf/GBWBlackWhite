local ffi = require "ffi"
local cast = ffi.cast
local ctostring = ffi.string
local getfenv = getfenv
 
ffi.cdef[[
     typedef struct gbw_http_session_t gbw_http_session_t;
     struct gbw_http_session_t {

		 uint32_t srcIP;
		 uint32_t dstIP;
         const char *method;
		 const char *uri;
		 const char *host;
     };
 ]]

