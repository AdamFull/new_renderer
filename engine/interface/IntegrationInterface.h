#pragma once

namespace engine
{
	namespace integration
	{
		class IIntegration
		{
		public:
			virtual ~IIntegration() = default;
		};

		class IIntegrationManager
		{
		public:
			virtual ~IIntegrationManager() = default;
		};
	}
}