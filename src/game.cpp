#include "platform.h"
#include "game.h"
#include "vec2.h"
#include <numbers>
//#include "inplace_vector.h"

#ifndef _arch_dreamcast
#define DLL_EXPORT extern "C" __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

struct alignas(u32) VPad {
	VPadFrame curr;
	VPadFrame prev;

	constexpr bool isDown(int button) const { return curr.buttons & (1 << button); }
	constexpr bool wasDown(int button) const { return prev.buttons & (1 << button); }
	constexpr bool isPressed(int button) const { return isDown(button) && wasDown(button); }
	constexpr bool isReleased(int button) const { return !isDown(button) && wasDown(button); }

	constexpr ivec2 idir() const {
		int x = isDown(B_RIGHT) - isDown(B_LEFT);
		int y = isDown(B_DOWN) - isDown(B_UP);
		return {x, y};
	}

	constexpr vec2 dir() const {
		ivec2 iret = idir();
		vec2 ret = vec2(iret);
		if(iret.x && iret.y) ret *= std::numbers::sqrt2_v<float> * 0.5f; //1/sqrt(2) == sqrt(2)/2
		return ret;
	}
};

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

enum {
	EASE_CONSTANT,
	EASE_LINEAR,
	EASE_IN_QUAD,
	EASE_OUT_QUAD,
	//EASE_IN_OUT_QUAD,

	EASE_FUNC_MASK = (1 << 6) - 1,
	EASE_FLAG_PING_PONG = (1 << 6),
	EASE_FLAG_INVERSE = (1 << 7),
};

constexpr f32 ease(f32 val, u32 func) {
	f32 ret;
	if(func & EASE_FLAG_PING_PONG) val = 1.f - abs(val * 2.f - 1.f);
	switch(func & EASE_FUNC_MASK) {
	case EASE_CONSTANT:     ret = 1.f; break;
	case EASE_LINEAR:       ret = val; break;
	case EASE_IN_QUAD:      ret = val * val; break;
	case EASE_OUT_QUAD:     ret = 1 - (1 - val) * (1 - val); break;
	//case EASE_IN_OUT_QUAD:  ret = ; break;
	default: ret = 0;
	}
	return (func & EASE_FLAG_INVERSE) ? 1.f - ret : ret;
}

template<typename T>
constexpr auto lerp(T begin, T end, f32 amt) {
	return begin + (end - begin) * amt;
	//return begin * (1.f - amt) + end * (amt);
}

constexpr f32 deg2rad(f32 deg) {
	return deg * std::numbers::pi_v<f32> / 180.f;
}

//make constexpr in c++26
inline vec2 rad2vec(f32 rad) {
	return { cosf(rad), sinf(rad) };
}

struct GameState;

template<size_t N>
struct UserData {
	u32 placeholder[N];

	template<typename T> T* as() {
		static_assert(sizeof(T) <= sizeof(*this));
		return reinterpret_cast<T*>(this);
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

bool rectContains(vec2 center, vec2 half_dim, vec2 target) {
	return target.x >= center.x - half_dim.x
		&& target.x < center.x + half_dim.x
		&& target.y >= center.y - half_dim.y
		&& target.y < center.y + half_dim.y;
}

constexpr size_t NUM_ENEMY_COUNTERS = 4;

struct Enemy;

struct EnemyBulletPattern {
	CounterMatrix<4> counters;
	void (*user_func[2])(Enemy& self, GameState& gs);
};

struct EnemyType {
	vec2 hitbox_half_size;
	b8 collide_bullets;
	b8 collide_player;
	EnemyBulletPattern patterns[4];
};

enum {
	ENEMY_BOSS1,

	NUM_ENEMY_TYPES,
};

void boss1_routine(Enemy& self, GameState& gs);
void boss1_routine2(Enemy& self, GameState& gs);

static const EnemyType enemy_types[] = {
	{	//ENEMY_BOSS1
		.hitbox_half_size = { 16, 16 },
		.collide_bullets = true,
		.patterns = {{
			.counters = {
				.maxs = {30, 300, 120},
				.is_looping = {true, true, true},
			},
			.user_func = { boss1_routine2 },
		}},
	},
};

template<typename T, T* data>
struct Index {
	u32 idx;
	constexpr T& get() const { return data[idx]; }
	constexpr T& operator*() const { return get(); }
	constexpr T* operator->() const { return &get(); }
};

struct Enemy {
	Index<const EnemyType, enemy_types> type;
	vec2 pos;
	u16 time_alive;
	u16 time_delay;
	u16 counter_values[NUM_ENEMY_COUNTERS];
	u8 curr_pattern_idx;
	UserData<4> user_data;

	constexpr const EnemyBulletPattern& pattern() const {
		return type->patterns[curr_pattern_idx];
	}

	constexpr CounterMatrixView<NUM_ENEMY_COUNTERS> counters(this auto& self) {
		return {self.pattern().counters, self.counter_values};
	}
};

enum BulletFlags {
	B_ACTIVE = (1 << 0),
};

struct Bullet;

struct BulletType {
	CounterMatrix<4> counters;
	u8 speed_mod_func;
	void (*user_func[2])(Bullet& self, GameState& gs);
};

static const BulletType bullet_types[] = {
	{},
};

struct Bullet {
	Index<const BulletType, bullet_types> type;
	vec2 pos;
	vec2 vel;
	u16 counter_values[4];
	UserData<4> user_data;

	constexpr CounterMatrixView<4> counters(this auto& self) {
		return {self.type->counters, self.counter_values};
	}
};

struct FreeNode {
	u32 flags;
	u32 next;
};

constexpr f32 PLAYER_RADIUS = 10.f;
constexpr f32 PLAYER_RADIUS_SQUARED = PLAYER_RADIUS * PLAYER_RADIUS;

constexpr size_t MAX_ENEMIES = 64;
constexpr size_t MAX_BULLETS = 640;

template <typename T, size_t N>
struct DynArr {
	u32 size;
	T data[N];

	constexpr auto begin(this auto& self) { return self.data + N - self.size; }
	constexpr auto end(this auto& self) { return self.data + N; }

	constexpr auto& front(this auto& self) { return *self.begin(); }
	constexpr auto& back(this auto& self) { return self.end()[-1]; }

	constexpr T& add() { return data[N - ++size]; }
	constexpr void remove(u32 idx) { data[idx] = data[N - size--]; }
	constexpr void remove(T* ptr) { remove((u32)(ptr - data)); }
};

struct GameState {
	struct {
		vec2 pos;
	} player;

	DynArr<Enemy, MAX_ENEMIES> enemies;
	DynArr<Bullet, MAX_BULLETS> bullets;

	void update(VPad pad);
	void render() const;
};

void boss1_routine(Enemy& self, GameState& gs) {
	if(!self.counters()[3]) return;
	constexpr f32 bullet_speed = 6.f;
	if(self.counters()[0]) {
		f32 angle = lerp(
			deg2rad(80.f),
			deg2rad(100.f),
			ease(self.counters()[1], EASE_LINEAR | EASE_FLAG_PING_PONG)
		);
		Bullet templ = {
			.vel = rad2vec(angle) * bullet_speed,
		};

		templ.pos = self.pos + vec2{-32.f, 0.f};
		gs.bullets.add() = templ;
		templ.pos = self.pos + vec2{32.f, 0.f};
		gs.bullets.add() = templ;
	}

	if(self.counters()[2]) {
		vec2 dir = normalize_safe(gs.player.pos - self.pos);
		if(dir == vec2{0.f}) dir = {0.f, 1.f};
		gs.bullets.add() = {
			.pos = self.pos,
			.vel = dir * bullet_speed,
		};
	}
}

void boss1_routine2(Enemy& self, GameState& gs) {
	if(!self.counters()[3]) return;
	constexpr f32 bullet_speed = 6.f;
	if(self.counters()[0]) {
		f32 angles[3];
		angles[0] = lerp(
			deg2rad(40.f),
			deg2rad(90.f),
			ease(self.counters()[1], EASE_LINEAR | EASE_FLAG_PING_PONG)
		);
		angles[1] = angles[0] - deg2rad(15.f);
		angles[2] = angles[0] + deg2rad(15.f);

		for(f32 angle : angles) {
			gs.bullets.add() = {
				.pos = self.pos + vec2{32.f, 0.f},
				.vel = rad2vec(angle) * bullet_speed,
			};
			gs.bullets.add() = {
				.pos = self.pos + vec2{-32.f, 0.f},
				.vel = rad2vec(deg2rad(180.f) - angle) * bullet_speed,
			};
		}
	}
}

void GameState::update(VPad pad) {
	vec2 vel = pad.dir() * 4.f;
	if(pad.isDown(B_C)) vel *= 0.5f;
	player.pos += vel;

	for(auto& enemy : enemies) {
		enemy.counters().update();
		for(auto f : enemy.pattern().user_func) {
			if(f) f(enemy, *this);
		}
	}

	for(auto& bullet : bullets) {
		bullet.counters().update();
		for(auto f : bullet.type->user_func) {
			if(f) f(bullet, *this);
		}
		bullet.pos += bullet.vel * ease(bullet.counters()[0], bullet.type->speed_mod_func);
		if(distancesq(bullet.pos, player.pos) <= PLAYER_RADIUS_SQUARED
		|| !rectContains({320, 240}, {300, 200}, bullet.pos)) {
			//hit
			bullets.remove(&bullet);
		}
	}
}

void GameState::render() const {
	for(auto& enemy : enemies) {
		platform_draw_sprite(1, enemy.pos.x, enemy.pos.y);
	}
	platform_draw_sprite(0, player.pos.x, player.pos.y);
	for(auto& bullet : bullets) {
		platform_draw_sprite(2, bullet.pos.x, bullet.pos.y);
	}
}

DLL_EXPORT GameState* game_create(size_t* size) {
	*size = sizeof(GameState);
	auto ret = (GameState*)malloc(sizeof(GameState));
	*ret = {};
	ret->enemies.add() = {
		.type = {ENEMY_BOSS1},
		.pos = {250.f, 80.f},
	};

	//ret->enemies.add() = {.pos = {450.f, 50.f}, .counters={{{.max = 210}}, {}, {true}}};
	//ret->enemies.add() = {.pos = {450.f, 450.f}, .counters={{{.max = 220}}, {}, {true}}};
	//ret->enemies.add() = {.pos = {50.f, 450.f}, .counters={{{.max = 230}}, {}, {true}}};
	return ret;
}

DLL_EXPORT void game_update(GameState* gs, VPadFrame curr, VPadFrame prev) {
	gs->update({curr, prev});
}

DLL_EXPORT void game_render(const GameState* gs) {
	gs->render();
}
