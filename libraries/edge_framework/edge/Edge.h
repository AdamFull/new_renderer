#pragma once

#include "EntryPoint.h"
#include "runtime/Runtime.h"

namespace edge
{
	std::unique_ptr<Runtime> CreateApplicationContext(const RuntimeContext& context);
}