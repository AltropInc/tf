
#include "StrScan.h"

using namespace tf;

size_t StrScan::split (
	std::vector<std::string>& substrings,
	char separator,
	bool skip_leading_sp,
	bool skip_trailing_sp
)
{
	size_t scanned(0);
	size_t start_pos(pos_);
	size_t end_pos(pos_);
	bool string_started (false);
	while (pos_ < length_)
	{
	    if (str_[pos_]==separator)
		{
			substrings.emplace_back(str_+start_pos, end_pos-start_pos);
			string_started = false;
			++scanned;
			start_pos = pos_+1;
			end_pos = start_pos;
		}
		else if (isspace(str_[pos_]))
		{
			if (skip_leading_sp && !string_started)
			{
				++start_pos;
				++end_pos;
			}
			else if (!skip_trailing_sp  && string_started)
			{
				++end_pos;
			}
			    
		}
		else
		{
			string_started = true;
			++end_pos;
		}
		++pos_;
  	}
	if (end_pos>start_pos)
	{
	    substrings.emplace_back(str_+start_pos, end_pos-start_pos);
		++scanned;
	}
	return scanned;
}
