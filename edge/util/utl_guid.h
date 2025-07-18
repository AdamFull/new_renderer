#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <random>
#include <format>
#include <array>
#include <unordered_map>
#include <functional>
#include <nlohmann/json.hpp>

#include "utl_hash.hpp"

namespace edge::utl {
	class guid {
	public:
        guid() = default;

        constexpr guid(const char* str) {
            parse(str);
        }

        auto to_string() const -> std::string {
            return std::format("{:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
                data[0], data[1], data[2], data[3],
                data[4], data[5],
                data[6], data[7],
                data[8], data[9],
                data[10], data[11], data[12], data[13], data[14], data[15]);
        }

        constexpr auto operator==(const guid& other) const -> bool {
            for (size_t i = 0; i < 16; ++i) {
                if (data[i] != other.data[i]) {
                    return false;
                }
            }
            return true;
        }

        constexpr auto operator!=(const guid& other) const -> bool {
            return !(*this == other);
        }

        // Less than operator for use in containers
        constexpr auto operator<(const guid& other) const -> bool {
            for (size_t i = 0; i < 16; ++i) {
                if (data[i] < other.data[i]) return true;
                if (data[i] > other.data[i]) return false;
            }
            return false;
        }

        // Check if GUID is null (all zeros)
        constexpr auto is_null() const -> bool {
            for (const auto& byte : data) {
                if (byte != 0) return false;
            }
            return true;
        }

        // Access to raw data
        constexpr auto get_raw() const -> const uint8_t* { return data.data(); }

        auto generate() -> void {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, 255);

            for (auto& byte : data) {
                byte = dis(gen);
            }

            // Set version (4) in the most significant 4 bits of the 7th byte
            data[6] = (data[6] & 0x0F) | 0x40;

            // Set variant (10) in the most significant 2 bits of the 9th byte
            data[8] = (data[8] & 0x3F) | 0x80;
        }

        constexpr auto parse(const char* str) -> void {
            data = {
                _d(str, 0), _d(str, 2), _d(str, 4), _d(str, 6),
                _d(str, 9), _d(str, 11),
                _d(str, 14), _d(str, 16),
                _d(str, 19), _d(str, 21),
                _d(str, 24), _d(str, 26), _d(str, 28), _d(str, 30), _d(str, 32), _d(str, 34)
            };
        }
	private:
        constexpr auto _v(const char c) const -> uint8_t {
            const char a = c | 0x20;
            return (a >= '0' && a <= '9') ? a - '0' : (a >= 'a' && a <= 'f') ? 10 + a - 'a' : throw std::logic_error{ "invalid hex character in GUID" };
        }

        constexpr uint8_t _d(const char* str, const int index) {
            return ((_v(str[index]) << 4) + _v(str[index + 1]));
        }

		std::array<uint8_t, 16> data;
	};

    inline void to_json(nlohmann::json& j, const guid& type) { 
        j = type.to_string();
    }

    inline void from_json(const nlohmann::json& j, guid& type) { 
        if (!j.is_string()) {
            throw std::invalid_argument{ "GUID must be deserialized from a string" };
        }

        std::string str = j.get<std::string>();

        // Remove braces if present
        if (str.length() == 38 && str.front() == '{' && str.back() == '}') {
            str = str.substr(1, 36);
        }

        if (str.length() != 36) {
            throw std::invalid_argument{ "Invalid GUID string format" };
        }

        type.parse(str.c_str());
    }

    constexpr auto operator "" _guid(const char* str, size_t size) -> guid {
        using namespace std::string_literals;

        if (size != 36 && size != 38)
            throw std::logic_error{ "invalid GUID format, allowed format are {XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX} or XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX"s };

        return guid(size == 36 ? str : str + 1);
    }

    inline std::ostream& operator<<(std::ostream& os, const guid& g) {
        return os << g.to_string();
    }
}

namespace std {
    template<>
    struct hash<edge::utl::guid> {
        std::size_t operator()(const edge::utl::guid& g) const noexcept {
            const uint8_t* data = g.get_raw();
            uint32_t seed = edge::murmur_hash(reinterpret_cast<const char*>(data), 8, 0);
            return edge::murmur_hash(reinterpret_cast<const char*>(data) + 8, 8, seed);
        }
    };
}