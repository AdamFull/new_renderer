#pragma once

#include <tracy/Tracy.hpp>

#define EDGE_PROFILE_ZONE_SCOPED ZoneScoped
#define EDGE_PROFILE_ZONE_SCOPED_NAMED(name) ZoneScopedN(name)
#define EDGE_PROFILE_FRAME_MARK FrameMark