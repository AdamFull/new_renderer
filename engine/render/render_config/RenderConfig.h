#pragma once

#include <engine/interface/render/RenderInterface.h>
#include <sol/sol.hpp>

#include <logger/logger.h>

namespace engine
{
	namespace render
	{
		class CRenderScriptExecutor
		{
		public:
			friend class CRenderScriptCore;

			~CRenderScriptExecutor();

			template<class _Ty>
			void addToEnvironment(const char* name, _Ty* data)
			{
				(*m_pEnvironment)[name] = data;
			}

			template<class... _Args>
			int run(const char* function, _Args&&... args)
			{
				auto found_function = m_scriptFunctions.find(utl::fnv1a_64_hash(function));
				if (found_function != m_scriptFunctions.end())
				{
					auto result = found_function->second->call(std::forward<_Args>(args)...);
					if (!result.valid())
					{
						auto err = result.get<sol::error>();
						log_error(err.what());
						return -1;
					}

					return 0;
				}
			}
		private:
			sol::environment* m_pEnvironment{ nullptr };
			std::unordered_map<u64, sol::protected_function*> m_scriptFunctions{};
		};

		class CRenderScriptCore
		{
		public:
			~CRenderScriptCore();
			void create(IRenderInterface* render_interface);
			std::unique_ptr<CRenderScriptExecutor> build(const std::string& name, const std::vector<uint8_t>& program_code);
		private:
			sol::state* m_pLuaState{ nullptr };
		};
	}
}