#pragma once
#include <algorithm>
#include <math.h>
#include "types.h"

template<typename T>
struct Vec2 {
	T x, y;

	constexpr Vec2() = default;
	constexpr Vec2(T x, T y) : x(x), y(y) {}
	constexpr Vec2(T fill) : Vec2(fill, fill) {}

	template<typename U>
	explicit constexpr Vec2(Vec2<U> v) : x((T)v.x), y((T)v.y) {}

	template<typename U> constexpr explicit operator Vec2<U>() const { return { (U)x, (U)y }; }
};

using vec2 = Vec2<f32>;
using dvec2 = Vec2<f64>;
using ivec2 = Vec2<s32>;
using uvec2 = Vec2<u32>;
using bvec2 = Vec2<b8>;

template<typename T> constexpr Vec2<T> operator+(Vec2<T> a, Vec2<T> b) { return {a.x + b.x, a.y + b.y}; }
template<typename T> constexpr Vec2<T> operator-(Vec2<T> a, Vec2<T> b) { return {a.x - b.x, a.y - b.y}; }
template<typename T> constexpr Vec2<T> operator-(Vec2<T> a) { return {-a.x, -a.y}; }
template<typename T> constexpr Vec2<T> operator*(Vec2<T> a, Vec2<T> b) { return {a.x * b.x, a.y * b.y}; }
template<typename T> constexpr Vec2<T> operator/(Vec2<T> a, Vec2<T> b) { return {a.x / b.x, a.y / b.y}; }
template<typename T> constexpr Vec2<T> operator*(Vec2<T> a, T b) { return {a.x * b, a.y * b}; }
template<typename T> constexpr Vec2<T> operator/(Vec2<T> a, T b) { return {a.x / b, a.y / b}; }

constexpr ivec2 operator<<(ivec2 a, ivec2 b) { return {a.x << b.x, a.y << b.y}; }
constexpr ivec2 operator>>(ivec2 a, ivec2 b) { return {a.x >> b.x, a.y >> b.y}; }
constexpr uvec2 operator<<(uvec2 a, uvec2 b) { return {a.x << b.x, a.y << b.y}; }
constexpr uvec2 operator>>(uvec2 a, uvec2 b) { return {a.x >> b.x, a.y >> b.y}; }

template<typename T> constexpr Vec2<T>& operator+=(Vec2<T>& a, Vec2<T> b) { return a = a + b; }
template<typename T> constexpr Vec2<T>& operator-=(Vec2<T>& a, Vec2<T> b) { return a = a - b; }
template<typename T> constexpr Vec2<T>& operator*=(Vec2<T>& a, Vec2<T> b) { return a = a * b; }
template<typename T> constexpr Vec2<T>& operator/=(Vec2<T>& a, Vec2<T> b) { return a = a / b; }
template<typename T> constexpr Vec2<T>& operator*=(Vec2<T>& a, T b) { return a = a * b; }
template<typename T> constexpr Vec2<T>& operator/=(Vec2<T>& a, T b) { return a = a / b; }

constexpr ivec2& operator<<=(ivec2& a, ivec2 b) { return a = a << b; }
constexpr uvec2& operator<<=(uvec2& a, uvec2 b) { return a = a << b; }

constexpr ivec2& operator>>=(ivec2& a, ivec2 b) { return a = a >> b; }
constexpr uvec2& operator>>=(uvec2& a, uvec2 b) { return a = a >> b; }

template<typename T> constexpr bool operator==(Vec2<T> a, Vec2<T> b) { return a.x == b.x && a.y == b.y; }

template<typename T> constexpr T dot(Vec2<T> a, Vec2<T> b) { return a.x * b.x + a.y * b.y; }

template<typename T> constexpr T lengthsq(Vec2<T> v) { return dot(v, v); }
template<typename T> inline    T length(Vec2<T> v)   { return (T)sqrt(lengthsq(v)); }
template<typename T> constexpr T distancesq(Vec2<T> a, Vec2<T> b) { return lengthsq(a - b); }
template<typename T> inline    T distance(Vec2<T> a, Vec2<T> b)   { return (T)sqrt(distancesq(a, b)); }
inline vec2 normalize(vec2 v) { return v / length(v); }
inline dvec2 normalize(dvec2 v) { return v / length(v); }
inline vec2 normalize_safe(vec2 v) { f32 len = length(v); return (len > 0.f) ? (v / length(v)) : 0.f; }
inline dvec2 normalize_safe(dvec2 v) { f64 len = length(v); return (len > 0.0) ? (v / length(v)) : 0.0; }

template<typename T> constexpr T min(Vec2<T> a, Vec2<T> b) { return { std::min(a.x, b.x), std::min(a.y, b.y)}; }
template<typename T> constexpr T min(Vec2<T> a, T b)       { return { std::min(a.x, b), std::min(a.y, b)}; }
template<typename T> constexpr T max(Vec2<T> a, Vec2<T> b) { return { std::max(a.x, b.x), std::max(a.y, b.y)}; }
template<typename T> constexpr T max(Vec2<T> a, T b)       { return { std::max(a.x, b), std::max(a.y, b)}; }
template<typename T> constexpr T clamp(Vec2<T> a, Vec2<T> minVal, Vec2<T> maxVal)
	{ return { std::clamp(a.x, minVal.x, maxVal.y), std::clamp(a.y, minVal.y, maxVal.y)}; }
template<typename T> constexpr T clamp(Vec2<T> a, T minVal, T maxVal)
	{ return { std::clamp(a.x, minVal, maxVal), std::clamp(a.y, minVal, maxVal)}; }

template<typename T> requires std::floating_point<T>
constexpr Vec2<T> mix(Vec2<T> a, Vec2<T> b, Vec2<T> wb) { return a * (Vec2<T>{ (T)1.0, (T)1.0 } - wb) + b * wb; }
template<typename T> requires std::floating_point<T>
constexpr Vec2<T> mix(Vec2<T> a, Vec2<T> b, T wb) { return a * ((T)1.0 - wb) + b * wb; }
