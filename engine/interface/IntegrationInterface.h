#pragma once

namespace engine
{
	namespace integration
	{
		class IRenderIntegration
		{
		public:
			virtual ~IRenderIntegration() = default;

			virtual void Init() = 0;
			virtual void destroy() = 0;

			virtual void preRender() = 0;
			virtual void postRender() = 0;
		};

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