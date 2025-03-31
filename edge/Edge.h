#pragma once

#include "EntryPoint.h"
#include "runtime/rt_runtime.h"

namespace edge
{
	std::unique_ptr<Runtime> create_application_context(IRuntimeContext& context);
}