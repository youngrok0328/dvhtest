#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
namespace std
{
template<class _Ty>
class allocator;

template<class _Ty, class _Alloc>
class vector;

template<class _Ty, class _Alloc>
class list;

template<class _Ty>
struct less;

template<class _Ty1, class _Ty2>
struct pair;

template<class _Kty, class _Ty, class _Pr, class _Alloc>
class map;
} // namespace std

template<class _Ty>
using StdFwdVector = std::vector<_Ty, std::allocator<_Ty>>;

template<class _Ty>
using StdFwdList = std::list<_Ty, std::allocator<_Ty>>;

template<class _Kty, class _Ty>
using StdFwdMap = std::map<_Kty, _Ty, std::less<_Kty>, std::allocator<std::pair<const _Kty, _Ty>>>;

namespace StdFwd
{
template<class _Ty>
using vector = std::vector<_Ty, std::allocator<_Ty>>;

template<class _Ty>
using list = std::list<_Ty, std::allocator<_Ty>>;

template<class _Kty, class _Ty>
using map = std::map<_Kty, _Ty, std::less<_Kty>, std::allocator<std::pair<const _Kty, _Ty>>>;
} // namespace StdFwd

//HDR_6_________________________________ Header body
//
