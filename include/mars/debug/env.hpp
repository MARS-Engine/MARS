#pragma once

namespace mars::env {

#if defined (_WIN32)
static constexpr bool platform_win32 = true; 
#else
static constexpr bool platform_win32 = false;
#endif

#if defined (__linux__)
static constexpr bool platform_linux = true;
#else
static constexpr bool platform_linux = false;
#endif

#ifdef MARS_FORMATTED_OUTPUT
static constexpr bool formatted_output = true;
#else
static constexpr bool formatted_output = false;
#endif

}