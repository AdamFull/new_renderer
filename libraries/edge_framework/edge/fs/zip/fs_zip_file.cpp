#include "fs_zip_file.h"

#include "../../util/Logger.h"

#include <zip.h>

namespace edge::fs
{
	static const char* get_zip_compression_method_name(zip_uint16_t comp_method) {
		switch (comp_method) {
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

	static const char* get_zip_encryption_method_name(zip_uint16_t encryption_method) {
		switch (encryption_method) {
		case ZIP_EM_NONE:			return "none";
		case ZIP_EM_TRAD_PKWARE:	return "pkware";
		case ZIP_EM_AES_128:		return "aes128";
		case ZIP_EM_AES_192:		return "aes192";
		case ZIP_EM_AES_256:		return "aes256";
		default:					return "unknown";
		}
	}

	ZipFile::ZipFile(zip_t* zip, std::string_view path, FileModeFlags mode_flags) :
		zip_handle{ zip } {
		constexpr const zip_flags_t flags{ ZIP_FL_ENC_GUESS | ZIP_FL_NOCASE };

		auto file_index = zip_name_locate(zip_handle, path.data(), flags);
		if (file_index < 0) {
			EDGE_LOGE("[ZipFile] Zip error: {}. File \"{}\"", zip_strerror(zip_handle), path);
			return;
		}

		file_stat = new zip_stat_t;
		if (zip_stat_index(zip_handle, file_index, flags, file_stat) != 0) {
			EDGE_LOGE("[ZipFile] Failed to read \"{}\" file metadata.", path);
			return;
		}

		file_handle = zip_fopen_index(zip_handle, file_index, 0);

		auto* zip_err = zip_get_error(zip_handle);
		if (zip_err && zip_err->zip_err != 0) {
			char buffer[256];
			int buffer_offset = zip_error_to_str(buffer, sizeof(buffer), zip_err->zip_err, errno);

			buffer_offset = snprintf(buffer + buffer_offset, sizeof(buffer) - buffer_offset,
				"\nName: %s\nSize: %llu"
				"\nCompressed size: %llu\nModification time: %s"
				"\nCRC: %lu"
				"\nCompression method: %s"
				"\nEncryption method: %s",
				file_stat->name, file_stat->size,
				file_stat->comp_size, ctime(&file_stat->mtime),
				file_stat->crc,
				get_zip_compression_method_name(file_stat->comp_method),
				get_zip_encryption_method_name(file_stat->encryption_method)
			);

			EDGE_LOGE("[ZipFile] Failed to open file \"{}\". Error: {}", path, buffer);
		}
	}

	ZipFile::~ZipFile() {
		if (file_stat) {
			delete file_stat;
			file_stat = nullptr;
		}

		close();
	}

	auto ZipFile::is_open() -> bool {
		return (file_handle);
	}

	auto ZipFile::close() -> void {
		std::lock_guard<std::mutex> lock(mutex);
		if (file_handle) {
			zip_fclose(file_handle);
			file_handle = nullptr;
		}
	}

	auto ZipFile::size() -> uint64_t {
		if (!file_handle)
			return 0ull;

		return file_stat->size;
	}

	auto ZipFile::seek(uint64_t offset, SeekOrigin origin) -> int64_t
{
		std::lock_guard<std::mutex> lock(mutex);
		if (zip_fseek(file_handle, static_cast<long>(offset), static_cast<int>(origin)) != 0)
			return -1;
		return zip_ftell(file_handle);
	}

	auto ZipFile::tell() -> int64_t {
		std::lock_guard<std::mutex> lock(mutex);
		if (!file_handle)
			return -1;

		return zip_ftell(file_handle);
	}

	auto ZipFile::read(void* buffer, uint64_t size) -> int64_t {
		std::lock_guard<std::mutex> lock(mutex);
		if (!file_handle)
			return -1;

		return zip_fread(file_handle, buffer, size);
	}

	auto ZipFile::write(const void* buffer, uint64_t size) -> int64_t {
		assert(false && "NOT IMPLEMENTED");
		return -1;
	}
}