#pragma once
#include "types.h"

struct CounterView {
	const u16& max;
	u16& value;

	constexpr bool isMax() const { return value >= max; }
	constexpr operator bool() const { return isMax(); }
	constexpr operator float() const { return (float)value / (float)max; }

	constexpr bool update() { if(!max || value < max) value++; return isMax(); }
	constexpr bool update_looping() { if(++value > max) value = 0; return isMax(); }
};

struct CounterArray {
	const u16* maxs;
	u16* values;
	constexpr CounterView operator[](size_t i) const {
		return { maxs[i], values[i] };
	}
};

template<size_t N>
struct CounterMatrix {
	u16 maxs[N];
	b8 is_looping[N];
	u8 input[N];

	void update(u16* values) const {
		CounterArray counters = {maxs, values};
		for(size_t i = 0; i < N; i++) {
			if(!input[i] || (counters[input[i]-1])) {
				if(is_looping[i]) counters[i].update_looping();
				else counters[i].update();
			}
		}
	}
};

template<size_t N>
struct CounterMatrixView {
	const CounterMatrix<N>& matrix;
	u16* values;

	inline void update() const { matrix.update(values); }

	constexpr CounterView operator[](size_t i) const {
		return { matrix.maxs[i], values[i] };
	}
};
