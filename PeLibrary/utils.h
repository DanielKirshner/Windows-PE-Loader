#pragma once

#include <stdbool.h>
#include <stdint.h>

#define HAS_FLAG(num, flag) (((num) & (flag)) == (flag))

/**
@brief
	Converts section characteristics to memory protection flags.
	Checks the given section characteristics from PE headers and maps them to right Windows memory protection flags.
	The conversion is based on whether the section has read, write, or execute permissions.
@param
	`characteristics` The section characteristics to be checked.
	This is a combination of IMAGE_SCN_MEM_* flags that describe the section's memory attributes (e.g., read, write, execute).
@param
	`protection` Pointer to a size_t variable where the resultant memory protection flag will be stored.
	The protection flag corresponds to one of the PAGE_* constants.
@return
	Returns true if a valid protection flag is determined, otherwise returns false.
 */
bool Utils__section_characteristics_to_protection_flag(
	__in const size_t characteristics,
	__out size_t* const protection);