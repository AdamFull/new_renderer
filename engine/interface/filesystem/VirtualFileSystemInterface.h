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

            virtual void Mount(const char* mountpoint, std::unique_ptr<IVFSInterface>&& vfsptr) = 0;
            virtual void Unmount(const char* mountpoint) = 0;

            virtual void Copy(const char* from, const char* to) = 0;

            virtual bool CreateDirectories(const char* path) = 0;
            virtual bool CreateDirectory(const char* path) = 0;

            virtual bool Exists(const char* path) = 0;

            virtual size_t FileSize(const char* path) = 0;

            virtual bool IsDirectory(const char* path) = 0;
            virtual bool IsFile(const char* path) = 0;

            virtual bool Remove(const char* path) = 0;
            virtual void Rename(const char* original, const char* newfile) = 0;

            virtual std::unique_ptr<IVFSOutputFileInterface> CreateFile(const char* path) = 0;
            virtual std::unique_ptr<IVFSInputFileInterface> OpenFile(const char* path) = 0;

            virtual std::unique_ptr<IDirectoryIteratorInterface> Walk(const char* path, bool recursive) = 0;

            virtual bool ReadFile(const char* path, char* data, size_t size) = 0;
            virtual bool ReadFile(const char* path, std::string& data) = 0;
            virtual bool ReadFile(const char* path, utl::vector<uint8_t>& data) = 0;
            virtual bool ReadFile(const char* path, utl::vector<uint32_t>& data) = 0;

            virtual bool WriteFile(const char* path, const std::string& data) = 0;
            virtual bool WriteFile(const char* path, const utl::vector<uint8_t>& binary) = 0;

            template<class _Ty>
            bool ReadJson(const char* path, _Ty& type)
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
            bool WriteJson(const char* path, const _Ty& type, int32_t indent = -1)
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
            bool ReadBson(const char* path, _Ty& type)
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
            bool WriteBson(const char* path, const _Ty& type)
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