#pragma once

#include "Platform.h"
#include "ValueWrapper.h"
#include "StrPool.h"
#include <stdint.h>
#include <unordered_map>
#include <bitset>

namespace tf
{
//----------------------------------------------------------------------------
// Comman helper templates
//----------------------------------------------------------------------------
// Template to check if the first T and all the rest Ts are the same type
// Requires C++17 std::conjunction
template<typename T, typename... Ts>
using all_same_type = std::conjunction<std::is_same<T, Ts>...>;
#define NUMARGS(...)  (sizeof((int[]){__VA_ARGS__})/sizeof(int))

/**
 * \class EnumBase
 * \brief Base of Enum for handling enum names
 */
class EnumBase
{
  protected:
    static int fromString(const char** name_list, const int* nmae_indice, const char* enum_name, size_t enum_number);
  	static void setEnumNames(const char *names_buf, const char**names, size_t enum_num);
};


template<typename EnumT> class EnumType {};

//! Macro to define reflective Enum types
#define ENUM(NAME, UnderlingType, ...) \
class NAME; \
template<> class EnumType<NAME>: \
    public tf::OPIncrementable<UnderlingType,NAME>, public tf::EnumBase \
{ public: \
    enum enum_type: UnderlingType { __VA_ARGS__ }; \
    constexpr EnumType(UnderlingType val): OPIncrementable(val) {} \
}; \
class NAME: public EnumType<NAME> \
{	public: \
  using underlying_type = UnderlingType; \
	constexpr static size_t _enum_number = NUMARGS(__VA_ARGS__); \
  inline static const char* _type_name = #NAME; \
  inline static const char* _names[_enum_number] { nullptr }; \
  inline static int _name_indice[_enum_number] { -1 }; \
  inline static enum_type enum_values [] {__VA_ARGS__}; \
	constexpr static size_t count() { return _enum_number; } \
	constexpr static NAME max() { return NAME(_enum_number-1); } \
	constexpr static NAME invalid() { return NAME(_enum_number); } \
	constexpr static bool isValid(NAME ev) { return ev.value_>=0 && ev.value_<_enum_number; } \
	constexpr static underlying_type toUnderlying(NAME ev) { return ev.value_; } \
	constexpr static NAME fromUnderlying(underlying_type v) { return NAME(v); } \
	constexpr NAME(underlying_type v): EnumType(v){} \
	constexpr NAME(enum_type ev): EnumType(UnderlingType(ev)){} \
	const char* toString() const { init(); return _names[value_]; } \
	static NAME fromString(const char* name) { init(); int res=EnumBase::fromString(_names, _name_indice, name, _enum_number); return res<0?invalid():fromUnderlying(res); } \
  operator enum_type () { return enum_type(value_); } \
  operator const enum_type () const { return enum_type(value_); } \
  template <typename StrOT> friend StrOT& operator << (StrOT& sot, NAME ev) { sot << ev.toString(); return sot; } \
  private: \
  inline static char _names_buf[] { #__VA_ARGS__ }; \
	static void init() { if (!_names[0]) EnumBase::setEnumNames(_names_buf, _names, _enum_number); } \
}

/**
 * \class EnumSet
 * \brief implements a bitset of enum
 * @note The limitation of not being constexpr for constructor comes from
 *       std::bitset whose constructor is not constexpr as it should be.
 *       Using constexpr EnumSet value is not very common so we will stick
 *       with std::bitset.
 */
template<typename ET>
class EnumSet
{
    using bitset_t = std::bitset<sizeof(ET)*CHAR_BIT>;
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

} // namespace tf