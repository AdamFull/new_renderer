#pragma once

#include <string>

namespace engine
{
	struct FWindowCreateInfo
	{

	};

	class IWindowInterface
	{
	public:
		virtual ~IWindowInterface() = default;

		virtual void create(const FWindowCreateInfo& window_ci) = 0;
		virtual void destroy() = 0;
	};
}