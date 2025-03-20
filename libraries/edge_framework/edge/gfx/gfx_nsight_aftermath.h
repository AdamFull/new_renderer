#pragma once

#include <span>
#include <unordered_map>

#include "gfx_base.h"

#if USE_NSIGHT_AFTERMATH
#include <GFSDK_Aftermath.h>
#include <GFSDK_Aftermath_GpuCrashDump.h>
#include <GFSDK_Aftermath_GpuCrashDumpDecoding.h>

inline bool operator==(const GFSDK_Aftermath_ShaderDebugInfoIdentifier& lhs, const GFSDK_Aftermath_ShaderDebugInfoIdentifier& rhs)
{
	return lhs.id[0] == rhs.id[0] && lhs.id[1] == rhs.id[1];
}

template <>
struct std::hash<GFSDK_Aftermath_ShaderDebugInfoIdentifier>
{
	std::size_t operator()(const GFSDK_Aftermath_ShaderDebugInfoIdentifier& k) const
	{
		return std::hash<uint64_t>()(k.id[0]) ^ (std::hash<uint64_t>()(k.id[1]) << 1);
	}
};

inline bool operator==(const GFSDK_Aftermath_ShaderBinaryHash& lhs, const GFSDK_Aftermath_ShaderBinaryHash& rhs)
{
	return lhs.hash == rhs.hash;
}

template <>
struct std::hash<GFSDK_Aftermath_ShaderBinaryHash>
{
	std::size_t operator()(const GFSDK_Aftermath_ShaderBinaryHash& k) const
	{
		return std::hash<uint64_t>()(k.hash);
	}
};

inline bool operator==(const GFSDK_Aftermath_ShaderDebugName& lhs, const GFSDK_Aftermath_ShaderDebugName& rhs)
{
	return strncmp(lhs.name, rhs.name, sizeof(lhs.name)) == 0;
}

template <>
struct std::hash<GFSDK_Aftermath_ShaderDebugName>
{
	std::size_t operator()(const GFSDK_Aftermath_ShaderDebugName& k) const
	{
		return std::hash<const char*>()(k.name);
	}
};

namespace edge
{
	namespace gfx
	{
		class NsightAftermath {
		public:
			NsightAftermath();
			virtual ~NsightAftermath();

			bool is_initialized();

			void begin_frame();
			static void try_begin_frame();

			void end_frame();
			static void try_end_frame();

			virtual void register_shader_binary(std::span<uint8_t> shader_code) = 0;
			virtual void register_shader_debug_binary(std::span<uint8_t> stripped_shader_code, std::span<uint8_t> full_shader_code) = 0;

			void set_marker(ICommandList* cmd_list, const char* name);
			static void try_set_marker(ICommandList* cmd_list, const char* name);

		private:
			void on_gpu_crash_dump(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize);
			void on_shader_debug_info(const void* pShaderDebugInfo, const uint32_t shaderDebugInfoSize);
			void on_crash_dump_description(PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription addDescription);
			void on_resolve_marker(const void* pMarkerData, const uint32_t markerDataSize, void** ppResolvedMarkerData, uint32_t* pResolvedMarkerDataSize);

			void on_shader_debug_info_lookup(const GFSDK_Aftermath_ShaderDebugInfoIdentifier* pIdentifier, PFN_GFSDK_Aftermath_SetData setShaderDebugInfo);
			void on_shader_lookup(const GFSDK_Aftermath_ShaderBinaryHash* pShaderHash, PFN_GFSDK_Aftermath_SetData setShaderBinary);
			void on_shader_source_debug_info_lookup(const GFSDK_Aftermath_ShaderDebugName* pShaderDebugName, PFN_GFSDK_Aftermath_SetData setShaderBinary);

			static void gpu_crash_dump_callback(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize, void* pUserData);
			static void shader_debug_info_callback(const void* pShaderDebugInfo, const uint32_t shaderDebugInfoSize, void* pUserData);
			static void crash_dump_description_callback(PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription addDescription, void* pUserData);
			static void resolve_marker_callback(const void* pMarkerData, const uint32_t markerDataSize, void* pUserData, void** ppResolvedMarkerData, uint32_t* pResolvedMarkerDataSize);

			static void shader_debug_info_lookup_callback(const GFSDK_Aftermath_ShaderDebugInfoIdentifier* pIdentifier, PFN_GFSDK_Aftermath_SetData setShaderDebugInfo, void* pUserData);
			static void shader_lookup_callback(const GFSDK_Aftermath_ShaderBinaryHash* pShaderHash, PFN_GFSDK_Aftermath_SetData setShaderBinary, void* pUserData);
			static void shader_source_debug_info_lookup_callback(const GFSDK_Aftermath_ShaderDebugName* pShaderDebugName, PFN_GFSDK_Aftermath_SetData setShaderBinary, void* pUserData);

		protected:
			virtual void set_marker_impl(ICommandList* cmd_list, const void* data_index) = 0;

			std::unordered_map<GFSDK_Aftermath_ShaderDebugInfoIdentifier, std::vector<uint8_t>> shader_debug_data{};
			std::unordered_map<GFSDK_Aftermath_ShaderBinaryHash, std::span<uint8_t>> shader_binary_map{};
			std::unordered_map<GFSDK_Aftermath_ShaderDebugName, std::span<uint8_t>> shader_full_binary_map{};

			const static uint32_t frame_history_size = 4;
			std::array<std::unordered_map<uint64_t, std::string>, frame_history_size> per_frame_markers{};

			uint32_t frame_index{ 0u };
			bool initialized{ false };

			static NsightAftermath* self;
		};
	}
}

#endif