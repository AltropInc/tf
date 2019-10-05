/**
 * @file StrScan.h
 * @brief Defines StrScan Class, a part of tf core
 * @author David Shang
 * @version 1.00
 * $Id: $
 */

#pragma once

#include "Platform.h"
#include "StrBuffer.h"
#include "EnumSet.h"
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
    StrScan(StrScan&& oth) = default;
    StrScan(const StrScan& oth) = default;
	
    const char* c_str() const { return str_; }
    size_t length() const { return length_; }
    bool empty() const { return str_==nullptr; }
    bool done() const { return pos_>=length_; }
    char current () const { return *(str_+pos_); }
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
	
    StrScan& operator >> (int8_t& n) { n=int8_t(toInteger()); skipSeparator(); return *this; };
    StrScan& operator >> (int16_t& n) { n=int16_t(toInteger()); skipSeparator(); return *this; };
    StrScan& operator >> (int32_t& n) { n=int32_t(toInteger()); skipSeparator(); return *this; };
    StrScan& operator >> (int64_t& n) { n=int64_t(toInteger()); skipSeparator(); return *this; };
    StrScan& operator >> (uint8_t& n) { n=uint8_t(toUnsigned()); skipSeparator(); return *this; };
    StrScan& operator >> (uint16_t& n) { n=uint16_t(toUnsigned()); skipSeparator(); return *this; };
    StrScan& operator >> (uint32_t& n) { n=uint32_t(toUnsigned()); skipSeparator(); return *this; };
    StrScan& operator >> (uint64_t& n) { n=uint64_t(toUnsigned()); skipSeparator(); return *this; };
    StrScan& operator >> (double& n) { n=toDouble(); skipSeparator(); return *this; };
    StrScan& operator >> (bool& n) { n=toBool(); skipSeparator(); return *this; };
    StrScan& operator >> (std::string& n) { n=toString(); skipSeparator(); return *this; };
    StrScan& operator >> (StrRef& n) { n=toStrRef(); skipSeparator(); return *this; };
    StrScan& operator >> (StrRefInLength& n) { n=toStrRefInLength(); skipSeparator(); return *this; };
    StrScan& operator >> (std::vector<std::string>& strings)
    {  split(strings); skipSeparator(); return *this;  }

    // For any type that has static method fromStr
    template <typename T>
    StrScan& operator >> (T& value)
    {   value=T::fromStr(toString().c_str()); skipSeparator(); return *this;  }

    // For EnumSet using refective Enum type only. Not work for standard c++ enum
    template <typename T>
    StrScan& operator >> (EnumSet<T>& value);

    // Create and release a block parser for a block using different format
    StrScan newBlockParser(char blockStart=0, char newSeparator=0);
    void releaseBlockParser(StrScan& blockParser);

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
        char ch = current();
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
    char nextChar();
    void skipWhiteSpace();
    void getString(std::string& strOut);
    void getIdentifier(std::string& strOut);
    void getNumber();
    void skipSeparator();
    void skipSplitSeparator();

    static bool isBlockStartCh(char ch);
    static char blockEndCh(char start_ch);

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

    char block_start_ {'\0'};
};

template <typename T>
StrScan& StrScan::operator >> (EnumSet<T>& value)
{
    char start_ch = current();
    char terminator = getTerminator();
    if (isBlockStartCh(start_ch))
    {
        setTerminator(blockEndCh(start_ch));
        nextChar(); // skip block start_ch
    }
    while (!atValueEnd())
    {
        T ev = toType<T>();
        if (ev!=T::INVALID) value.set(ev);
        skipSplitSeparator();
    }
    if (isBlockStartCh(start_ch))
    {
        setTerminator(terminator); 
        nextChar(); // skip blockEndCh
    }
    skipSeparator();
    return *this;
}


}
