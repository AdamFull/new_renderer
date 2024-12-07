#pragma once

#include "interface/EngineSystem.h"

namespace engine
{
	class CEngineSystemManager
	{
	public:
		template<class _Ty, class... _Args>
		void RegisterSystem(_Args&& ...args)
		{
			m_vSystems.emplace_back(std::make_unique<_Ty>(std::forward<_Args>(args)...));
		}

		void PreInitialize();
		void Initialize();
		void PostInitialize();

		void PreUpdate(f32 dt);
		void Update(f32 dt);
		void PostUpdate(f32 dt);

		void Render();

		void PreDestroy();
		void Destroy();
		void PostDestroy();
	private:
		utl::vector<std::unique_ptr<IEngineSystem>> m_vSystems{};
	};
}