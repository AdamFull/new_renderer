#pragma once

#include "TextureStructures.h"
#include "ShaderStructures.h"
#include "VertexStructures.h"
#include "BufferStructures.h"

namespace engine
{
	namespace render
	{
		class CUniformHandle
		{
		public:
			template<class _Ty>
			void Create(const _Ty& block)
			{
				m_data.resize(block.size);
				m_uniforms = block.uniforms;
			}

			// For render script
			void Set(u64 name, const void* raw_data, u64 size)
			{
				auto found = m_uniforms.find(name);
				if (found == m_uniforms.end() || !raw_data)
					return;

				auto& uniform = found->second;

				if (size == 0ull)
					size = uniform.size;

				std::memcpy(m_data.data() + uniform.offset, raw_data, size);
			}

			template<class _Ty>
			void Set(u64 name, const _Ty& object, u64 size = 0ull)
			{
				auto found = m_uniforms.find(name);
				if (found == m_uniforms.end())
					return;

				auto& uniform = found->second;

				if (size == 0ull)
					size = (std::min)(sizeof(_Ty), uniform.size);

				std::memcpy(m_data.data() + uniform.offset, &object, size);
			}

			template<class _Ty>
			_Ty* Get(u64 name)
			{
				auto found = m_uniforms.find(name);
				if (found == m_uniforms.end())
					return nullptr;

				auto& uniform = found->second;

				if (sizeof(_Ty) > uniform.size)
					return nullptr;

				return reinterpret_cast<_Ty*>(m_data.data() + uniform.offset);
			}

			const utl::vector<char>& Data() const
			{
				return m_data;
			}
		private:
			utl::hash_map<u64, FReflectionUniform> m_uniforms{};
			utl::vector<char> m_data{};
		};



		class IRenderResource
		{
		public:
			virtual ~IRenderResource() = default;

			u64 Handle() const { return m_resourceId; }

			explicit operator u64() const { return Handle(); }

		protected:
			u64 m_resourceId{};
		};


		class IBufferObject : public IRenderResource
		{
		public:
			virtual ~IBufferObject() = default;

			virtual void Create(const std::string& name, const FBufferCreateInfo& create_info) = 0;
			virtual void Resize(const std::string& name, const FBufferCreateInfo& create_info) = 0;
			virtual void Upload(utl::vector<u8>&& data, const utl::vector<FBufferUploadRegion>& regions) = 0;

			virtual void Map() = 0;
			virtual void Unmap() = 0;
			virtual void Write(const void* data, u64 size = ~0ull, u64 offset = 0ull) = 0;
			virtual void Flush(u64 size = ~0ull, u64 offset = 0ull) = 0;
			virtual void Clear() = 0;

			virtual u64 Size() = 0;
			virtual u64 InstanceSize() = 0;
			virtual u64 Count() = 0;

			virtual uint64_t Address() = 0;

			virtual void* Data() = 0;
		};

		class IGPUVector
		{
		public:
			struct upload_range
			{
				u64 offset{ 0ull };
				u64 size{ 0ull };
			};

			virtual ~IGPUVector() = default;

			inline void CreateRaw(const std::string& name, u64 instance_size, u64 capacity = 64ull, EBufferUsageFlags flag = EBufferUsageFlagBits::eStorageBuffer)
			{
				// Init intermediate data buffer
				m_vData.resize(instance_size * capacity);
				// Allocate buffer
				Allocate(name, flag, instance_size, capacity);
			}

			template<class _Ty>
			void Create(const std::string& name, u64 capacity = 64ull, EBufferUsageFlags flag = EBufferUsageFlagBits::eStorageBuffer)
			{
				CreateRaw(name, sizeof(_Ty), capacity, flag);
			}

			inline void CopyBytes(const void* data, u64 byte_count)
			{
				TryGrow(byte_count);
				DoCopy(data, byte_count);
			}

			inline void CopyRaw(const void* data, u64 count)
			{
				u64 size = ToBytes(count);
				CopyBytes(data, size);
			}

			template<class _Ty, size_t _Size>
			void Copy(const std::array<_Ty, _Size>& data)
			{
				assert(m_elemSize == sizeof(_Ty) && "Invalid element size.");
				CopyRaw(&data[0], _Size);
			}

			template<class _Ty>
			void Copy(const utl::vector<_Ty>& data)
			{
				assert(m_elemSize == sizeof(_Ty) && "Invalid element size.");

				if (data.empty())
					return;

				CopyRaw(&data[0], data.size());
			}

			template<class _Ty>
			_Ty* EmplaceBack(const _Ty& new_element = _Ty())
			{
				assert(m_elemSize == sizeof(_Ty) && "Invalid element size.");

				u64 size = sizeof(_Ty);
				TryGrow(size);
				DoCopy(&new_element, size);

				return (_Ty*)(&m_vData[m_cursor - size]);
			}

			inline void SetRaw(u64 index, const void* data)
			{
				DoInsert(data, m_elemSize, ToBytes(index));
			}

			template<class _Ty>
			void Set(u64 index, _Ty&& data)
			{
				assert(m_elemSize == sizeof(_Ty) && "Invalid element size.");
				SetRaw(index, &data);
			}

			u64 Pos() const
			{
				return m_cursor / m_elemSize;
			}

			u64 Count() const
			{
				return m_lastCursor / m_elemSize;
			}

			u64 Capacity() const
			{
				return m_vData.size() / m_elemSize;
			}

			u64 Stride() const
			{
				return m_elemSize;
			}

			template<class _Ty>
			const _Ty* at(u64 index) const
			{
				return (const _Ty*)(&m_vData[ToBytes(index)]);
			}

			virtual void Upload() = 0;
			virtual void Resize(u64 new_size) = 0;
			virtual const std::unique_ptr<IBufferObject>& Buffer() = 0;
		protected:
			inline u64 ToBytes(u64 size) const { return size * m_elemSize; }

			// Try to grow temporary array
			inline void TryGrow(u64 size)
			{
				if (m_cursor + size > m_vData.size())
				{
					u64 new_size = (m_cursor + size) * 2.f;
					m_vData.resize(new_size);
				}
			}

			inline void DoInsert(const void* data, u64 size, u64 offset)
			{
				auto new_hash = utl::murmur_hash_64(data, size, 0);
				auto old_hash = utl::murmur_hash_64(&m_vData[offset], size, 0);

				if (new_hash != old_hash)
				{
					std::memcpy(&m_vData[offset], data, size);
					m_vUploadRanges.emplace_back(upload_range{ offset, size });
				}
			}

			// Copy data only when it changed ans det dirty flag
			inline void DoCopy(const void* data, u64 size)
			{
				DoInsert(data, size, m_cursor);
				m_cursor += size;
				m_lastCursor = m_cursor;
			}

			// Allocates buffer
			virtual void Allocate(const std::string& name, EBufferUsageFlags flag, u64 stride, u64 capacity) = 0;

			utl::vector<upload_range> m_vUploadRanges{};
			utl::vector<u8> m_vData{};
			u64 m_cursor{ 0ull };	// Something like cursor in bytes
			u64 m_lastCursor{ 0ull };
			u64 m_elemSize{ 0ull };
		};

		class IShaderObject : public IRenderResource
		{
		public:
			virtual ~IShaderObject() = default;

			virtual void Create(const std::string& name, const FGraphicsShaderCreateInfo& create_info, const utl::vector<FShaderCreationBlockInfo>& stages) = 0;
			virtual void Create(const std::string& name, const FComputeShaderCreateInfo& create_info, const utl::vector<FShaderCreationBlockInfo>& stages) = 0;

			virtual const std::unique_ptr<CUniformHandle>& GetUniformBlock(u64 name) = 0;
			virtual const std::unique_ptr<CUniformHandle>& GetConstantBlock(u64 name) = 0;

			virtual void BindBuffer(u64 name, u64 buffer_resource) = 0;
			virtual void BindTexture(u64 name, u64 texture_resource, u32 mip = ~0u) = 0;

			virtual void FlushDescriptors() = 0;
			virtual void FlushConstants(EQueueTypeFlagBits queue) = 0;
			virtual void Bind(EQueueTypeFlagBits queue) = 0;
			virtual void BindResources(EQueueTypeFlagBits queue) = 0;

			virtual const FShaderReflectionData& GetReflection() const = 0;
		};

		class ITextureObject : public IRenderResource
		{
		public:
			virtual ~ITextureObject() = default;

			virtual void Create(const std::string& name, const FTextureCreateInfo& create_info) = 0;
			virtual void Create(const std::string& name, const FTextureCreateInfo& create_info, FTextureUploadInfo&& upload_info) = 0;
		};

		class IVertexHandler
		{
		public:
			virtual ~IVertexHandler() = default;

			virtual void Create(const FVertexHandlerCreateInfo& create_info) = 0;
			virtual void Allocate() = 0;

			virtual void Bind(EQueueTypeFlagBits queue) = 0;

			virtual const FVertexRange& AddMeshData(const utl::vector<FVertexAttributePointer>& attributes_data, const FVertexAttributePointer& indices) = 0;

			virtual u64 AddVertices(const utl::vector<FVertexAttributePointer>& attributes_data) = 0;
			virtual u64 AddVertices(const void* data, u64 count, u64 stride) = 0;
			virtual u64 AddIndices(const FVertexAttributePointer& indices) = 0;
			virtual u64 AddIndices(const void* data, u64 count, u64 stride) = 0;

			virtual const FVertexDeclarationInfo& GetVertexDeclaration() const = 0;
			virtual u64 GetHash() const = 0;
			virtual const std::string& GetDefinitions() const = 0;

			virtual const std::unique_ptr<IGPUVector>& GetGeometryBuffer(EGeometryBufferIndices buffer_index) = 0;
		};
	}
}