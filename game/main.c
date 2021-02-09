#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "game.h"
#include "geom.h"

#define RGB_INT_RED rgb_mask(255, 0, 0)
#define RGB_INT_GREEN rgb_mask(0, 255, 0)
#define RGB_INT_BLUE rgb_mask(0, 0, 255)
#define RGB_INT_WHITE rgb_mask(255, 255, 255)

void init();
void close();
void updateMobMovement(mob_t* mob);
void drawMob(mob_t* mob);
void fire(mob_t* shooter);
int main(int argc, char* args[]);
void spawnMob();
void drawPlayer(mob_t* mob);
void drawEnemy(mob_t* mob);
void drawProj(mob_t* mob);
int randIntRangeIncl(int min, int max_incl);
int randIntRangeExcl(int min, int max_excl);
double randDblRangeIncl(double min, double max_incl);
bool isInRange(double val, double lower, double upper);
void killPlayer(mob_t* player);
void gameOver();

const int SCREEN_WIDTH = 1920 / 2;
const int SCREEN_HEIGHT = 1080 / 2;
SDL_Window* window;
SDL_Renderer* renderer;

const int OFFSCREEN_MARGIN = 200;

// TODO make player struct with mob struct as member
int lives = 3;
int score = 0;

// player movement constants
// TODO implement for each mob struct
const double max_accel = 0.125;
const double max_rot_accel = 0.005;
const double max_rot_vel = 0.05;
const double max_speed = 15;

const double player_accel = 0.05; // 0.1
const double player_rot_accel = 0.001; // 0.001

int id_ctr = 1;

mob_list_node_t* mob_list;
bool quit;

int main(int argc, char* args[]) {
	init();

	mob_t player = { 0 };
	player.id = 0;
	player.MOB_TYPE = MOB_TYPE_PLAYER;
	player.position.x = SCREEN_WIDTH / 2;
	player.position.y = SCREEN_HEIGHT / 2;
	player.rollover = true;
	player.color = rgb_mask(100, 60, 255);

	mob_list = malloc(sizeof(mob_list_node_t));
	mob_list->value = NULL;
	mob_list->next = NULL;

	quit = false;
	SDL_Event e;

	// main loop
	while (!quit) {
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);

		// event loop
		while (SDL_PollEvent(&e) != 0) {
#pragma region held keys
			const Uint8* keystate = SDL_GetKeyboardState(NULL);

			// modify player accel
			if (keystate[SDL_SCANCODE_W]) {
				vec2d_t fwd = get_fwd_vector(&player);
				player.accel.x += fwd.x * player_accel;
				player.accel.y += fwd.y * player_accel;
			}
			else {
				player.accel.x = 0;
				player.accel.y = 0;
			}

			if (keystate[SDL_SCANCODE_A] && !keystate[SDL_SCANCODE_D]) {
				player.rot_accel -= player_rot_accel;
			}
			else if (keystate[SDL_SCANCODE_D] && !keystate[SDL_SCANCODE_A]) {
				player.rot_accel += player_rot_accel;
			}
			else {
				player.rot_accel = 0;
			}

			if (e.type == SDL_QUIT) {
				quit = true;
			}
#pragma endregion

			// single press keys
			else if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
				switch (e.key.keysym.sym) {
				case SDLK_q:
					quit = true;
					break;
				case SDLK_e:
					// reset
					player.accel.x = 0;
					player.accel.y = 0;
					player.rot_accel = 0;

					player.velocity.x = 0;
					player.velocity.y = 0;
					player.rot_vel = 0;

					player.position.x = SCREEN_WIDTH / 2;
					player.position.y = SCREEN_HEIGHT / 2;
					player.rotation = 0;
					break;
				case SDLK_SPACE:
					fire(&player);
					break;
				default:
					break;
				}
			}
		}

		updateMobMovement(&player);
		drawMob(&player);

		for (int i = 0; i < mob_list_size(mob_list); i++) {
			mob_t* mob = mob_list_get_index(mob_list, i);

			updateMobMovement(mob);

			bool outside =
				mob->position.x < 0.0 - (double)OFFSCREEN_MARGIN ||
				mob->position.x >(double)SCREEN_WIDTH + (double)OFFSCREEN_MARGIN ||
				mob->position.y < 0.0 - (double)OFFSCREEN_MARGIN ||
				mob->position.y >(double)SCREEN_HEIGHT + (double)OFFSCREEN_MARGIN;

			// destroy mob if outside screen
			if (outside || mob->isHit) {
				mob_list_node_t** mob_list_ptr = &mob_list;

				// remove from list and adjust index
				mob_list_remove_index(mob_list_ptr, i);
				i--;

				mob = NULL;
				free(mob);
			}
			else {
				drawMob(mob);
			}
		}

		if ((rand() % 1000) < 100) {
			spawnMob(mob_list);
		}

		SDL_RenderPresent(renderer);
		SDL_Delay(16); // ~60fps
	}

	gameOver();

	close();
}

void spawnMob() {
	mob_t* mob = malloc(sizeof(mob_t));

	// position must be off screen, but within margin
	bool top = randIntRangeIncl(0, 1);
	bool right = randIntRangeIncl(0, 1);

	if (right) {
		mob->position.x = 0 - randDblRangeIncl(50, OFFSCREEN_MARGIN);
	}
	else {
		mob->position.x = (double)SCREEN_WIDTH + randDblRangeIncl(50, OFFSCREEN_MARGIN);
	}

	if (top) {
		mob->position.y = 0 - randDblRangeIncl(50, SCREEN_HEIGHT);
	}
	else {
		mob->position.y = (double)SCREEN_HEIGHT + randDblRangeIncl(50, OFFSCREEN_MARGIN);
	}

	// velocity must be towards screen
	int magicnumber = 2;

	if (right) {
		mob->velocity.x = randIntRangeIncl(0, magicnumber);
	}
	else {
		mob->velocity.x = -1 * randDblRangeIncl(0, magicnumber);
	}

	if (top) {
		mob->velocity.y = randIntRangeIncl(0, magicnumber);
	}
	else {
		mob->velocity.y = -1 * randDblRangeIncl(0, magicnumber);
	}

	mob->rotation = rand() % 314;
	mob->rot_vel = (rand() % 50) * 0.0005;
	mob->rot_accel = 0;
	mob->accel.x = 0;
	mob->accel.y = 0;
	mob->rollover = false;
	mob->MOB_TYPE = MOB_TYPE_ENEMY;
	mob->id = id_ctr++;
	mob->color = rgb_mask(rand() % 256, rand() % 256, rand() % 256);
	mob->isHit = false;

	mob_list_push(mob_list, mob);
}

void fire(mob_t* shooter) {
	vec2d_t fwd = get_fwd_vector(shooter);
	double speed = 20;
	double offset = 25.0;

	mob_t* proj = malloc(sizeof(mob_t));
	proj->position.x = shooter->position.x + fwd.x * offset;
	proj->position.y = shooter->position.y + fwd.y * offset;
	proj->velocity.x = fwd.x * speed;
	proj->velocity.y = fwd.y * speed;
	proj->rotation = shooter->rotation;
	proj->rot_vel = 0;
	proj->rot_accel = 0;
	proj->accel.x = 0;
	proj->accel.y = 0;
	proj->rollover = false;
	proj->MOB_TYPE = MOB_TYPE_PROJECTILE;
	proj->id = id_ctr++;
	proj->color = rgb_mask(rand() % 256, rand() % 256, rand() % 256);
	proj->isHit = false;

	mob_list_push(mob_list, proj);
}

void updateMobMovement(mob_t* mob) {
	// clamp accel
	clamp(&mob->accel.x, max_accel * -1, max_accel);
	clamp(&mob->accel.y, max_accel * -1, max_accel);
	clamp(&mob->rot_accel, max_rot_accel * -1, max_rot_accel);

	// modify velocity
	mob->velocity.x += mob->accel.x;
	mob->velocity.y += mob->accel.y;
	mob->rot_vel += mob->rot_accel;

	// clamp velocity
	clamp(&mob->rot_vel, max_rot_vel * -1, max_rot_vel);
	clamp_vector_magnitude(&mob->velocity, max_speed);

	// modify position
	mob->position.x += mob->velocity.x;
	mob->position.y += mob->velocity.y;
	mob->rotation += mob->rot_vel;

	// rollover at screen edges
	if (mob->rollover) {
		if (mob->position.x < 0) {
			mob->position.x = (double)SCREEN_WIDTH - 1;
		}
		else if (mob->position.x >= SCREEN_WIDTH) {
			mob->position.x = 0;
		}

		if (mob->position.y < 0) {
			mob->position.y = (double)SCREEN_HEIGHT - 1;
		}
		else if (mob->position.y >= SCREEN_HEIGHT) {
			mob->position.y = 0;
		}
	}

	// check if player is hit by enemy
	if (mob->MOB_TYPE == MOB_TYPE_PLAYER) {
		for (int i = 0; i < mob_list_size(mob_list); i++) {
			mob_t* other_mob = mob_list_get_index(mob_list, i);

			double hit_radius = 30;

			if (other_mob->MOB_TYPE == MOB_TYPE_ENEMY) {
				if (isInRange(mob->position.x, other_mob->position.x - hit_radius, other_mob->position.x + hit_radius) &&
					isInRange(mob->position.y, other_mob->position.y - hit_radius, other_mob->position.y + hit_radius)) {
					killPlayer(mob);
				}
			}
		}
	}

	// check if enemy is hit by projectile
	// TODO each mob needs a list of points we can check if another mob is inside of
	if (mob->MOB_TYPE == MOB_TYPE_ENEMY) {
		for (int i = 0; i < mob_list_size(mob_list); i++) {
			mob_t* other_mob = mob_list_get_index(mob_list, i);

			double hit_radius = 30;

			if (other_mob->MOB_TYPE == MOB_TYPE_PROJECTILE) {
				if (isInRange(mob->position.x, other_mob->position.x - hit_radius, other_mob->position.x + hit_radius) &&
					isInRange(mob->position.y, other_mob->position.y - hit_radius, other_mob->position.y + hit_radius)) {
					mob->isHit = true;
					other_mob->isHit = true;
					score++;
				}
			}
		}
	}
}

void setHighScore(int index, int newScore) {
	char string[256];

	printf("High score!\n");
	printf("Enter name: ");
	gets(string);

	FILE* file;
	errno_t err;

	err = fopen_s(&file, "high_scores.txt", "w+");

	fputs("name1\n", file);
	fputs("0\n", file);
	fputs("name2\n", file);
	fputs("0\n", file);
	fputs("name3\n", file);
	fputs("0\n", file);

	fclose(file);
}

void gameOver() {
	printf("game over. score: %d\n", score);

	high_scores_t scores = getHighScores();

	if (score > scores.score1) {
		setHighScore(0, score);
	}
	else if (score > scores.score2) {
		setHighScore(1, score);
	}
	else if (score > scores.score3) {
		setHighScore(2, score);
	}
}

void killPlayer(mob_t* player) {
	lives--;

	printf("player dead. %d lives left.\n", lives);

	player->accel.x = 0;
	player->accel.y = 0;
	player->rot_accel = 0;

	player->velocity.x = 0;
	player->velocity.y = 0;
	player->rot_vel = 0;

	player->position.x = SCREEN_WIDTH / 2;
	player->position.y = SCREEN_HEIGHT / 2;
	player->rotation = 0;

	mob_list_node_t** mob_list_ptr = &mob_list;
	for (int i = 0; i < mob_list_size(mob_list); i++) {
		mob_t* mob = mob_list_get_index(mob_list, i);
		// remove from list and adjust index
		mob_list_remove_index(mob_list_ptr, i);
		i--;

		mob = NULL;
		free(mob);
	}

	if (lives == 0) {
		quit = true;
	}
}

void drawPlayer(mob_t* mob) {
	double nose_length = 25.0;
	double angle = 155.0;
	double side_length = 40.0;

	vec2d_t fwd = get_fwd_vector(mob);

	vec2d_t center_pt = {
		mob->position.x,
		mob->position.y
	};

	vec2d_t nose_pt = {
		center_pt.x + fwd.x * nose_length,
		center_pt.y + fwd.y * nose_length
	};

	vec2d_t temp_vec = fwd;
	rotate_vector(&temp_vec, deg_to_rad(angle));
	scale_vector(&temp_vec, side_length);

	vec2d_t back_right_pt = {
		nose_pt.x + temp_vec.x,
		nose_pt.y + temp_vec.y
	};

	temp_vec = fwd;
	rotate_vector(&temp_vec, deg_to_rad(-1 * angle));
	scale_vector(&temp_vec, side_length);

	vec2d_t back_left_pt = {
		nose_pt.x + temp_vec.x,
		nose_pt.y + temp_vec.y
	};

	// center to nose
	//SDL_RenderDrawLine(renderer,
	//	(int)round(center_pt.x),
	//	(int)round(center_pt.y),
	//	(int)round(nose_pt.x),
	//	(int)round(nose_pt.y));

	// nose to back right
	SDL_RenderDrawLine(renderer,
		(int)round(nose_pt.x),
		(int)round(nose_pt.y),
		(int)round(back_right_pt.x),
		(int)round(back_right_pt.y));

	// nose to back left
	SDL_RenderDrawLine(renderer,
		(int)round(nose_pt.x),
		(int)round(nose_pt.y),
		(int)round(back_left_pt.x),
		(int)round(back_left_pt.y));

	// back right to center
	SDL_RenderDrawLine(renderer,
		(int)round(back_right_pt.x),
		(int)round(back_right_pt.y),
		(int)round(center_pt.x),
		(int)round(center_pt.y));

	// back left to center
	SDL_RenderDrawLine(renderer,
		(int)round(back_left_pt.x),
		(int)round(back_left_pt.y),
		(int)round(center_pt.x),
		(int)round(center_pt.y));
}

void drawEnemy(mob_t* mob) {
	vec2d_t fwd = get_fwd_vector(mob);

	double side_lenth = 40;
	double center_to_corner = side_lenth / 1.41;

	vec2d_t center_pt = {
		mob->position.x,
		mob->position.y
	};

	vec2d_t last_pt = { 0 };

	rotate_vector(&fwd, deg_to_rad(45));

	for (int i = 0; i < 5; i++) {
		rotate_vector(&fwd, deg_to_rad(90));

		vec2d_t corner_pt = {
			center_pt.x + fwd.x * center_to_corner,
			center_pt.y + fwd.y * center_to_corner
		};

		if (i > 0) {
			SDL_RenderDrawLine(renderer,
				(int)round(last_pt.x),
				(int)round(last_pt.y),
				(int)round(corner_pt.x),
				(int)round(corner_pt.y));
		}

		last_pt = corner_pt;
	}
}

void drawProj(mob_t* mob) {
	double length = 15.0;

	vec2d_t fwd = get_fwd_vector(mob);

	vec2d_t center_pt = {
		mob->position.x,
		mob->position.y
	};

	vec2d_t nose_pt = {
		center_pt.x + fwd.x * length / 2,
		center_pt.y + fwd.y * length / 2
	};

	vec2d_t back_pt = {
		center_pt.x - fwd.x * length / 2,
		center_pt.y - fwd.y * length / 2
	};

	SDL_RenderDrawLine(renderer,
		(int)round(nose_pt.x),
		(int)round(nose_pt.y),
		(int)round(back_pt.x),
		(int)round(back_pt.y));
}

void drawMob(mob_t* mob) {
	Uint8 r = 0;
	Uint8 g = 0;
	Uint8 b = 0;
	rgb_unmask(mob->color, &r, &g, &b);

	SDL_SetRenderDrawColor(renderer, r, g, b, 255);

	if (mob->MOB_TYPE == MOB_TYPE_PLAYER) {
		drawPlayer(mob);
	}
	else if (mob->MOB_TYPE == MOB_TYPE_ENEMY) {
		drawEnemy(mob);
	}
	else if (mob->MOB_TYPE == MOB_TYPE_PROJECTILE) {
		drawProj(mob);
	}
}

void init() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL init error: %s\n", SDL_GetError());
		exit(1);
	}

	window = SDL_CreateWindow("sdl2 game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

	if (window == NULL) {
		printf("SDL window error: %s\n", SDL_GetError());
		exit(2);
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	if (renderer == NULL) {
		printf("SDL renderer error: %s\n", SDL_GetError());
		exit(3);
	}

	srand((unsigned int)time(NULL));
}

void close() {
	SDL_DestroyRenderer(renderer);
	renderer = NULL;

	SDL_DestroyWindow(window);
	window = NULL;

	SDL_Quit();
}
