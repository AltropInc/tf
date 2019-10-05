#include "StrScan.h"

#include <assert.h>

using namespace tf;

uint64_t StrScan::toUnsigned ()
{
    if (pos_ >= length_)
    {
        return 0;
    }
	char const *p = str_ + pos_;
    if (*p=='0')
    {
        ++p;
        ++pos_;
        if (*p=='x' || *p=='X')
        {
            return toUnsignedFromHex();
        }
        else if (uint64_t(*p - '0') < 8)
        {
            return toUnsignedFromOct();
        }  
    }
    return toUnsignedFromDec();
}

uint64_t StrScan::toUnsignedFromDec ()
{
    uint64_t val = 0;
	uint64_t diff;
    char const *p = str_ + pos_;
    while (pos_< length_ && (diff = uint64_t(*p - '0')) < 10)
    {
      	val = val * 10 + diff;
        ++p;
	    ++pos_;
    }
    return val;
}

uint64_t StrScan::toUnsignedFromHex ()
{
    uint64_t val = 0;
	char const *p = str_ + pos_;
    uint64_t diff;
    while (pos_< length_)
    {
        if ((diff = uint64_t(*p - '0')) < 10)
        {
            val = (val << 4) + diff;
        }
        else if ((diff = uint64_t(*p - 'A')) < 6)
        {
            val = (val << 4 ) + 10 + diff;
        }
        else if ((diff = uint64_t(*p - 'a')) < 6)
        {
            val = (val << 4 ) + 10 + diff;
        }
        else
        {
            break;
        }
        
        ++p;
	    ++pos_;
    }
    return val;
}

uint64_t StrScan::toUnsignedFromOct ()
{
    uint64_t val = 0;
	char const *p = str_ + pos_;
    uint64_t diff;
    while (pos_< length_ && (diff = uint64_t(*p - '0')) < 8)
    {
        val = (val << 3) + diff;
        ++p;
	    ++pos_;
    }
    return val;
}

int64_t StrScan::toInteger ()
{
	char const *p = str_ + pos_;
	if (*p=='-')
    {
        ++pos_;
		return -int64_t(toUnsigned());
    }
	return int64_t(toUnsigned());
}

const std::string& StrScan::toString ()
{
	char const *p = str_ + pos_;
	char const * startp = p;
	while (*p && !isSeparator_[*p] &&
		   (!splitSeparator_ || *p!=splitSeparator_) &&
		   *p!=terminator_)
    {
        ++p;; ++pos_;
    }
	tv_.string_.assign(startp, size_t(p-startp));

    return tv_.string_;
}

StrRef StrScan::toStrRef ()
{
    assert(clearSeparator_);
	char const *p = str_ + pos_;
    char const * startp = p;
	while (*p && !isSeparator_[*p])
    {
        ++p; ++pos_;
    }
    return StrRef(startp);
}

StrRefInLength StrScan::toStrRefInLength ()
{
	char const *p = str_ + pos_;
    char const * startp = p;
	while (*p && !isSeparator_[*p])
    {
        ++p; ++pos_;
    }
    return StrRefInLength(startp, size_t(p-startp));
}

bool StrScan::toBool ()
{
	size_t startp = pos_;
	char const *p = str_ + pos_;
    char startCh = *p;
	while (*p && !isSeparator_[*p])
    {
        ++pos_;
        ++p;
    }

    return startCh=='t' || startCh=='T' || startCh=='1';
}

char StrScan::toChar ()
{
	const char ch = *(str_ + pos_);
	if (ch && !isSeparator_[ch])
    {
        ++pos_;
        return ch;
    }
    return '\0';
}

double StrScan::toDouble()
{
    getNumber();
    return tv_.getDouble();
}

bool StrScan::isBlockStartCh(char ch)
{
	return ch=='[' || ch=='{' || ch=='(' || ch=='<';
}

char StrScan::blockEndCh(char block_start)
{
    switch (block_start)
    {
        case '[': return ']';
        case '{': return '}';
        case '(': return ')';
        case '<': return '>';
    }
	return '\0';
}

StrScan StrScan::newBlockParser(char block_start, char new_separator)
{
    if (block_start)
    {
        // skip block start
        ++pos_;
     }
    StrScan block_parser(str_+pos_, length_-pos_);
    if (new_separator)
    {
        block_parser.setSplitSeparator(new_separator);
    }
	if (block_start)
	{
    	block_parser.setTerminator(blockEndCh(block_start));
    }
    block_parser.block_start_ = block_start;
    return block_parser;
}

void StrScan::releaseBlockParser(StrScan& block_parser)
{
    pos_ += block_parser.pos_;
    if (block_parser.block_start_)
    {
        // skip block end
        ++pos_;
    }
}

size_t StrScan::split (std::vector<std::string>& substrings)
{
	size_t scanned(0);
	size_t start_pos(pos_);
	size_t end_pos(pos_);
	bool string_started (false);
	while (pos_ < length_ && str_[pos_]!=terminator_)
	{
	    if (str_[pos_]==splitSeparator_)
		{
			substrings.emplace_back(str_+start_pos, end_pos-start_pos);
			string_started = false;
			++scanned;
			start_pos = pos_+1;
			end_pos = start_pos;
		}
		else if (isspace(str_[pos_]))
		{
			if (skipLeadingSp_ && !string_started)
			{
				++start_pos;
				++end_pos;
			}
			else if (!skipTrailingSp_  && string_started)
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

void StrScan::skipSeparator()
{
    if (pos_<length_ && isSeparator_[uint8_t(*(str_+pos_))])
    {
        if (clearSeparator_)
        {
            *(const_cast<char*>(str_ + pos_)) = '\0';
        }
        ++pos_;
    }
}

void StrScan::skipSplitSeparator()
{
    if (pos_<length_ && current()==splitSeparator_)
    {
        if (clearSeparator_)
        {
            *(const_cast<char*>(str_ + pos_)) = '\0';
        }
        ++pos_;
    }
}

inline void StrScan::skipWhiteSpace()
{
    char ch = current();
    while (ch && isspace(ch))
    {
        ch = done() ? '\0' : *(str_ + ++pos_);
    }
}

inline char StrScan::nextChar()
{
    skipWhiteSpace();
    return done() ? '\0' : *(str_ + pos_++);
}

void StrScan::getString(std::string& strOut)
{
    tv_.string_.clear();
    char ch = nextChar();
    while (ch && ch!='"')
    {
        if (ch=='\\')
        {
            ch = nextChar();
            switch (ch)
            {
                case '\\': strOut.push_back(ch); break;
                case '"': strOut.push_back(ch); break;
                case 'n': strOut.push_back('\n'); break;
                case 't': strOut.push_back('\t'); break;
                case 'r': strOut.push_back('\r'); break;
                case 'f': strOut.push_back('\f'); break;
                default: strOut.push_back(ch); break;
            }
        }
        tv_.string_.push_back(ch);
        ch = nextChar();
    }
    tv_.token_ = Token::String;
}

void StrScan::getIdentifier(std::string& strOut)
{
    tv_.string_.clear();
    char ch = current();
    while (ch &&
           (ch>='A' && ch<='Z' || ch>='a' && ch<='z' || ch>='0' && ch<='9' || ch=='_')
          )
    {
        tv_.string_.push_back(ch);
        ch = done() ? '\0' : *(str_ + ++pos_);
    }
    tv_.token_ = Token::Identifier;
}

void StrScan::getNumber()
{
    char ch = current();
    tv_.integer_ = 0;
    bool integerGot = false;
    bool isNeg = false;
    if (ch=='-')
    {
        nextChar();
        isNeg = true;
    }

    if (ch>='0' && ch<='9')
    {
        tv_.integer_ = toUnsignedFromDec();
        if (isNeg)
        {
            tv_.integer_ = -tv_.integer_;
        }
        integerGot = true;
        ch = current();
    }
    else if (ch!='.')
    {
        size_t savedPos = pos_;
        getIdentifier(tv_.string_);
        if (tv_.string_=="Infinity" || tv_.string_=="NaN")
        {
            tv_.integer_ = isNeg ? std::numeric_limits<int64_t>::min()
                                 : std::numeric_limits<int64_t>::max();
            tv_.token_ = Token::Integer;
            return;
        }
        pos_ = savedPos;
        tv_.token_ = Token::NegSign;
        return;
    }

    if (ch=='.')
    {
        ch = *(str_ + ++pos_);
        if (ch>='0' && ch<='9')
        {
            size_t old_pos = pos_;
            double decimal = toUnsignedFromDec();
            tv_.double_ = double(tv_.integer_);          
            if (pos_-old_pos<=18)
            {
                tv_.double_ +=  decimal/s_exp10[pos_-old_pos];
            }
            tv_.token_ = Token::Double;
        }
        else if (integerGot)
        {
            tv_.token_ = Token::Double;
        }
        else
        {
            tv_.token_ = Token::DotSign;
        }
		ch = current();
    }
    else
    {
        tv_.token_ = integerGot ? Token::Integer : Token::Unknown;
    }

    if (ch=='E' || ch=='e')
    {
        ch = *(str_ + ++pos_);
        bool isNegExp = ch=='-';
        if (ch=='+' || ch=='-')
        {
            ++pos_;
            unsigned exp = toUnsignedFromDec();
            if (isNegExp)
            {
                if (tv_.token_==Token::Double)
                {
                    tv_.double_ = exp<=18 ? tv_.double_/s_exp10[exp] : 0.0;
                }
                else if (tv_.token_==Token::Integer)
                {
                    tv_.integer_ = exp<=18 ? tv_.integer_/s_exp10[exp] : 0.0;
                }
            }
            else
            {
                if (tv_.token_==Token::Double)
                {
                    tv_.double_ = exp<=18 ? tv_.double_*s_exp10[exp] : 0;
                }
                else if (tv_.token_==Token::Integer)
                {
                    tv_.integer_ = exp<=18 ? tv_.integer_*s_exp10[exp] : std::numeric_limits<int64_t>::max();
                }
            }
            
        }
    }
}

const StrScan::TokenValue* StrScan::getToken()
{
    skipWhiteSpace();
    if (done())
    {
        tv_.token_ = Token::Done;
        return &tv_;
    }
    char ch = current();
    switch (ch)
    {
        case '\0': tv_.token_ = Token::Done; return &tv_;
        case '{': nextChar(); tv_.token_ = Token::LCurly; return &tv_;
        case '}': nextChar(); tv_.token_ = Token::RCurly; return &tv_;
        case '[': nextChar(); tv_.token_ = Token::LBracket; return &tv_;
        case ']': nextChar(); tv_.token_ = Token::RBracket; return &tv_;
        case '(': nextChar(); tv_.token_ = Token::LParenthesis; return &tv_;
        case ')': nextChar(); tv_.token_ = Token::RParenthesis; return &tv_;
        case '=': nextChar(); tv_.token_ = Token::EqualSign; return &tv_;
        case '>': nextChar(); tv_.token_ = Token::GreaterSign; return &tv_;
        case '<': nextChar(); tv_.token_ = Token::LessSign; return &tv_;
        //case '-': nextChar(); tv_.token_ = Token::NegSign; return &tv_;
        case '.': nextChar(); tv_.token_ = Token::DotSign; return &tv_;
        case ':': nextChar(); tv_.token_ = Token::Colon; return &tv_;
        case ';': nextChar(); tv_.token_ = Token::SemiColon; return &tv_;
        case '"': getString(tv_.string_); return &tv_;
    }

    if (ch>='A' && ch<='Z' || ch>='a' && ch<='z' || ch=='_')
    {
        getIdentifier(tv_.string_);
        if (tv_.string_=="Infinity" || tv_.string_=="NaN")
        {
            tv_.integer_ = std::numeric_limits<int64_t>::max();
            tv_.token_ = Token::Integer;
        }
        return &tv_;
    }

    getNumber();

    // If number does not present, skip one char
    if (tv_.token_ == Token::Unknown)
    {
        nextChar();
    }

    return &tv_;
}
