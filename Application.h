#pragma once

#include <interface/Application.h>

namespace engine
{
	class CApplication : public IApplication
	{
	public:
		~CApplication() = default;

		void Initialize() override;
		void Update(f32 dt) override;
		void Render() override;
	};
}