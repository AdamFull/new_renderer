#include "gfx_nsight_aftermath.h"

#include <fstream>
#include <filesystem>

#if USE_NSIGHT_AFTERMATH

constexpr const char* base_path_str = "nvaftermath";

namespace edge::gfx
{
	NsightAftermath* NsightAftermath::self{ nullptr };

	NsightAftermath::NsightAftermath() {
		GFSDK_Aftermath_Result result = GFSDK_Aftermath_EnableGpuCrashDumps(
			GFSDK_Aftermath_Version_API,
			GFSDK_Aftermath_GpuCrashDumpWatchedApiFlags_Vulkan,
			GFSDK_Aftermath_GpuCrashDumpFeatureFlags_DeferDebugInfoCallbacks, // Let the Nsight Aftermath library cache shader debug information.
			gpu_crash_dump_callback,                                             // Register callback for GPU crash dumps.
			shader_debug_info_callback,                                          // Register callback for shader debug information.
			crash_dump_description_callback,                                     // Register callback for GPU crash dump description.
			resolve_marker_callback,                                            // Register callback for resolving application-managed markers.
			this);

		initialized = GFSDK_Aftermath_SUCCEED(result);
		if (initialized) {
			self = this;
		}
	}

	NsightAftermath::~NsightAftermath() {
		if (initialized) {
			initialized = false;
			self = nullptr;

			GFSDK_Aftermath_DisableGpuCrashDumps();
		}
	}

	bool NsightAftermath::is_initialized() {
		return initialized;
	}

	void NsightAftermath::begin_frame() {
		per_frame_markers[frame_index % frame_history_size].clear();
	}

	void NsightAftermath::try_begin_frame() {
		if (self) {
			self->begin_frame();
		}
	}

	void NsightAftermath::end_frame() {
		frame_index++;
	}

	void NsightAftermath::try_end_frame() {
		if (self) {
			self->end_frame();
		}
	}

	void NsightAftermath::set_marker(ICommandList* cmd_list, const char* name) {
		uint32_t marker_map_index = frame_index % frame_history_size;
		auto& current_frame_marker_map = per_frame_markers[marker_map_index];
		size_t marker_id = marker_map_index * 10000 + current_frame_marker_map.size() + 1;

		set_marker_impl(cmd_list, (const void*)marker_id);
	}

	void NsightAftermath::try_set_marker(ICommandList* cmd_list, const char* name) {
		if (self) {
			self->set_marker(cmd_list, name);
		}
	}

	void NsightAftermath::on_gpu_crash_dump(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize) {
		if (!std::filesystem::exists(base_path_str)) {
			std::filesystem::create_directories(base_path_str);
		}

		// Create a GPU crash dump decoder object for the GPU crash dump.
		GFSDK_Aftermath_GpuCrashDump_Decoder decoder = {};

		GFSDK_Aftermath_GpuCrashDump_CreateDecoder(GFSDK_Aftermath_Version_API, pGpuCrashDump, gpuCrashDumpSize, &decoder);

		// Use the decoder object to read basic information, like application
		// name, PID, etc. from the GPU crash dump.
		GFSDK_Aftermath_GpuCrashDump_BaseInfo baseInfo = {};
		GFSDK_Aftermath_GpuCrashDump_GetBaseInfo(decoder, &baseInfo);

		// Use the decoder object to query the application name that was set
		// in the GPU crash dump description.
		uint32_t applicationNameLength = 0;
		GFSDK_Aftermath_GpuCrashDump_GetDescriptionSize(decoder, GFSDK_Aftermath_GpuCrashDumpDescriptionKey_ApplicationName, &applicationNameLength);

		std::string applicationName(applicationNameLength, '\0');
		GFSDK_Aftermath_GpuCrashDump_GetDescription(decoder, GFSDK_Aftermath_GpuCrashDumpDescriptionKey_ApplicationName, applicationNameLength, applicationName.data());

		// Create a unique file name for writing the crash dump data to a file.
		// Note: due to an Nsight Aftermath bug (will be fixed in an upcoming
		// driver release) we may see redundant crash dumps. As a workaround,
		// attach a unique count to each generated file name.
		static int count = 0;
		const std::string baseFileName = std::format("{}-{}-{}", applicationName, baseInfo.pid, ++count);

		// Write the crash dump data to a file using the .nv-gpudmp extension
		// registered with Nsight Graphics.
		const std::string crashDumpFileName = baseFileName + ".nv-gpudmp";
		std::ofstream dumpFile(crashDumpFileName, std::ios::out | std::ios::binary);
		if (dumpFile) {
			dumpFile.write((const char*)pGpuCrashDump, gpuCrashDumpSize);
			dumpFile.close();
		}

		// Decode the crash dump to a JSON string.
		// Step 1: Generate the JSON and get the size.
		uint32_t jsonSize = 0;
		GFSDK_Aftermath_GpuCrashDump_GenerateJSON(
			decoder,
			GFSDK_Aftermath_GpuCrashDumpDecoderFlags_ALL_INFO,
			GFSDK_Aftermath_GpuCrashDumpFormatterFlags_NONE,
			shader_debug_info_lookup_callback,
			shader_lookup_callback,
			shader_source_debug_Info_lookup_callback,
			this,
			&jsonSize);

		// Step 2: Allocate a buffer and fetch the generated JSON.
		std::vector<char> json(jsonSize);
		GFSDK_Aftermath_GpuCrashDump_GetJSON(decoder, jsonSize, json.data());

		// Write the crash dump data as JSON to a file.
		const std::string jsonFileName = crashDumpFileName + ".json";
		std::ofstream jsonFile(jsonFileName, std::ios::out | std::ios::binary);
		if (jsonFile) {
			// Write the JSON to the file (excluding string termination)
			jsonFile.write(json.data(), json.size() - 1);
			jsonFile.close();
		}

		// Destroy the GPU crash dump decoder object.
		GFSDK_Aftermath_GpuCrashDump_DestroyDecoder(decoder);
	}

	void NsightAftermath::on_shader_debug_info(const void* pShaderDebugInfo, const uint32_t shaderDebugInfoSize) {
		if (!std::filesystem::exists(base_path_str)) {
			std::filesystem::create_directories(base_path_str);
		}

		// Get shader debug information identifier
		GFSDK_Aftermath_ShaderDebugInfoIdentifier identifier = {};
		GFSDK_Aftermath_Result result = GFSDK_Aftermath_GetShaderDebugInfoIdentifier(GFSDK_Aftermath_Version_API, pShaderDebugInfo, shaderDebugInfoSize, &identifier);

		if (GFSDK_Aftermath_SUCCEED(result)) {
			shader_debug_data[identifier] = std::vector<uint8_t>((uint8_t*)pShaderDebugInfo, (uint8_t*)pShaderDebugInfo + shaderDebugInfoSize);

			std::filesystem::path base_path{ base_path_str };
			base_path /= std::format("shader-{}-{}.nvdbg", identifier.id[0], identifier.id[1]);
			std::ofstream fout(base_path, std::ios::out | std::ios::binary);
			if (fout)
			{
				fout.write((const char*)pShaderDebugInfo, shaderDebugInfoSize);
			}
		}
	}

	void NsightAftermath::on_crash_dump_description(PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription addDescription) {
		addDescription(GFSDK_Aftermath_GpuCrashDumpDescriptionKey_ApplicationName, "EdgeApplication");
		addDescription(GFSDK_Aftermath_GpuCrashDumpDescriptionKey_ApplicationVersion, "v1.0");
	}

	void NsightAftermath::on_resolve_marker(const void* pMarkerData, const uint32_t markerDataSize, void** ppResolvedMarkerData, uint32_t* pResolvedMarkerDataSize) {
		for (auto& markers : per_frame_markers) {
			const auto& found_marker = markers.find((uint64_t)pMarkerData);
			if (found_marker != markers.end()) {
				const std::string& found_marker_data = found_marker->second;
				// std::string::data() will return a valid pointer until the string is next modified
				// we don't modify the string after calling data() here, so the pointer should remain valid
				*ppResolvedMarkerData = (void*)found_marker_data.data();
				*pResolvedMarkerDataSize = (uint32_t)found_marker_data.length();
				return;
			}
		}
	}

	void NsightAftermath::on_shader_debug_info_lookup(const GFSDK_Aftermath_ShaderDebugInfoIdentifier* pIdentifier, PFN_GFSDK_Aftermath_SetData setShaderDebugInfo) {
		auto found = shader_debug_data.find(*pIdentifier);
		if (found != shader_debug_data.end()) {
			setShaderDebugInfo(found->second.data(), static_cast<uint32_t>(found->second.size()));
		}
	}

	void NsightAftermath::on_shader_lookup(const GFSDK_Aftermath_ShaderBinaryHash* pShaderHash, PFN_GFSDK_Aftermath_SetData setShaderBinary) {
		auto found = shader_binary_map.find(*pShaderHash);
		if (found != shader_binary_map.end()) {
			setShaderBinary(found->second.data(), static_cast<uint32_t>(found->second.size()));
		}
	}

	void NsightAftermath::on_shader_source_debug_info_lookup(const GFSDK_Aftermath_ShaderDebugName* pShaderDebugName, PFN_GFSDK_Aftermath_SetData setShaderBinary) {
		auto found = shader_full_binary_map.find(*pShaderDebugName);
		if (found != shader_full_binary_map.end()) {
			setShaderBinary(found->second.data(), static_cast<uint32_t>(found->second.size()));
		}
	}


	void NsightAftermath::gpu_crash_dump_callback(const void* pGpuCrashDump, const uint32_t gpuCrashDumpSize, void* pUserData) {
		auto* handle = static_cast<NsightAftermath*>(pUserData);
		handle->on_gpu_crash_dump(pGpuCrashDump, gpuCrashDumpSize);
	}

	void NsightAftermath::shader_debug_info_callback(const void* pShaderDebugInfo, const uint32_t shaderDebugInfoSize, void* pUserData) {
		auto* handle = static_cast<NsightAftermath*>(pUserData);
		handle->on_shader_debug_info(pShaderDebugInfo, shaderDebugInfoSize);
	}

	void NsightAftermath::crash_dump_description_callback(PFN_GFSDK_Aftermath_AddGpuCrashDumpDescription addDescription, void* pUserData) {
		auto* handle = static_cast<NsightAftermath*>(pUserData);
		handle->on_crash_dump_description(addDescription);
	}

	void NsightAftermath::resolve_marker_callback(const void* pMarkerData, const uint32_t markerDataSize, void* pUserData, void** ppResolvedMarkerData, uint32_t* pResolvedMarkerDataSize) {
		auto* handle = static_cast<NsightAftermath*>(pUserData);
		handle->on_resolve_marker(pMarkerData, markerDataSize, ppResolvedMarkerData, pResolvedMarkerDataSize);
	}

	void NsightAftermath::shader_debug_info_lookup_callback(const GFSDK_Aftermath_ShaderDebugInfoIdentifier* pIdentifier, PFN_GFSDK_Aftermath_SetData setShaderDebugInfo, void* pUserData) {
		auto* handle = static_cast<NsightAftermath*>(pUserData);
		handle->on_shader_debug_info_lookup(pIdentifier, setShaderDebugInfo);
	}

	void NsightAftermath::shader_lookup_callback(const GFSDK_Aftermath_ShaderBinaryHash* pShaderHash, PFN_GFSDK_Aftermath_SetData setShaderBinary, void* pUserData) {
		auto* handle = static_cast<NsightAftermath*>(pUserData);
		handle->on_shader_lookup(pShaderHash, setShaderBinary);
	}

	void NsightAftermath::shader_source_debug_Info_lookup_callback(const GFSDK_Aftermath_ShaderDebugName* pShaderDebugName, PFN_GFSDK_Aftermath_SetData setShaderBinary, void* pUserData) {
		auto* handle = static_cast<NsightAftermath*>(pUserData);
		handle->on_shader_source_debug_info_lookup(pShaderDebugName, setShaderBinary);
	}
}

#endif