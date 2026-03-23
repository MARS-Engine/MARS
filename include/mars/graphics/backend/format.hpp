#pragma once

#include <mars/utility/enum_flags.hpp>

#include <climits>
#include <meta>
#include <optional>
#include <type_traits>

enum mars_buffer_type {
	MARS_BUFFER_TYPE_VERTEX = 1 << 1,
	MARS_BUFFER_TYPE_INDEX = 1 << 2,
	MARS_BUFFER_TYPE_UNIFORM = 1 << 3,
	MARS_BUFFER_TYPE_UNIFORM_TEXEL_BUFFER = 1 << 4,
	MARS_BUFFER_TYPE_TRANSFER_SRC = 1 << 5,
	MARS_BUFFER_TYPE_TRANSFER_DST = 1 << 6,
	MARS_BUFFER_TYPE_UNORDERED_ACCESS = 1 << 7,
	MARS_BUFFER_TYPE_STRUCTURED = 1 << 8,
	MARS_BUFFER_TYPE_SHADER_RESOURCE = 1 << 9,
};

enum mars_buffer_property {
	MARS_BUFFER_PROPERTY_HOST_VISIBLE = 1 << 1,
	MARS_BUFFER_PROPERTY_DEVICE_LOCAL = 1 << 2,
};

enum mars_format_type {
	MARS_FORMAT_UNDEFINED,
	MARS_FORMAT_R8_UNORM,
	MARS_FORMAT_RG8_UNORM,
	MARS_FORMAT_RGB8_UNORM,
	MARS_FORMAT_RGBA8_UNORM,
	MARS_FORMAT_R8_SRGB,
	MARS_FORMAT_RG8_SRGB,
	MARS_FORMAT_RGB8_SRGB,
	MARS_FORMAT_RGBA8_SRGB,
	MARS_FORMAT_BGR8_SRGB,
	MARS_FORMAT_BGRA8_SRGB,
	MARS_FORMAT_R32_SFLOAT,
	MARS_FORMAT_RG32_SFLOAT,
	MARS_FORMAT_RGB32_SFLOAT,
	MARS_FORMAT_RGBA32_SFLOAT,
	MARS_FORMAT_R32_UINT,
	MARS_FORMAT_RG32_UINT,
	MARS_FORMAT_RGB32_UINT,
	MARS_FORMAT_RGBA32_UINT,
	MARS_FORMAT_RGBA16_SFLOAT,
	MARS_FORMAT_R32_TYPELESS,
};

enum mars_depth_format {
	MARS_DEPTH_FORMAT_UNDEFINED,
	MARS_DEPTH_FORMAT_D32_SFLOAT,
};

enum mars_texture_filter_mode {
	MARS_TEXTURE_FILTER_LINEAR,
	MARS_TEXTURE_FILTER_NEAREST
};

enum mars_texture_usage : uint8_t {
	MARS_TEXTURE_USAGE_TRANSFER_SRC = 1 << 1,
	MARS_TEXTURE_USAGE_TRANSFER_DST = 1 << 2,
	MARS_TEXTURE_USAGE_SAMPLED = 1 << 3,
	MARS_TEXTURE_USAGE_COLOR_ATTACHMENT = 1 << 4,
	MARS_TEXTURE_USAGE_STORAGE = 1 << 5,
};

enum mars_texture_type : uint8_t {
	MARS_TEXTURE_TYPE_2D,
	MARS_TEXTURE_TYPE_CUBE,
};

template <>
struct mars::enum_flags::enabled<mars_texture_usage> : std::true_type {};

namespace mars::graphics {
template <typename T>
consteval mars_format_type make_format() {
	mars_format_type result = {};

	const auto get_type = [](std::meta::info _info) -> mars_format_type {
		if (std::meta::is_same_type(_info, ^^float))
			return MARS_FORMAT_R32_SFLOAT;
		else if (std::meta::is_same_type(_info, ^^unsigned int))
			return MARS_FORMAT_R32_UINT;
		throw "unsupported type";
	};

	if constexpr (std::meta::is_class_type(^^T)) {
		std::optional<std::meta::info> value;

		std::meta::access_context ctx = std::meta::access_context::current();

		for (auto mem : std::meta::nonstatic_data_members_of(^^T, ctx)) {
			std::meta::info mem_type_info = std::meta::type_of(mem);

			if (value.has_value() && !std::meta::is_same_type(value.value(), mem_type_info))
				throw "all non static data members must have the same type";

			if (result == MARS_FORMAT_UNDEFINED)
				result = get_type(mem_type_info);
			else
				result = static_cast<mars_format_type>(result + 1);
			value = mem_type_info;
		}
	} else
		result = get_type(^^T);

	return result;
}
} // namespace mars::graphics
