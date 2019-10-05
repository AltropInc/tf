#include "Enum.h"
#include <iostream>
#include <assert.h>
#include <cstring>

namespace tf {

void EnumBase::setEnumNames(
	const char *names_buf,
	const char**names,
	size_t enum_num)
{
	size_t scanned(0);
	const char* cp (names_buf);
	const char* start_cp(cp);
	bool string_started (false);
	while (true)
	{
		if (scanned>=enum_num)
		{
			break;
		}
	    if (*cp==',' || *cp=='\0')
		{
			const bool at_end(*cp=='\0');
			const char* end_pos = const_cast<char*>(cp-1);
			while (end_pos >= start_cp && isspace(*end_pos))
			{
				--end_pos;
			}
			*const_cast<char*>(end_pos+1) = '\0';
			names[scanned++] = start_cp;
			++cp;
			start_cp = cp;
			string_started = false;
			if (at_end)
			{
				break;
			}
			continue;
		}
		if (isspace(*cp))
		{
			if (!string_started)
			{
				++cp;
				++start_cp;
				continue;
			}
		}
		string_started = true;
		++cp;
  	}
	assert(scanned==enum_num);
}

int EnumBase::fromString(const char** name_list, const int* name_indice, const char* enum_name, size_t enum_number)
{
	if (name_indice[0] < 0)
	{
		int* indice = const_cast<int*>(name_indice);
		int res = -1;
		for (int i=0; i<enum_number; ++i)
		{
			*(indice+i) = i;
			if (res<0 && name_list[*(indice+i)] &&
			    std::strcmp(name_list[*(indice+i)], enum_name) == 0)
			{
				res = i;
			}
		}
		// Not sorted yet. Sort names in name_indice
		for (int n=0; n<enum_number; ++n)
		{
			for (int m=n+1; m<enum_number; ++m)
			{
				if (name_list[*(indice+n)])
				{
					if (!name_list[*(indice+m)] ||
					    std::strcmp(name_list[*(indice+m)], name_list[*(indice+n)])<0 )
					{
						std::swap(*(indice+n), *(indice+m));
					}
				}
				else
				{
					break;
				}
				
			}
		}
		return res;
	}
	const int * start = name_indice;
	const int * end = name_indice+enum_number-1;
	while (start < end)
	{
		const int * mid = start + (end-start)/2;
		int res = name_list[*mid] ? std:: strcmp(enum_name, name_list[*mid]) : 1;
		if (res==0)
		{
			return *mid;
		}
		if (res<0)
		{
			end = mid;
		}
		else
		{
			start = end;
		}
	}
	return -1;
}

//ENUM(Color, uint8_t, Red, Green, Blue);
//EnumSet<Color> colors(Color::Red, Color::Green);
} // namespace tf

//using namespace tf;

//int main(void)
//{
//	Color c = Color::Red;
//	std::cout << c << std::endl;
//	std::cout << colors << std::endl;
//	return 0;
//}
