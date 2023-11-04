#pragma once

#include <interface/RenderInterface.h>
#include <udetail.hpp>

namespace engine
{
	namespace render
	{
		class CUniformHandle
		{
		public:
			template<class _Ty>
			void create(const _Ty& block)
			{
				data = std::make_unique<char[]>(block.size);
				uniforms = block.uniforms;
			}

			template<class _Ty>
			void set(const _Ty& object, size_t offset, size_t size)
			{
				if (!data)
					return;

				std::memcpy(data.get(), ((char*)&object) + offset, size);
			}

			template<class _Ty>
			void set(const char* name, const _Ty& object, size_t size = 0ull)
			{
				auto found = uniforms.find(utl::fnv1a_64_hash(name));
				if (found == uniforms.end())
					return;

				auto& uniform = found->second;

				if (size == 0ull)
					size = (std::min)(sizeof(_Ty), uniform.size);

				set(object, uniform.offset, size);
			}
		private:
			std::unordered_map<size_t, FReflectionUniform> uniforms{};
			std::unique_ptr<char[]> data{};
		};

		class CShaderObject
		{
		public:
			void create(FResourceView resource, const FShaderReflectionData& reflection);
			const std::unique_ptr<CUniformHandle>& getUniformBlock(const char* name);
			void bindTexture(const char* name, FResourceView texture_resource);
		private:
			std::unordered_map<size_t, std::unique_ptr<CUniformHandle>> uniformBlocks{};
			std::unordered_map<size_t, FResourceView> bindTextures{};
			FResourceView shaderView{};
		};
	}
}