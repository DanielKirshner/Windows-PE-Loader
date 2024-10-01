#include "utils.h"

#include <Windows.h>

bool Utils__section_characteristics_to_protection_flag(
	__in const uint32_t characteristics,
	__out uint32_t* const protection)
{
	*protection = 0x00;

	if (HAS_FLAG(characteristics, IMAGE_SCN_MEM_WRITE) &&
		HAS_FLAG(characteristics, IMAGE_SCN_MEM_EXECUTE))
	{
		*protection = PAGE_EXECUTE_READWRITE;
		return true;
	}
	if (HAS_FLAG(characteristics, IMAGE_SCN_MEM_EXECUTE))
	{
		*protection = PAGE_EXECUTE_READ;
		return true;
	}
	if (HAS_FLAG(characteristics, IMAGE_SCN_MEM_WRITE))
	{
		*protection = PAGE_READWRITE;
		return true;
	}
	if (HAS_FLAG(characteristics, IMAGE_SCN_MEM_READ))
	{
		*protection = PAGE_READONLY;
		return true;
	}
	return false;
}