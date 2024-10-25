#pragma once

#include <GFXStructures.h>
#include <cassert>

namespace engine
{
	namespace gfx
	{
		inline ResourceState GetTextureState(TextureAccess access)
		{
			ResourceState resource_state{};

			switch (access.layout)
			{
			case engine::gfx::TextureLayout::eUndefined:
			case engine::gfx::TextureLayout::ePresent:
			{
				resource_state.stage = StageFlag::eAllCommands;
				resource_state.access = AccessFlag::eNone;
			} break;
			case engine::gfx::TextureLayout::eColorAttachmentOptimal:
			{
				resource_state.stage = StageFlag::eColorAttachmentOutput;

				if(access.type & AccessTypeFlag::eRead)
					resource_state.access |= AccessFlag::eColorAttachmentRead;
				if (access.type & AccessTypeFlag::eWrite)
					resource_state.access |= AccessFlag::eColorAttachmentWrite;
			} break;
			case engine::gfx::TextureLayout::eDepthStencilAttachmentOptimal:
			case engine::gfx::TextureLayout::eDepthAttachmentOptimal:
			{
				resource_state.stage = StageFlag::eEarlyFragmentTests | StageFlag::eLateFragmentTests;

				if (access.type & AccessTypeFlag::eRead)
					resource_state.access = AccessFlag::eDepthStencilAttachmentRead;
				if (access.type & AccessTypeFlag::eWrite)
					resource_state.access = AccessFlag::eDepthStencilAttachmentWrite;
			} break;
			case engine::gfx::TextureLayout::eDepthReadOnlyOptimal:
			case engine::gfx::TextureLayout::eDepthStencilReadOnlyOptimal:
			{
				resource_state.stage = StageFlag::eComputeShader | StageFlag::eAllGraphics;
				resource_state.access = AccessFlag::eShaderRead | AccessFlag::eDepthStencilAttachmentRead;
			} break;
			case engine::gfx::TextureLayout::eShader:
			{
				resource_state.stage = StageFlag::eComputeShader | StageFlag::eAllGraphics;

				if (access.type & AccessTypeFlag::eRead)
					resource_state.access |= AccessFlag::eShaderRead | AccessFlag::eShaderStorageRead | AccessFlag::eShaderSampledRead;
				if (access.type & AccessTypeFlag::eWrite)
					resource_state.access |= AccessFlag::eShaderWrite | AccessFlag::eShaderStorageWrite;
			} break;
			case engine::gfx::TextureLayout::eTransfer:
			{
				resource_state.stage = StageFlag::eTransfer;

				if(access.type & AccessTypeFlag::eRead)
					resource_state.access |= AccessFlag::eTransferRead;
				if(access.type & AccessTypeFlag::eWrite)
					resource_state.access |= AccessFlag::eTransferWrite;
			} break;
			case engine::gfx::TextureLayout::ePreinitialized:
			{
				resource_state.stage = StageFlag::eHost;

				if (access.type & AccessTypeFlag::eRead)
					resource_state.access |= AccessFlag::eHostRead;
				if (access.type & AccessTypeFlag::eWrite)
					resource_state.access |= AccessFlag::eHostWrite;
			} break;
			case engine::gfx::TextureLayout::eAttachmentOptimal:
			case engine::gfx::TextureLayout::eReadOnlyOptimal:
			case engine::gfx::TextureLayout::eStencilReadOnlyOptimal:
			case engine::gfx::TextureLayout::eStencilAttachmentOptimal:
			case engine::gfx::TextureLayout::eDepthReadOnlyStencilAttachmentOptimal:
			case engine::gfx::TextureLayout::eDepthAttachmentStencilReadOnlyOptimal:
			{
				assert(false && "NOT IMPLEMENTED");
			} break;
			default:
				break;
			}

			return resource_state;
		}
	}
}