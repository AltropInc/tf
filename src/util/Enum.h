#pragma once

#include "Platform.h"
#include "ValueWrapper.h"
#include <stdint.h>
#include <unordered_map>

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

} // namespace tf
