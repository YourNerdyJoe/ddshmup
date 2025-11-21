#pragma once
#include "types.h"
#include <utility>

template<typename T> struct PlaceholderStorage {
	u8 alignas(T) data[sizeof(T)];
};

//just for trivially copiable types atm
template<typename T, size_t N> class inplace_vector {
	size_t size_;
	//PlaceholderStorage<T> data_[N];
	T data_[N];

public:
	//constexpr T* data() { return reinterpret_cast<T*>(data_); }
	//constexpr const T* data() const { return reinterpret_cast<const T*>(data_); }
	constexpr auto data(this auto& self) { return self.data_ + 0; }
	constexpr size_t size() const { return size_; }
	constexpr bool empty() const { return size() != 0; }
	constexpr size_t capacity() const { return N; }

	constexpr auto begin(this auto& self) { return self.data(); }
	constexpr auto end(this auto& self) { return self.data() + self.size(); }

	constexpr auto& front(this auto& self) { return *self.begin(); }
	constexpr auto& back(this auto& self) { return self.end()[-1]; }

	constexpr auto& operator[](this auto& self, size_t i) { return data()[i]; }

	constexpr void* push_back_noinit() { return data_ + size_++; }
	constexpr T& push_back(const T& value) { return new (push_back_noinit()) T(value); }
	//constexpr T& push_back(T&& value) { return new (push_back_noinit()) T(std::move(value)); }

	//constexpr void pop_back() { back()->~T(); size_--; }
	constexpr void pop_back() { size_--; }
	constexpr void clear() { size_ = 0; }

	//constexpr inplace_vector() : size_{0} {};
};
