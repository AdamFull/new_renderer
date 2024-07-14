#pragma once

#include "VFSInterface.h"
#include "VFSInputFileInterface.h"
#include "VFSOutputFileInterface.h"
#include "DirectoryIteratorInterface.h"
#include <nlohmann/json.hpp>

namespace engine
{
    namespace filesystem
    {
        class IVirtualFileSystemInterface
        {
        public:
            virtual ~IVirtualFileSystemInterface() = default;

            virtual void Mount(const std::string& mountpoint, std::unique_ptr<IVFSInterface>&& vfsptr) = 0;
            virtual void Unmount(const std::string& mountpoint) = 0;

            virtual void Copy(const std::string& from, const std::string& to) = 0;

            virtual bool CreateDirectories(const std::string& path) = 0;
            virtual bool CreateDirectory(const std::string& path) = 0;

            virtual bool Exists(const std::string& path) = 0;

            virtual size_t FileSize(const std::string& path) = 0;

            virtual bool IsDirectory(const std::string& path) = 0;
            virtual bool IsFile(const std::string& path) = 0;

            virtual bool Remove(const std::string& path) = 0;
            virtual void Rename(const std::string& original, const std::string& newfile) = 0;

            virtual std::unique_ptr<IVFSOutputFileInterface> CreateFile(const std::string& path) = 0;
            virtual std::unique_ptr<IVFSInputFileInterface> OpenFile(const std::string& path) = 0;

            virtual std::unique_ptr<IDirectoryIteratorInterface> Walk(const std::string& path, bool recursive) = 0;

            virtual bool ReadFile(const std::string& path, char* data, size_t size) = 0;
            virtual bool ReadFile(const std::string& path, std::string& data) = 0;
            virtual bool ReadFile(const std::string& path, utl::vector<uint8_t>& data) = 0;

            virtual bool WriteFile(const std::string& path, const std::string& data) = 0;
            virtual bool WriteFile(const std::string& path, const utl::vector<uint8_t>& binary) = 0;

            template<class _Ty>
            bool ReadJson(const std::string& path, _Ty& type)
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
            bool WriteJson(const std::string& path, const _Ty& type, int32_t indent = -1)
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
            bool ReadBson(const std::string& path, _Ty& type)
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
            bool WriteBson(const std::string& path, const _Ty& type)
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