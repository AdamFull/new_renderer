#include "fs_zip_file.h"

#include "../../util/Logger.h"

#include <zip.h>

namespace edge
{
	namespace fs
	{
		static const char* GetZipCompressionMethodName(zip_uint16_t comp_method)
		{
			switch (comp_method) 
			{
			case ZIP_CM_DEFAULT:		return "default";
			case ZIP_CM_STORE:			return "store";
			case ZIP_CM_SHRINK:			return "shrink";
			case ZIP_CM_REDUCE_1:		return "reduce_1";
			case ZIP_CM_REDUCE_2:		return "reduce_2";
			case ZIP_CM_REDUCE_3:		return "reduce_3";
			case ZIP_CM_REDUCE_4:		return "reduce_4";
			case ZIP_CM_IMPLODE:		return "implode";
			case ZIP_CM_DEFLATE:		return "deflate";
			case ZIP_CM_DEFLATE64:		return "deflate64";
			case ZIP_CM_PKWARE_IMPLODE:	return "pkware";
			case ZIP_CM_BZIP2:			return "bzip2";
			case ZIP_CM_LZMA:			return "lzma";
			case ZIP_CM_TERSE:			return "terse";
			case ZIP_CM_LZ77:			return "lz77";
			case ZIP_CM_LZMA2:			return "lzma2";
			case ZIP_CM_ZSTD:			return "zstd";
			case ZIP_CM_XZ:				return "xz";
			case ZIP_CM_JPEG:			return "jpeg";
			case ZIP_CM_WAVPACK:		return "wavpach";
			case ZIP_CM_PPMD:			return "ppmd";
			default:					return "unknown";
			}
		}

		static const char* GetZipEncryptionMethodName(zip_uint16_t encryption_method)
		{
			switch (encryption_method)
			{
			case ZIP_EM_NONE:			return "none";
			case ZIP_EM_TRAD_PKWARE:	return "pkware";
			case ZIP_EM_AES_128:		return "aes128";
			case ZIP_EM_AES_192:		return "aes192";
			case ZIP_EM_AES_256:		return "aes256";
			default:					return "unknown";
			}
		}

		ZipFile::ZipFile(zip_t* zip, std::string_view path, FileModeFlags mode_flags) :
			zip_handle{ zip }
		{
			zip_int64_t idx = zip_name_locate(zip_handle, path.data(), ZIP_FL_ENC_GUESS);
			file_handle = zip_fopen_index(zip_handle, idx, 0);

			auto* zip_err = zip_get_error(zip_handle);
			if (zip_err)
			{
				char buffer[256];
				int buffer_offset = zip_error_to_str(buffer, sizeof(buffer), zip_err->zip_err, errno);

				zip_stat_t st;
				if (zip_stat(zip_handle, path.data(), ZIP_FL_NOCASE, &st) == 0)
				{
					buffer_offset = snprintf(buffer + buffer_offset, sizeof(buffer) - buffer_offset,
						"\nName: %s\nSize: %llu"
						"\nCompressed size: %llu\nModification time: %s"
						"\nCRC: %lu"
						"\nCompression method: %s"
						"\nEncryption method: %s",
						st.name, st.size,
						st.comp_size, ctime(&st.mtime),
						st.crc, 
						GetZipCompressionMethodName(st.comp_method),
						GetZipEncryptionMethodName(st.encryption_method)
					);
				}
				
				EDGE_LOGE("[ZipFile] Failed to open file: {}. Error: {}", path, buffer);
			}
		}

		ZipFile::~ZipFile()
		{
			Close();
		}

		auto ZipFile::IsOpen() -> bool
		{
			return (file_handle);
		}

		auto ZipFile::Close() -> void
		{
			if (file_handle)
			{
				zip_fclose(file_handle);
				file_handle = nullptr;
			}
		}

		auto ZipFile::Seek(uint64_t offset, SeekOrigin origin) -> uint64_t
		{
			if (!file_handle)
			{
				return 0ull;
			}

			return zip_fseek(file_handle, offset, (int32_t)origin);
		}

		auto ZipFile::Tell() -> uint64_t
		{
			if (!file_handle)
			{
				return 0ull;
			}

			return zip_ftell(file_handle);
		}

		auto ZipFile::Read(void* buffer, uint64_t size) -> uint64_t
		{
			if (!file_handle)
			{
				return 0ull;
			}

			return zip_fread(file_handle, buffer, size);
		}

		auto ZipFile::Write(const void* buffer, uint64_t size) -> uint64_t 
		{
			return 0ull;
		}
	}
}