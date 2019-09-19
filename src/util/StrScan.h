/**
 * @file StrScan.h
 * @brief Defines StrScan Class, a part of tf core
 * @author David Shang
 * @version 1.00
 * $Id: $
 */

#pragma once
#include "Platform.h"
#include <stddef.h>
#include <cstring>
#include <string>
#include <vector>

namespace tf {
/**
 * \class StrScan
 * \ingroup core
 * \brief String scan helper converting string into values in various formats
 */
class StrScan
{
public:
    StrScan(const char* str, size_t length) : str_(str), length_(length) {}
    StrScan(const char* begin, const char* end) : str_(begin), length_(size_t(end-begin)) {}
    explicit StrScan(const std::string& str) : str_(str.c_str()), length_(str.length()) {}
    explicit StrScan(const char* str) : str_(str), length_(strlen(str)) {}

    const char* c_str() const { return str_; }
    size_t length() const { return length_; }
    bool empty() const { return str_==nullptr; }
    bool done() const { return length_==pos_; }
    char current () const { return *(str+pos_); }
    size_t pos () const { return pos_; }
	
    uint64_t toUnsigned ();
    int64_t toInteger ();
    uint64_t toUnsignedFromDec ();
    uint64_t toUnsignedFromHex ();
    uint64_t toUnsignedFromOct ();
    double toDouble ();
    bool toBool ();
    char toChar ();
    const std::string& toString ();
    StrRef toStrRef ();
    StrRefInLength toStrRefInLength ();
	
    // For any type that has static method fromStr
    template <typename T>
    T toType () { return T::fromStr(toString().c_str()); }

    size_t split(std::vector<std::string>& substrings);
	
    StrScan& operator >> (int8_t& n) { n=toInteger<int8_t>(); return *this; };
    StrScan& operator >> (int16_t& n) { n=toInteger<int16_t>(); return *this; };
    StrScan& operator >> (int32_t& n) { n=toInteger<int32_t>(); return *this; };
    StrScan& operator >> (int64_t& n) { n=toInteger<int64_t>(); return *this; };
    StrScan& operator >> (uint8_t& n) { n=toInteger<uint8_t>(); return *this; };
    StrScan& operator >> (uint16_t& n) { n=toInteger<uint16_t>(); return *this; };
    StrScan& operator >> (uint32_t& n) { n=toInteger<uint32_t>(); return *this; };
    StrScan& operator >> (uint64_t& n) { n=toInteger<uint64_t>(); return *this; };
    StrParser& operator >> (double& n) { n=toDouble(); skipSeparator(); return *this; };
    StrParser& operator >> (bool& n) { n=toBool(); skipSeparator(); return *this; };
    StrParser& operator >> (std::string& n) { n=toString(); skipSeparator(); return *this; };
    StrParser& operator >> (StrRef& n) { n=toStrRef(); skipSeparator(); return *this; };
    StrParser& operator >> (StrRefInLength& n) { n=toStrRefInLength(); skipSeparator(); return *this; };
    StrParser& operator >> (std::vector<std::string>& strings)
    {  split(strings); skipSeparator(); return *this;  }

    // For any type that has static method fromStr
    template <typename T>
    StrParser& operator >> (T& value)
    {   value=T::fromStr(toString().c_str()); skipSeparator(); return *this;  }
    // For EnumSet using refective Enum type only. Not work for standard c++ enum
    template <typename T>
    StrParser& operator >> (EnumSet<T>& value)
    {
        while (!atValueEnd())
        {
            T ev = toType<T>();
            if (ev!=T::INVALID)
            {
                value.set(ev);
            }
            skipSplitSeparator();
        }
        skipSeparator();
        return *this;
    }

    // Create and release a block parser for a block using different format
    StrParser newBlockParser(char blockStart=0, char newSeparator=0);
    void releaseBlockParser(StrParser& blockParser);

    void setTerminator(char ch) { terminator_ = ch; }
    char getTerminator() const { return terminator_; }
    void setSplitSeparator(char ch) { splitSeparator_ = ch; }
    char getSplitSeparator() const { return splitSeparator_; }
    void setSkipLeadingSp(bool ch) { skipLeadingSp_ = ch; }
    void setSkipTrailingSp(bool ch) { skipTrailingSp_ = ch; }
    bool getSkipLeadingSp() const { return skipLeadingSp_; }
    bool getSkipTrailingSp() const { return skipTrailingSp_; }

    void setClearSeparator(bool clear) { clearSeparator_ = clear; }
    bool getClearSeparator() const { return clearSeparator_; }

    void addSeparator(char ch) { isSeparator_[uint8_t(ch)] = true; }
    void remSeparator(char ch) { isSeparator_[uint8_t(ch)] = false; }

    bool atValueEnd()
    {
        char ch = curChar();
        return pos_>=length_ || !ch || isSeparator_[uint8_t(ch)] ||
               ch==terminator_;
    }

    // A simple token parser of parsing jason, xml etc.
    enum class Token
    {
        LCurly,
        RCurly,
        LBracket,
        RBracket,
        LParenthesis,
        RParenthesis,
        Comma,
        Colon,
        SemiColon,
        EqualSign,
        GreaterSign,
        LessSign,
        NegSign,
        DotSign,
        Identifier,
        String,
        Char,
        Integer,
        Double,
        Done,
        Unknown
    };

    struct TokenValue
    {
        Token               token_;
        std::string         string_;
        int64_t             integer_;
        double              double_;

        bool isNumber() const { return token_ == Token::Double || token_ == Token::Integer; }

        double getDouble() const
        {
            return token_ == Token::Double ? double_ :
                             token_ == Token::Integer ? static_cast<double>(integer_) : 0.0;      
        }
        int64_t getInteger() const
        {
            return token_ == Token::Integer ? integer_ :
                          token_ == Token::Double ? static_cast<int64_t>(double_) : int64_t(0);      
        }
    };

    const TokenValue* getToken();
	
private:
    char curChar();
    char nextChar();
    void skipWhiteSpace();
    void getString(std::string& strOut);
    void getIdentifier(std::string& strOut);
    void getNumber();
    void skipSeparator();
    void skipSplitSeparator();

    const char* str_ { nullptr };
	size_t length_ { 0 };
	size_t pos_ { 0 };
     
    // Parer options
    char terminator_ {'\0'};

    // When set, separator will be replaced by 0 after parsing
    bool clearSeparator_ {false};

    // Options in split
    char splitSeparator_ {','};
 	bool skipLeadingSp_ { false };
	bool skipTrailingSp_ { false };

    std::vector<bool> isSeparator_ { std::vector<bool>(256, false) };
    TokenValue  tv_;

    char blockStart_ {'\0'};
};

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

const std::string& StrParser::toString ()
{
	char const *p = str_ + pos_;
	char const * startp = p;
	while (*p && !isSeparator_[*p] && (!splitSeparator_ || *p!=splitSeparator_))
    {
        ++p;; ++pos_;
    }
	tv_.string_.assign(startp, size_t(p-startp));

    return tv_.string_;
}

StrRef StrParser::toStrRef ()
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

StrRefInLength StrParser::toStrRefInLength ()
{
	char const *p = str_ + pos_;
    char const * startp = p;
	while (*p && !isSeparator_[*p])
    {
        ++p; ++pos_;
    }
    return StrRefInLength(startp, size_t(p-startp));
}

bool StrParser::toBool ()
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

char StrParser::toChar ()
{
	const char ch = *(str_ + pos_);
	if (ch && !isSeparator_[ch])
    {
        ++pos_;
        return ch;
    }
    return '\0';
}

double StrParser::toDouble()
{
    getNumber();
    return tv_.getDouble();
}

StrParser StrParser::newBlockParser(char blockStart, char newSeparator)
{
    if (blockStart)
    {
        // skip block start
        ++pos_;
     }
    StrParser blockParser(str_+pos_, length_-pos_);
    if (newSeparator)
    {
        blockParser.setSplitSeparator(newSeparator);
    }
    switch (blockStart)
    {
        case '[': blockParser.setTerminator(']'); break;
        case '{': blockParser.setTerminator('}'); break;
        case '(': blockParser.setTerminator(')'); break;
        case '<': blockParser.setTerminator('>'); break;
    }
    blockParser.blockStart_ = blockStart;
    return blockParser;
}

void StrParser::releaseBlockParser(StrParser& blockParser)
{
    pos_ += blockParser.pos_;
    if (blockParser.blockStart_)
    {
        // skip block end
        ++pos_;
    }
}

size_t StrParser::split (std::vector<std::string>& substrings)
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

inline char StrParser::curChar()
{
    return *(str_ + pos_);
}

void StrParser::skipSeparator()
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

void StrParser::skipSplitSeparator()
{
    if (pos_<length_ && curChar()==splitSeparator_)
    {
        if (clearSeparator_)
        {
            *(const_cast<char*>(str_ + pos_)) = '\0';
        }
        ++pos_;
    }
}

inline void StrParser::skipWhiteSpace()
{
    char ch = curChar();
    while (ch && isspace(ch))
    {
        ch = nextChar();
    }
}

inline char StrParser::nextChar()
{
    skipWhiteSpace();
    return done() ? '\0' : *(str_ + pos_++);
}

void StrParser::getString(std::string& strOut)
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

void StrParser::getIdentifier(std::string& strOut)
{
    tv_.string_.clear();
    char ch = curChar();
    while (ch &&
           (ch>='A' && ch<='Z' || ch>='a' && ch<='z' || ch>='0' && ch<='9' || ch=='_')
          )
    {
        tv_.string_.push_back(ch);
        ch = nextChar();
    }
    tv_.token_ = Token::Identifier;
}

void StrParser::getNumber()
{
    char ch = curChar();
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
            
            if (pos_-old_pos<=18)
            {
                tv_.double_ +=  decimal/s_exp10[pos_-old_pos+1];
            }
            tv_.token_ = Token::Double;
        }
        else if (integerGot)
        {
            tv_.token_ = Token::Double;
        }
        tv_.token_ = Token::DotSign;
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

const StrParser::TokenValue* StrParser::getToken()
{
    if (done())
    {
        tv_.token_ = Token::Done;
        return &tv_;
    }
    char ch = curChar();
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

}
