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
		class NsightAftermath
		{
		public:
			NsightAftermath();
			virtual ~NsightAftermath();

			bool IsInitialized();

			void BeginFrame();
			void EndFrame();

			virtual void RegisterShaderBinary(std::span<uint8_t> shader_code) = 0;
			virtual void RegisterShaderDebugBinary(std::span<uint8_t> stripped_shader_code, std::span<uint8_t> full_shader_code) = 0;

			void SetMarker(CommandList* cmd_list, const char* name);

		private:
			void OnGpuCrashDump(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize);
			void OnShaderDebugInfo(const void* pShaderDebugInfo, const uint32_t shaderDebugInfoSize);
			void OnCrashDumpDescription(PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription addDescription);
			void OnResolveMarker(const void* pMarkerData, const uint32_t markerDataSize, void** ppResolvedMarkerData, uint32_t* pResolvedMarkerDataSize);

			void OnShaderDebugInfoLookup(const GFSDK_Aftermath_ShaderDebugInfoIdentifier* pIdentifier, PFN_GFSDK_Aftermath_SetData setShaderDebugInfo);
			void OnShaderLookup(const GFSDK_Aftermath_ShaderBinaryHash* pShaderHash, PFN_GFSDK_Aftermath_SetData setShaderBinary);
			void OnShaderSourceDebugInfoLookup(const GFSDK_Aftermath_ShaderDebugName* pShaderDebugName, PFN_GFSDK_Aftermath_SetData setShaderBinary);

			static void GpuCrashDumpCallback(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize, void* pUserData);
			static void ShaderDebugInfoCallback(const void* pShaderDebugInfo, const uint32_t shaderDebugInfoSize, void* pUserData);
			static void CrashDumpDescriptionCallback(PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription addDescription, void* pUserData);
			static void ResolveMarkerCallback(const void* pMarkerData, const uint32_t markerDataSize, void* pUserData, void** ppResolvedMarkerData, uint32_t* pResolvedMarkerDataSize);

			static void ShaderDebugInfoLookupCallback(const GFSDK_Aftermath_ShaderDebugInfoIdentifier* pIdentifier, PFN_GFSDK_Aftermath_SetData setShaderDebugInfo, void* pUserData);
			static void ShaderLookupCallback(const GFSDK_Aftermath_ShaderBinaryHash* pShaderHash, PFN_GFSDK_Aftermath_SetData setShaderBinary, void* pUserData);
			static void ShaderSourceDebugInfoLookupCallback(const GFSDK_Aftermath_ShaderDebugName* pShaderDebugName, PFN_GFSDK_Aftermath_SetData setShaderBinary, void* pUserData);

		protected:
			virtual void SetMarkerImpl(CommandList* cmd_list, const void* data_index) = 0;

			std::unordered_map<GFSDK_Aftermath_ShaderDebugInfoIdentifier, std::vector<uint8_t>> shader_debug_data{};
			std::unordered_map<GFSDK_Aftermath_ShaderBinaryHash, std::span<uint8_t>> shader_binary_map{};
			std::unordered_map<GFSDK_Aftermath_ShaderDebugName, std::span<uint8_t>> shader_full_binary_map{};

			const static uint32_t frame_history_size = 4;
			std::array<std::unordered_map<uint64_t, std::string>, frame_history_size> per_frame_markers{};

			uint32_t frame_index{ 0u };
			bool initialized{ false };
		};
	}
}

#endif