#pragma once

namespace engine
{
	namespace integration
	{
		class IIntegration
		{
		public:
			virtual ~IIntegration() = default;

			virtual void create() = 0;
			virtual void destroy() = 0;

			virtual void render() = 0;
		};

		class IIntegrationManager
		{
		public:
			virtual ~IIntegrationManager() = default;
		};
	}
}