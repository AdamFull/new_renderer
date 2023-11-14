#pragma once

#if ENABLE_ENGINE_PROFILING

#if defined(PLATFORM_WIN32)
#define TRACY_CALLSTACK 1
#endif

#else

#endif