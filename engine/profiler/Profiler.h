#pragma once

#if !defined(BUILD_WITHOUT_DEBUGGIND) && defined(ENABLE_TRACY_PROFILING)

#if _WIN32
#undef max
#undef min
#endif // _WIN32

#include <tracy/Tracy.hpp>
#ifdef PROFILE_VULKAN
#include <tracy/TracyVulkan.hpp>
#endif // PROFILE_VULKAN

#define TRACY_PROFILE_SCOPE(name) ZoneScopedN(name)
#define TRACY_PROFILE_FUNCTION() ZoneScoped
#define TRACY_PROFILE_FRAMEMARKER() FrameMark
#define TRACY_PROFILE_TAG(name) ZoneText(name, strlen(name))
#define TRACY_PROFILE_LOG(text, size) TracyMessage(text, size)
#define TRACY_PROFILE_LOCK(type, var, name) TracyLockableN(type, var, name)
#define TRACY_PROFILE_LOCKMARKER(var) LockMark(var)
#define TRACY_PROFILE_SETTHREADNAME(name) tracy::SetThreadName(name)

#else

#define TRACY_PROFILE_SCOPE(name)
#define TRACY_PROFILE_FUNCTION()
#define TRACY_PROFILE_FRAMEMARKER()
#define TRACY_PROFILE_TAG(name)
#define TRACY_PROFILE_LOG(text, size)
#define TRACY_PROFILE_LOCK(type, var, name) type var
#define TRACY_PROFILE_LOCKMARKER(var)
#define TRACY_PROFILE_SETTHREADNAME(name)
#define TRACY_PROFILE_FUNCTION_LOW()
#define TRACY_PROFILE_SCOPE_LOW(name)

#endif