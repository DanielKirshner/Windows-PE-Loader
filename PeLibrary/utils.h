#pragma once

#include <stdbool.h>
#include <stdint.h>

#define HAS_FLAG(num, flag) (((num) & (flag)) == (flag))

bool Utils__section_characteristics_to_protection_flag(
	const uint32_t characteristics,
	uint32_t* const protection);