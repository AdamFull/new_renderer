#pragma once

#include "VFSInterface.h"
#include "VFSInputFileInterface.h"
#include "VFSOutputFileInterface.h"
#include "DirectoryIteratorInterface.h"

#include "../util/Serialization.hpp"

#include <string_view>

namespace edge
{
    namespace filesystem
    {
        class IVirtualFileSystemInterface
        {
        public:
            virtual ~IVirtualFileSystemInterface() = default;

            virtual void Mount(std::string_view mountpoint, std::unique_ptr<IVFSInterface>&& vfsptr) = 0;
            virtual void Unmount(std::string_view mountpoint) = 0;

            virtual void Copy(std::string_view from, std::string_view to) = 0;

            [[nodiscard]] virtual auto CreateDirectories(std::string_view path) -> bool = 0;
            [[nodiscard]] virtual auto CreateDirectory(std::string_view path) -> bool = 0;

            [[nodiscard]] virtual auto Exists(std::string_view path) -> bool = 0;

            [[nodiscard]] virtual auto FileSize(std::string_view path) -> size_t = 0;

            [[nodiscard]] virtual auto IsDirectory(std::string_view path) -> bool = 0;
            [[nodiscard]] virtual auto IsFile(std::string_view path) -> bool = 0;

            [[nodiscard]] virtual auto Remove(std::string_view path) -> bool = 0;
            virtual void Rename(std::string_view original, std::string_view newfile) = 0;

            [[nodiscard]] virtual auto CreateFile(std::string_view path) -> std::unique_ptr<IVFSOutputFileInterface> = 0;
            [[nodiscard]] virtual auto OpenFile(std::string_view path) -> std::unique_ptr<IVFSInputFileInterface> = 0;

            [[nodiscard]] virtual auto Walk(std::string_view path, bool recursive) -> std::unique_ptr<IDirectoryIteratorInterface> = 0;

            [[nodiscard]] virtual auto ReadFile(std::string_view path, char* data, size_t size) -> bool = 0;
            [[nodiscard]] virtual auto ReadFile(std::string_view path, std::string& data) -> bool = 0;
            [[nodiscard]] virtual auto ReadFile(std::string_view path, std::vector<uint8_t>& data) -> bool = 0;
            [[nodiscard]] virtual auto ReadFile(std::string_view path, std::vector<uint32_t>& data) -> bool = 0;

            [[nodiscard]] virtual auto WriteFile(std::string_view path, const std::string& data) -> bool = 0;
            [[nodiscard]] virtual auto WriteFile(std::string_view path, const std::vector<uint8_t>& binary) -> bool = 0;

            template<class _Ty>
            bool ReadJson(std::string_view path, _Ty& type)
            {
                std::string data{};
                if (ReadFile(path, data))
                {
                    if (!data.empty())
                    {
                        auto json = nlohmann::json::parse(data);
                        if (!json.empty())
                        {
                            json.get_to(type);
                            return true;
                        }
                    }
                }

                return false;
            }

            template<class _Ty>
            bool WriteJson(std::string_view path, const _Ty& type, int32_t indent = -1)
            {
                auto json = nlohmann::json(type);
                if (!json.empty())
                {
                    auto data = json.dump(indent);
                    if (!data.empty())
                        return WriteFile(path, data);
                }

                return false;
            }

            template<class _Ty>
            bool ReadBson(std::string_view path, _Ty& type)
            {
                std::string data{};
                if (ReadFile(path, data))
                {
                    auto json = nlohmann::json::from_bson(data);
                    if (!json.empty())
                    {
                        json.get_to(type);
                        return true;
                    }
                }

                return false;
            }

            template<class _Ty>
            bool WriteBson(std::string_view path, const _Ty& type)
            {
                auto json = nlohmann::json(type);
                if (!json.empty())
                {
                    auto data = nlohmann::json::to_bson(json);
                    if (!data.empty())
                        return WriteFile(path, data);
                }

                return false;
            }
        };
    }
}