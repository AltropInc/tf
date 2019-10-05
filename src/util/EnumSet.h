#pragma once

#include "Enum.h"
#include <bitset>

namespace tf
{

// Helper to convert startdard enum or non-startdard enum value to size_t
struct EnumToSizeT
{
    template <class ET>
    typename std::enable_if<std::is_enum<ET>::value,size_t>::type
    static get(ET e)
    {
        return static_cast<typename std::underlying_type<ET>::type>(e);
    }

    template <class ET>
    typename std::enable_if<!std::is_enum<ET>::value,size_t>::type
    static get(ET e)
    { 
        return ET::toUnderlying(e);
    }
};

// A simple replacement of std::bitset. Number of bits limitation is 64.
// And this limitation fits our current usage.
template <typename T>
struct BitSet
{
    T   value_ { 0UL} ;
    BitSet() = default;
    BitSet(const BitSet& oth): value_(oth.value_) {}
    BitSet(T val): value_(val) {}
    BitSet& set (size_t v) { value_ |= (T(1) << v); return *this; }
    BitSet& set () { value_ = ~(0UL); return *this; }
    BitSet& reset (size_t v) { value_ &= ~(T(1) << v); return *this; }
    BitSet& reset () { value_ = 0UL;  return *this; }
    bool test (size_t v) const { return (value_ | (T(1) << v))!=0; }
    BitSet& flip (size_t v) { value_ ^ (T(1) << v); return *this; }
    BitSet& flip () { value_ = ~value_; return *this; }
    bool empty() const { return value_==0; }
    bool any() const { return value_!=0; }
    size_t count() const { return __builtin_popcountl(value_); }
    size_t size() const { return __builtin_popcountl(value_); }
    BitSet& operator |= (BitSet oth) { value_|= oth.value_; *this; }
    BitSet& operator &= (BitSet oth) { value_&= oth.value_; *this; }
    uint64_t to_ullong() const { return uint64_t(value_); }
    std::string to_string () const
    {
        std::string output;
        for (size_t e = 0; e < sizeof(T)*8; ++e)
        {
            output.push_back(((T(1) << e) & value_) ? '1' : '0');
        }
        return output;
    }
};

/**
 * \class EnumSet
 * \brief implements a bitset of enum
 * @note The limitation of not being constexpr for constructor comes from
 *       std::bitset whose constructor is not constexpr as it should be.
 *       Using constexpr EnumSet value is not very common so we will stick
 *       with std::bitset.
 */
template<typename ET, typename BT=std::bitset<sizeof(ET)*CHAR_BIT>>
class EnumSet
{
    using bitset_t = BT;
    bitset_t bitset_;

  public:
    EnumSet() = default;
    explicit EnumSet(ET e) { bitset_.set(ET::toUnderlying(e)); }
    EnumSet(const EnumSet &es) : bitset_(es.bitset_) {}

    EnumSet &operator=(const EnumSet &es) {bitset_ = es.bitset_; return *this;}

    bool operator==(const EnumSet &es) const { return bitset_ == es.bitset_; }
    bool operator!=(const EnumSet &es) const { return bitset_ != es.bitset_; }

    void set(ET e) { bitset_.set(size_t(ET::toUnderlying(e))); }
    EnumSet &set(ET e, bool value)
    { bitset_.set(size_t(ET::toUnderlying(e)), value); return *this; }
    EnumSet& set() const { return bitset_.set(); }

    void unset(ET e) { bitset_.reset(size_t(ET::toUnderlying(e))); }
    void clear() { bitset_.reset(); }
    EnumSet& reset() { bitset_.reset(); return *this; }
    EnumSet& reset(ET e) { bitset_.reset(size_t(ET::toUnderlying(e))); return *this; }
  
    void toggle(ET e) { bitset_.flip(size_t(ET::toUnderlying(e))); }
    EnumSet&  flip(ET e) { bitset_.flip(size_t(ET::toUnderlying(e))); return *this; }
    EnumSet& flip() const { return bitset_.flip(); }

    bool has(ET e) const { return bitset_.test(size_t(ET::toUnderlying(e))); }
 
    explicit operator bool() const { return bitset_.any(); }
    size_t size() const { return bitset_.size(); }
    size_t count() const { return bitset_.count(); }
 
    EnumSet &operator|=(ET e) { set(e); return *this; }
    EnumSet &operator&=(ET e)
    { const bool v=test(e); clear(); return set(e, v); }

    EnumSet &operator|=(const EnumSet &es)
    { bitset_ |= es.bitset_; return *this; }
    EnumSet &operator&=(const EnumSet &es)
    { bitset_ &= es.bitset_; return *this; }

    friend EnumSet &operator | (EnumSet es1, const EnumSet &es2)
    { es1 |= es2; return es1; }
    friend EnumSet &operator & (EnumSet es1, const EnumSet &es2)
    { es1 &= es2; return es1; }

    EnumSet operator&(ET e) { EnumSet tmp(*this); return tmp&=e; }
    EnumSet operator|(ET e) { EnumSet tmp(*this); return tmp|=e; }

    bool operator[](ET e) const { return bitset_[size_t(ET::toUnderlying(e))]; }
   
    ///Contructor from multiple underlying enum type values
    ///@Usage: EnumSet<EnumType>(EnumValue1, EnumValue3, EnumValue5)
    template<typename... ETS>
             //class = std::enable_if_t<all_same_type<ET, ETS...>::value, void>>
    EnumSet(ET e, ETS... others)
    {
        bitset_.set(size_t(ET::toUnderlying(e)));
        *this |= EnumSet(others...);
    }

    /// Returns string in binary bitset format
    std::string toStringRaw() const { return bitset_.to_string(); }

    /// Returns string in a set of enum names
    std::string toString() const
    {
        std::string str = "(";
        for (auto e: ET::enum_values)
        {
            if (has(ET(e)))
            {
                if (str.length() > 1) str += ",";
                str += ET(e).toString();
            }
        }
        str += ")";
        return str;
    }

    friend std::ostream& operator<<(std::ostream& stream, const EnumSet& es)
    { return stream << es.toString(); }
};

template <typename ET> using EnumSet8  = EnumSet<ET, BitSet<uint8_t>>;
template <typename ET> using EnumSet16 = EnumSet<ET, BitSet<uint16_t>>;
template <typename ET> using EnumSet32 = EnumSet<ET, BitSet<uint32_t>>;
template <typename ET> using EnumSet64 = EnumSet<ET, BitSet<uint64_t>>;

} // namespace tf
