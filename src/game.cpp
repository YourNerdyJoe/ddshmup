#include "platform.h"
#include "game.h"
#include "vec2.h"
#include <numbers>
#include <concepts>
#include <assert.h>
#include <ranges>
#include "counter.h"
#include <span>
#include "render_text.h"

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
	constexpr bool isPressed(int button) const { return isDown(button) && !wasDown(button); }
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

	template<typename T> T& as() {
		static_assert(sizeof(T) <= sizeof(*this));
		return *reinterpret_cast<T*>(this);
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

template <typename T, T... ts>
std::span<T> static_span() {
	static T arr[] = {
		ts...
	};
	return std::span(arr);
}

struct EnemyType {
	vec2 hitbox_half_size;
	u32 hp;
	u32 score_on_hit;
	u32 score_on_kill;
	b8 collide_bullets;
	b8 collide_player;
	b8 is_boss;
	std::span<const EnemyBulletPattern> patterns;
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
		.hp = 20,
		.score_on_hit = 1234,
		.score_on_kill = 5678,
		.collide_bullets = true,
		.is_boss = true,
		.patterns = static_span<const EnemyBulletPattern, EnemyBulletPattern{
			.counters = {
				.maxs = {30, 300, 120},
				.is_looping = {true, true, true},
			},
			.user_func = { boss1_routine2 },
		}>(),
	},
};

template<typename T, T* data>
struct Index {
	u32 idx;

	constexpr T& operator*() const { return data[idx]; }
	constexpr T* operator->() const { return &data[idx]; }
};

struct Enemy {
	Index<const EnemyType, enemy_types> type;
	vec2 pos;
	u32 damage;
	u16 time_alive;
	u16 time_delay;
	u16 counter_values[NUM_ENEMY_COUNTERS];
	u8 curr_pattern_idx;
	UserData<4> user_data;

	constexpr const EnemyBulletPattern& pattern() const {
		return type->patterns[curr_pattern_idx];
	}

	constexpr CounterMatrixView<NUM_ENEMY_COUNTERS> counters() {
		return {pattern().counters, counter_values};
	}
};

struct Bullet;

struct BulletType {
	u32 sprite_id;
};

static const BulletType bullet_types[] = {
	{},
};

struct Bullet {
	struct {
		u32 inactive : 1;
	} flags;
	Index<const BulletType, bullet_types> type;
	Bullet* next_in_group;
	vec2 pos;
	vec2 vel;
	u8 speed_mod_func;
	u8 cancelled;
	u16 counter_values[4];
	CounterMatrix<4> counter_matrix;
	void (*user_func[2])(Bullet& self, GameState& gs);
	UserData<4> user_data;

	constexpr CounterMatrixView<4> counters() {
		return {counter_matrix, counter_values};
	}

	constexpr void cancel() {
		constexpr u8 CANCEL_ANIMATION_TIME = 60;
		cancelled = CANCEL_ANIMATION_TIME;
	}
};

constexpr f32 PLAYER_RADIUS = 10.f;
constexpr f32 PLAYER_RADIUS_SQUARED = PLAYER_RADIUS * PLAYER_RADIUS;
constexpr f32 PLAYER_GRAZE_RADIUS = 50.f;
constexpr f32 PLAYER_GRAZE_RADIUS_SQUARED = PLAYER_GRAZE_RADIUS * PLAYER_GRAZE_RADIUS;

constexpr size_t MAX_ENEMIES = 64;
constexpr size_t MAX_BULLETS = 640;

template <typename T, size_t N> requires std::is_trivial_v<T>
struct DynArr {
	u32 size;
	T data[N];

	constexpr auto begin(this auto& self) { return self.data; }
	constexpr auto end(this auto& self) { return self.data + self.size; }

	//constexpr auto& front(this auto& self) { return *self.begin(); }
	//constexpr auto& back(this auto& self) { return self.end()[-1]; }

	constexpr T& add() { return data[size++]; }
	constexpr void remove(u32 idx) { data[idx] = data[--size]; }
	constexpr void remove(T* ptr) { remove((u32)(ptr - data)); }
	constexpr void clear() { size = 0; }

	constexpr bool contains_ptr(const T* ptr) const { return ptr >= begin() && ptr < end(); }
	constexpr bool full() const { return size == N; }
};

struct ScorePopup {
	vec2 pos;
	u32 val;
	u32 life;
};

constexpr u32 STARTING_BOMBS = 3;
constexpr u32 STARTING_LIVES = 2;

struct GameState {
	struct {
		vec2 pos;
	} player;

	DynArr<Enemy, MAX_ENEMIES> enemies;
	DynArr<Bullet, MAX_BULLETS> bullets;
	Bullet* next_free_bullet;
	DynArr<vec2, 100> player_bullets;
	DynArr<ScorePopup, MAX_ENEMIES + MAX_BULLETS> score_popups;
	u64 score;
	u16 player_shot_timer;
	u16 bombs;
	u16 lives;
	u16 is_bombing;
	u16 prox_multiplier;
	u16 graze_counter;
	u16 graze_reset_timer;
	u16 invuln;

	inline auto active_bullets(this auto& self) {
		return self.bullets | std::views::filter([](const Bullet& b){ return !b.flags.inactive; });
	}

	inline auto active_bullets_reverse(this auto& self) {
		return self.bullets | std::views::reverse | std::views::filter([](const Bullet& b){ return !b.flags.inactive; });
	}

	inline void add_bullet(const Bullet& b) {
		if(next_free_bullet) {
			Bullet* next = next_free_bullet->next_in_group;
			*next_free_bullet = b;
			next_free_bullet = next;
		}
		else {
			bullets.add() = b;
		}
	}

	inline void remove_bullet(Bullet& b) {
		assert(bullets.contains_ptr(&b));
		b.flags.inactive = true;
		b.next_in_group = next_free_bullet;
		next_free_bullet = &b;
	}

	inline void add_score(vec2 pos, u32 amt) {
		if(amt) {
			amt *= prox_multiplier;
			score += amt;
			if(!score_popups.full()) {
				score_popups.add() = {
					.pos = pos,
					.val = amt,
					.life = 60,
				};
			}
		}
	}

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
		gs.add_bullet(templ);
		templ.pos = self.pos + vec2{32.f, 0.f};
		gs.add_bullet(templ);
	}

	if(self.counters()[2]) {
		vec2 dir = normalize_safe(gs.player.pos - self.pos);
		if(dir == vec2{0.f}) dir = {0.f, 1.f};
		gs.add_bullet({
			.pos = self.pos,
			.vel = dir * bullet_speed,
		});
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
			gs.add_bullet({
				.pos = self.pos + vec2{32.f, 0.f},
				.vel = rad2vec(angle) * bullet_speed,
			});
			gs.add_bullet({
				.pos = self.pos + vec2{-32.f, 0.f},
				.vel = rad2vec(deg2rad(180.f) - angle) * bullet_speed,
			});
		}
	}
}

void GameState::update(VPad pad) {
	vec2 vel = pad.dir() * 4.f;
	if(pad.isDown(B_C)) vel *= 0.5f;
	player.pos += vel;

	for(auto& s : score_popups | std::views::reverse) {
		s.pos.y -= 1.f;
		if(s.life) s.life--;
		else score_popups.remove(&s);
	}

	for(auto& enemy : enemies) {
		enemy.counters().update();
		for(auto f : enemy.pattern().user_func) {
			if(f) f(enemy, *this);
		}

		if(enemy.type->is_boss) {
			f32 dist[4] = {100.f, 150.f, 200.f, 250.f};
			prox_multiplier = 5;
			for(u16 i = 0; i < 4; i++) {
				if(distancesq(player.pos, enemy.pos) < dist[i]*dist[i]) {
					break;
				}
				prox_multiplier--;
			}
		}
	}

	//bomb
	if(!is_bombing && pad.isPressed(B_B)) {
		if(bombs) {
			bombs--;
			constexpr u32 BOMB_DURATION = 60;
			is_bombing = BOMB_DURATION;
			for(auto& enemy : enemies) {
				constexpr u32 BOMB_DAMAGE = 10;
				enemy.damage += BOMB_DAMAGE;
			}
		}
	}
	if(is_bombing) {
		is_bombing--;
		for(auto& bullet : active_bullets()) {
			if(!bullet.cancelled) {
				bullet.cancel();
				add_score(bullet.pos, graze_counter);
			}
		}
	}

	//player shoot
	if(player_shot_timer) {
		player_shot_timer--;
	}
	else if(pad.isDown(B_A) || pad.isDown(B_C)) {
		constexpr u32 PLAYER_SHOT_TIMER_RESET = 6;
		player_shot_timer = PLAYER_SHOT_TIMER_RESET;
		player_bullets.add() = player.pos;
	}

	//hit enemies
	for(auto& bullet : player_bullets | std::views::reverse) {
		bullet.y -= 10;
		if(!rectContains({200, 240}, {220, 260}, bullet)) {
			player_bullets.remove(&bullet);
			continue;
		}
		for(auto& enemy : enemies) {
			if(enemy.type->collide_bullets
			&& rectContains(enemy.pos, enemy.type->hitbox_half_size, bullet)) {
				enemy.damage++;
				add_score(enemy.pos, enemy.type->score_on_hit);
				//TODO: player bullet hit particle effect
				player_bullets.remove(&bullet);
				break;
			}
		}
	}

	//kill damaged enemies
	for(auto& enemy : enemies | std::views::reverse) {
		if(enemy.type->hp && enemy.damage >= enemy.type->hp) {
			add_score(enemy.pos, enemy.type->score_on_kill);
			//TODO: enemy death anim
			enemies.remove(&enemy);
		}
	}

	for(auto& bullet : active_bullets_reverse()) {
		if(!bullet.cancelled) {
			bullet.counters().update();
			for(auto f : bullet.user_func) {
				if(f) f(bullet, *this);
			}
		}
		bullet.pos += bullet.vel * ease(bullet.counters()[0], bullet.speed_mod_func);
		if(!bullet.cancelled) {
			if(!rectContains({200, 240}, {220, 260}, bullet.pos)) {
				remove_bullet(bullet);	//out of bounds
			}
			else if(!invuln && distancesq(bullet.pos, player.pos) <= PLAYER_RADIUS_SQUARED) {
				//hit
				//remove_bullet(bullet);
				if(lives) {
					lives--;
					invuln = 150;
					bombs = STARTING_BOMBS;
				}
				//else //TODO: lose
			}
			else if(distancesq(bullet.pos, player.pos) <= PLAYER_GRAZE_RADIUS_SQUARED) {
				if(graze_counter < 9999) graze_counter++;
				graze_reset_timer = 200;
			}
		}
		else if(!--bullet.cancelled) {
			remove_bullet(bullet);
		}
	}

	if(invuln) invuln--;

	if(graze_reset_timer) {
		graze_reset_timer--;
	}
	else if(graze_counter) {
		graze_counter--;
	}
}

struct Rect {
	int x, y, w, h;
	constexpr vec2 pos() const { return {(f32)x, (f32)y}; }
	constexpr vec2 dim() const { return {(f32)w, (f32)h}; }
};

enum {
	SPR_PLAYER,
	SPR_PLAYER_BULLET,
	SPR_BULLET,
	SPR_BULLET_CANCEL,
	SPR_ENEMY,
	SPR_ICO_BOMB,
	SPR_ICO_LIFE,
};

static const Rect sprite_data[] = {
	{0, 0, 48, 64},
	{0, 64, 32, 16},
	{48, 0, 16, 16},
	{48, 16, 16, 16},
	{64, 0, 48, 48},
	{0, 112, 16, 16},
	{16, 112, 16, 16},
};

void draw_sprite(u32 id, vec2 pos) {
	Rect spr = sprite_data[id];
	vec2 origin = spr.dim() / 2.f;
	pos -= origin;
	platform_draw_sprite(pos.x, pos.y, spr.x, spr.y, spr.w, spr.h);
}

void GameState::render() const {
	for(auto& enemy : enemies) {
		draw_sprite(SPR_ENEMY, enemy.pos);
	}
	for(auto& bullet : player_bullets) {
		draw_sprite(SPR_PLAYER_BULLET, bullet);
	}
	draw_sprite(SPR_PLAYER, player.pos);
	for(auto& bullet : active_bullets()) {
		if(bullet.cancelled) draw_sprite(SPR_BULLET_CANCEL, bullet.pos); //TODO: cancel animate
		else draw_sprite(SPR_BULLET, bullet.pos);
	}
	for(auto& s : score_popups) {
		renderTextFmt((ivec2)s.pos, "%d", s.val);
	}

	//ui
	for(int i = 0; i < 480; i += 32) {
		platform_draw_sprite(640.f - 240.f, (f32)i, 0, 80, 120, 32);
		platform_draw_sprite(640.f - 120.f, (f32)i, 0, 80, 120, 32);
	}
	renderTextFmt({420, 16}, "%012llu", score);
	for(u32 i = 0; i < bombs; i++) {
		draw_sprite(SPR_ICO_BOMB, {420.f+i*16, 64});
	}
	for(u32 i = 0; i < lives; i++) {
		draw_sprite(SPR_ICO_LIFE, {420.f+i*16, 48});
	}
	renderTextFmt({420, 96}, "prox: %d", (int)prox_multiplier);
	renderTextFmt({420, 112}, "graze: %d (%d)", (int)graze_counter, (int)graze_reset_timer);
}

DLL_EXPORT GameState* game_create(size_t* size) {
	*size = sizeof(GameState);
	auto ret = (GameState*)malloc(sizeof(GameState));
	*ret = {
		.player = {
			.pos = {250.f, 400.f},
		},
		.bombs = STARTING_BOMBS,
		.lives = STARTING_LIVES,
	};
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
