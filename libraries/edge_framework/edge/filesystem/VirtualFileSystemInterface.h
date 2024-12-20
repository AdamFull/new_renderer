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

            virtual bool CreateDirectories(std::string_view path) = 0;
            virtual bool CreateDirectory(std::string_view path) = 0;

            virtual bool Exists(std::string_view path) = 0;

            virtual size_t FileSize(std::string_view path) = 0;

            virtual bool IsDirectory(std::string_view path) = 0;
            virtual bool IsFile(std::string_view path) = 0;

            virtual bool Remove(std::string_view path) = 0;
            virtual void Rename(std::string_view original, std::string_view newfile) = 0;

            virtual std::unique_ptr<IVFSOutputFileInterface> CreateFile(std::string_view path) = 0;
            virtual std::unique_ptr<IVFSInputFileInterface> OpenFile(std::string_view path) = 0;

            virtual std::unique_ptr<IDirectoryIteratorInterface> Walk(std::string_view path, bool recursive) = 0;

            virtual bool ReadFile(std::string_view path, char* data, size_t size) = 0;
            virtual bool ReadFile(std::string_view path, std::string& data) = 0;
            virtual bool ReadFile(std::string_view path, std::vector<uint8_t>& data) = 0;
            virtual bool ReadFile(std::string_view path, std::vector<uint32_t>& data) = 0;

            virtual bool WriteFile(std::string_view path, const std::string& data) = 0;
            virtual bool WriteFile(std::string_view path, const std::vector<uint8_t>& binary) = 0;

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