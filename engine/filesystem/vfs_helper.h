#pragma once

#include <ie_vfs_exports.h>
#include <utility.hpp>

namespace engine
{
	namespace filesystem
	{
        namespace format
        {
            using namespace utl;
            // images
            constexpr inline u32 ktx = ".ktx"_utl_hash;
            constexpr inline u32 ktx2 = ".ktx2"_utl_hash;
            constexpr inline u32 dds = ".dds"_utl_hash;
            constexpr inline u32 png = ".png"_utl_hash;
            constexpr inline u32 jpg = ".jpg"_utl_hash;
            constexpr inline u64 image_ts = 5;

            // meshes
            constexpr inline u32 gltf = ".gltf"_utl_hash;
            constexpr inline u32 glb = ".glb"_utl_hash;
            constexpr inline u32 obj = ".obj"_utl_hash;
            constexpr inline u32 fbx = ".fbx"_utl_hash;
            constexpr inline u64 mesh_ts = 2;

            // scripts
            constexpr inline u32 js = ".js"_utl_hash;
            constexpr inline u64 script_ts = 1;

            // scenes
            constexpr inline u32 iescene = ".iescene"_utl_hash; // incenerate scene
            constexpr inline u64 scene_ts = 1;

            // audio
            constexpr inline u32 wav = ".wav"_utl_hash;
            constexpr inline u32 wave = ".wave"_utl_hash;
            constexpr inline u32 ogg = ".ogg"_utl_hash;
            constexpr inline u64 audio_ts = 3;

            // particles
            constexpr inline u32 efkefc = ".efkefc"_utl_hash;
        }

		struct INCENERATE_VFS_EXPORT fs
		{
            static std::string from_unicode(const std::u16string& data);
			static std::string get_mount_point(const std::string& path);
			static std::string to_posix_path(const std::string& path);
			static std::string get_local_path(const std::string& path);

            static std::string get_ext(const std::string& path);

            static std::string get_filename(const std::string& path);
            static std::string get_stem(const std::string& path); // Returns filename without extension
            static std::string parent_path(const std::string& path);
            static std::string path_append(const std::string& base_path, const std::string& relative_path);
            static std::string normalize(const std::string& path);

            static bool is_scene_format(const std::string& path);
            static bool is_image_format(const std::string& path);
            static bool is_ktx_format(const std::string& path);
            static bool is_mesh_format(const std::string& path);
            static bool is_gltf_format(const std::string& path);
            static bool is_obj_format(const std::string& path);
            static bool is_fbx_format(const std::string& path);
            static bool is_skybox_format(const std::string& path);
            static bool is_script_format(const std::string& path);
            static bool is_audio_format(const std::string& path);
            static bool is_particle_format(const std::string& path);

        private:
            static bool is_scene_format(u32 format);
            static bool is_image_format(u32 format);
            static bool is_ktx_format(u32 format);
            static bool is_mesh_format(u32 format);
            static bool is_gltf_format(u32 format);
            static bool is_obj_format(u32 format);
            static bool is_fbx_format(u32 format);
            static bool is_skybox_format(u32 format);
            static bool is_script_format(u32 format);
            static bool is_audio_format(u32 format);
            static bool is_particle_format(u32 format);
		};
	}
}