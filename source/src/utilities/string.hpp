#pragma once

#include <format>

namespace dvsku::dpf {
#define DPF_FORMAT(fmt, ...)	\
		std::format(fmt, ##__VA_ARGS__)

#define DPF_FORMAT_C(fmt, ...)	\
		std::format(fmt, ##__VA_ARGS__).c_str()
}