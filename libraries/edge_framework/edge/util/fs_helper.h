#pragma once

#include <hash.hpp>

namespace edge
{
	namespace fsh
	{
        namespace format
        {
            using namespace utl;
            // images
            constexpr inline uint32_t ktx = ".ktx"_hash32;
            constexpr inline uint32_t ktx2 = ".ktx2"_hash32;
            constexpr inline uint32_t dds = ".dds"_hash32;
            constexpr inline uint32_t png = ".png"_hash32;
            constexpr inline uint32_t jpg = ".jpg"_hash32;
            constexpr inline uint32_t jpeg = ".jpeg"_hash32;
            constexpr inline uint32_t tga = ".tga"_hash32;
            constexpr inline uint32_t bmp = ".bmp"_hash32;
            constexpr inline uint32_t psd = ".psd"_hash32;
            constexpr inline uint32_t gif = ".gif"_hash32;
            constexpr inline uint32_t hdr = ".hdr"_hash32;
            constexpr inline uint32_t pic = ".pic"_hash32;
            constexpr inline uint32_t pnm = ".pnm"_hash32;
            constexpr inline uint64_t image_ts = 13;

            // meshes
            constexpr inline uint32_t gltf = ".gltf"_hash32;
            constexpr inline uint32_t glb = ".glb"_hash32;
            constexpr inline uint32_t obj = ".obj"_hash32;
            constexpr inline uint32_t fbx = ".fbx"_hash32;
            constexpr inline uint64_t mesh_ts = 4;

            // audio
            constexpr inline uint32_t wav = ".wav"_hash32;
            constexpr inline uint32_t wave = ".wave"_hash32;
            constexpr inline uint32_t ogg = ".ogg"_hash32;
            constexpr inline uint64_t audio_ts = 3;
        }

        inline constexpr std::string_view get_mount_point(std::string_view path) 
        {
            auto first_separator = path.find('/');
            std::string_view result = path.substr(0, first_separator);

            if (first_separator != std::string_view::npos) 
            {
                auto second_separator = path.find('/', first_separator + 1);
                result = path.substr(0, second_separator);
            }
            return result.empty() ? "/" : result;
        }

        template<size_t _Size>
        inline constexpr std::string_view to_posix_path(std::string_view path, std::span<char, _Size> buffer)
        {
            size_t index = 0;
            for (char ch : path) 
            {
                if (index >= _Size - 1) break;
                buffer[index++] = (ch == '\\') ? '/' : ch;
            }

            buffer[index] = '\0';
            return std::string_view(buffer.data());
        }

        template<size_t _Size>
        inline constexpr std::string_view to_posix_path(std::string_view path, std::array<char, _Size> buffer)
        {
            return to_posix_path(path, std::span(buffer));
        }

        inline constexpr std::string_view get_local_path(std::string_view path) 
        {
            auto mount_point = get_mount_point(path);

            if (!path.starts_with(mount_point))
                return {};

            auto local_path = path.substr(mount_point.size());
            if (!local_path.empty() && local_path.front() == '/')
                local_path = local_path.substr(1);

            return local_path;
        }

        inline constexpr std::string_view get_ext(std::string_view path) 
        {
            auto dot_pos = path.rfind('.');
            return (dot_pos == std::string_view::npos) ? std::string_view{} : path.substr(dot_pos);
        }

        inline constexpr std::string_view get_filename(std::string_view path) 
        {
            auto last_sep = path.find_last_of("/\\");
            return (last_sep == std::string_view::npos) ? path : path.substr(last_sep + 1);
        }

        inline constexpr std::string_view get_stem(std::string_view path) 
        {
            auto filename = get_filename(path);
            auto ext = get_ext(filename);
            return filename.substr(0, filename.size() - ext.size());
        }

        inline constexpr std::string_view parent_path(std::string_view path) 
        {
            auto separator_pos = path.find_last_of("/\\");
            return (separator_pos == std::string_view::npos) ? std::string_view{} : path.substr(0, separator_pos);
        }

        template<size_t _Size>
        inline constexpr std::string_view normalize(std::string_view path, std::span<char, _Size> buffer) 
        {
            size_t index = 0;

            for (size_t i = 0; i < path.size(); ++i) 
            {
                if (index >= buffer.size() - 1) break;

                if (path[i] == '/' && (index == 0 || buffer[index - 1] == '/'))
                {
                    if (i >= buffer.size() - 1) break;
                    continue; // Skip duplicate '/'
                }

                if (path[i] == '.' && (i + 1 < path.size() && path[i + 1] == '/')) 
                {
                    if (i >= buffer.size() - 1) break;
                    ++i; // Skip "./"
                    continue;
                }
                buffer[index++] = path[i];
            }

            return std::string_view(buffer.data(), index);
        }

        template<size_t _Size>
        inline constexpr std::string_view normalize(std::string_view path, std::array<char, _Size> buffer)
        {
            return normalize(path, std::span(buffer));;
        }

        template<size_t _Size>
        inline constexpr std::string_view path_append(std::string_view base_path, std::string_view relative_path, std::span<char, _Size> buffer)
        {
            size_t index = 0;

            // Copy base_path to the buffer
            for (char ch : base_path) 
            {
                if (index >= buffer.size() - 1) break;
                buffer[index++] = ch;
            }

            // Add a separator if needed
            if (!base_path.empty() && base_path.back() != '/' && base_path.back() != '\\') 
            {
                if (index < buffer.size() - 1)
                    buffer[index++] = '/';
            }

            // Skip leading separator in relative_path if present
            size_t rel_start = (relative_path.front() == '/' || relative_path.front() == '\\') ? 1 : 0;

            // Copy relative_path to the buffer
            for (size_t i = rel_start; i < relative_path.size(); ++i) 
            {
                if (index >= buffer.size() - 1) break;
                buffer[index++] = relative_path[i];
            }

            // Null-terminate
            if (index < buffer.size())
                buffer[index] = '\0';

            return std::string_view(buffer.data(), index);
        }

        template<size_t _Size>
        inline constexpr std::string_view path_append(std::string_view base_path, std::string_view relative_path, std::array<char, _Size> buffer)
        {
            return path_append(base_path, relative_path, std::span(buffer));
        }

        //inline constexpr bool is_image_format(std::string_view path)
        //{
        //    auto ext = get_ext(path);
        //    auto format_hash = utl::murmur_hash(ext);
        //    return format_hash == format::ktx ||
        //        format_hash == format::ktx2 ||
        //        format_hash == format::dds ||
        //        format_hash == format::png ||
        //        format_hash == format::jpg;
        //}
        //
        //inline constexpr bool is_ktx_format(std::string_view path)
        //{
        //    auto ext = get_ext(path);
        //    auto format_hash = utl::murmur_hash(ext);
        //    return format_hash == format::ktx || format_hash == format::ktx2;
        //}
        //
        //inline constexpr bool is_mesh_format(std::string_view path)
        //{
        //    auto ext = get_ext(path);
        //    auto format_hash = utl::murmur_hash(ext);
        //    return format_hash == format::gltf ||
        //        format_hash == format::glb ||
        //        format_hash == format::obj ||
        //        format_hash == format::fbx;
        //}
        //
        //inline constexpr bool is_gltf_format(std::string_view path)
        //{
        //    auto ext = get_ext(path);
        //    auto format_hash = utl::murmur_hash(ext);
        //    return format_hash == format::gltf || format_hash == format::glb;
        //}
        //
        //inline constexpr bool is_obj_format(std::string_view path)
        //{
        //    auto ext = get_ext(path);
        //    auto format_hash = utl::murmur_hash(ext);
        //    return format_hash == format::obj;
        //}
        //
        //inline constexpr bool is_fbx_format(std::string_view path)
        //{
        //    auto ext = get_ext(path);
        //    auto format_hash = utl::murmur_hash(ext);
        //    return format_hash == format::fbx;
        //}
        //
        //inline constexpr bool is_audio_format(std::string_view path)
        //{
        //    auto ext = get_ext(path);
        //    auto format_hash = utl::murmur_hash(ext);
        //    return format_hash == format::wav ||
        //        format_hash == format::wave ||
        //        format_hash == format::ogg;
        //}
	}
}