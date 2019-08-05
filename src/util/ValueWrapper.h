#pragma once

#include "Platform.h"
#include <type_traits>   // for std::is_base_of
#include <utility>       // for std::move

//----------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------
namespace tf
{

//=============================================================================
// Definition of common sets of operators
//=============================================================================
#define TF_EQUALITY_OP_SET  \
    constexpr bool operator == (T oth) const { return value_ == oth.value_; }; \
    constexpr bool operator != (T oth) const { return value_ == oth.value_; };
#define TF_COMPARABLE_OP_SET \
    constexpr bool operator > (T oth) const { return BaseT::value_ > oth.value_; }; \
    constexpr bool operator < (T oth) const{ return BaseT::value_ < oth.value_; }; \
    constexpr bool operator >= (T oth) const{ return BaseT::value_ >= oth.value_; }; \
    constexpr bool operator <= (T oth) const{ return BaseT::value_ <= oth.value_; };
#define TF_ADDABLE_OP_SET \
    T& operator += (T oth) { BaseT::value_+= oth.value_; return (T&)(*this); } \
    T operator + (T oth) const { oth.value_+= BaseT::value_; return oth; } \
    T& operator -= (T oth) { BaseT::value_-= oth.value_; return (T&)(*this); } \
    T operator - (T oth) const { oth.value_-= BaseT::value_; return oth; }
#define TF_MULTIPLICABLE_OP_SET \
    T& operator *= (T oth) { BaseT::value_*= oth.value_; return (T&)(*this); } \
    T operator * (T oth) const { oth.value_*= BaseT::value_; return oth; } \
    T& operator /= (T oth) { BaseT::value_/= oth.value_; return (T&)(*this); } \
    T operator / (T oth) const { T tmp=this; tmp/=oth; return tmp; }
#define TF_INCREMENTAL_OP_SET \
    T operator ++ () { ++BaseT::value_; return (T&)(*this); }; \
    T operator -- () { --BaseT::value_; return (T&)(*this); }; \
    T operator ++ (int) { T tmp=this; ++tmp; return tmp; } \
    T operator -- (int) { T tmp=this; --tmp; return tmp; }
#define TF_NEGATABLE_SET \
    constexpr T operator - () const { return T(-BaseT::value_); }
#define TF_MODABLE_OP_SET \
    T& operator %= (T oth) { BaseT::value_%= oth.value_; return (T&)(*this); } \
    T operator % (T oth) const { T tmp=this; tmp%=oth; return tmp; }
#define TF_BITSET_OP_SET \
    T& operator &= (T oth) { BaseT::value_&= oth.value_; return (T&)(*this); } \
    T operator & (T oth) const { oth.value_&= BaseT::value_; return oth; } \
    T& operator |= (T oth) { BaseT::value_|= oth.value_; return (T&)(*this); } \
    T operator | (T oth) const { oth.value_|= BaseT::value_; return oth; } \
    T& operator ^= (T oth) { BaseT::value_^= oth.value_; return (T&)(*this); } \
    T operator ^ (T oth) const { oth.value_^= BaseT::value_; return oth; } \
    constexpr T operator ~ () const { return T(~BaseT::value_); };

//=============================================================================
// Common Operator Value Wrappers
//=============================================================================

//----------------------------------------------------------------------------
//@FullWraper: used for a value wraper that inherites all traits from its
// wrapped value
//----------------------------------------------------------------------------

template <typename VT>
class FullWrapper
{
  public:

    using WrappedType = VT;
    using WrapperType = FullWrapper<VT>;

    constexpr FullWrapper(VT val = VT{}): value_(val) {}

    // Accessment
    operator VT& () { return value_; }
    operator const VT& () const { return value_; }
    VT* operator -> () { return &value_; }
    const VT* operator -> () const { return &value_; }

  protected:
    VT   value_;
};

//----------------------------------------------------------------------------
//@OPWraperBase: used as a base class for all derived operator wrapper that
// inherits only a selected set of operators from the wrapped value
//----------------------------------------------------------------------------
template <typename VT, typename T>
class OPWrapperBase
{
  public:

    using WrappedType = VT;
    using WrapperType = T;

    constexpr OPWrapperBase(VT val = VT{}): value_(val) {}

    bool isValueWrapper() { return true; }

    //Assignment
    TF_INLINE T& operator = (const T& other) {
        value_ = other.value_; return *this; }
    operator VT& () { return value_; }
    operator const VT& () const { return value_; }

    TF_EQUALITY_OP_SET
  protected:
    VT   value_;
};

template <typename VT, typename T>
class OPCompariable: public OPWrapperBase<VT, T>
{
  public:
    using WrappedType = VT;
    using WrapperType = T;
    using BaseT = OPWrapperBase<VT, T>;

    constexpr OPCompariable(VT val = VT{}): BaseT(val) {}

    // Comparison Ops
    TF_COMPARABLE_OP_SET
};

template <typename VT, typename T>
class OPAddable: public OPCompariable<VT, T>
{
  public:
    using WrappedType = VT;
    using WrapperType = T;
    using BaseT = OPCompariable<VT, T>;

    constexpr OPAddable(VT val = VT{}): BaseT(val) {}

    // Addible OPs
    TF_ADDABLE_OP_SET
};

template <typename VT, typename T>
class OPArithmetic: public OPAddable<VT, T>
{
  public:
    using WrappedType = VT;
    using WrapperType = T;
    using BaseT = OPAddable<VT, T>;

    constexpr OPArithmetic(VT val = VT{}): BaseT(val) {}

    // Multiplicable OPs
    TF_MULTIPLICABLE_OP_SET
};

template <typename VT, typename T>
class OPIncrementable: public OPCompariable<VT, T>
{
  public:
    using WrappedType = VT;
    using WrapperType = T;
    using BaseT = OPCompariable<VT, T>;

    constexpr OPIncrementable(VT val = VT{}): BaseT(val) {}

    TF_INCREMENTAL_OP_SET
};

template <typename VT, typename T>
class OPAddIncrementable: public OPIncrementable<VT, T>
{
  public:
    using WrappedType = VT;
    using WrapperType = T;
    using BaseT = OPIncrementable<VT, T>;

    constexpr OPAddIncrementable(VT val = VT{}): BaseT(val) {}

    TF_ADDABLE_OP_SET
};

template <typename VT, typename T>
class OPUIntArithmetic: public OPArithmetic<VT, T>
{
  public:
    using WrappedType = VT;
    using WrapperType = T;
    using BaseT = OPArithmetic<VT, T>;

    constexpr OPUIntArithmetic(VT val = VT{}): BaseT(val) {}

    TF_INCREMENTAL_OP_SET
    TF_MODABLE_OP_SET
};

template <typename VT, typename T>
class OPIntArithmetic: public OPUIntArithmetic<VT, T>
{
  public:
    using WrappedType = VT;
    using WrapperType = T;
    using BaseT = OPUIntArithmetic<VT, T>;

    constexpr OPIntArithmetic(VT val = VT{}): BaseT(val) {}

    TF_NEGATABLE_SET
};

template <typename VT, typename T>
class OPBitset: public OPWrapperBase<VT, T>
{
  public:
    using WrappedType = VT;
    using WrapperType = T;
    using BaseT = OPWrapperBase<VT, T>;
    constexpr OPBitset(VT val = VT{}): BaseT(val) {}
    TF_BITSET_OP_SET
};

#undef TF_BITSET_OP_SET
#undef TF_MODABLE_OP_SET
#undef TF_INCREMENTAL_OP_SET
#undef TF_MULTIPLICABLE_OP_SET
#undef TF_ADDABLE_OP_SET
#undef TF_COMPARABLE_OP_SET
#undef TF_EQUALITY_OP_SET

} // namespace tf


