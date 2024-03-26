#pragma once

#include <format>

namespace dvsku::dpf {
#define DPF_FORMAT(fmt, ...)	\
		std::vformat(fmt, std::make_format_args(__VA_ARGS__))

#define DPF_FORMAT_C(fmt, ...)	\
		std::vformat(fmt, std::make_format_args(__VA_ARGS__)).c_str()
}