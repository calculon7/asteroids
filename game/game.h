#pragma once
#include <math.h>
#include <string.h>
#include <stdlib.h>

typedef struct vec2d {
	double x;
	double y;
} vec2d_t;

typedef enum MOB_TYPE {
	MOB_TYPE_PLAYER = 0,
	MOB_TYPE_ENEMY = 1,
	MOB_TYPE_PROJECTILE = 2,
} MOB_TYPE_t;

typedef struct mob {
	int id;
	MOB_TYPE_t MOB_TYPE;
	vec2d_t position;   // m
	vec2d_t velocity;	// m/s
	vec2d_t accel;		// m/s^2
	double rotation;	// rads
	double rot_vel;		// rads/s
	double rot_accel;	// rads/s^2
	bool rollover;
	int color; // masked rgb
	bool isHit;
} mob_t;

vec2d_t get_fwd_vector(mob_t* mob) {
	vec2d_t fwd;
	fwd.x = cos(mob->rotation);
	fwd.y = sin(mob->rotation);
	return fwd;
}

typedef struct mob_list_node {
	mob_t* value;
	struct mob_list_node* next;
} mob_list_node_t;

void mob_list_push(mob_list_node_t* head, mob_t* mob) {
	mob_list_node_t* current = head;

	if (current->value == NULL) {
		current->value = mob;
		current->next = NULL;
	}
	else {
		// go to last item in list
  		while (current->next != NULL) {
			current = current->next;
		}

		current->next = malloc(sizeof(mob_list_node_t));
		current->next->value = mob;
		current->next->next = NULL;
	}
}

void mob_list_remove_index(mob_list_node_t** head, int idx) {
	// if only one value in list, set value to NULL
	if ((*head)->next == NULL) {
		(*head)->value = NULL;
	}
	else if (idx == 0) {
		mob_list_node_t* orig_head = *head;

		*head = (*head)->next;
 	}
	else {
		mob_list_node_t* current = *head;

		// go to node before idx
		for (int i = 0; i < idx - 1; i++) {
			current = current->next;
		}

		mob_list_node_t* node_at_idx = current->next;

		current->next = current->next->next;

		free(node_at_idx);
	}
 }

int mob_list_size(mob_list_node_t* head) {
	mob_list_node_t* current = head;

	if (current->value == NULL) {
		return 0;
	}
	else {
		int count = 1;

 		while (current->next != NULL) {
			current = current->next;
			count++;
		}

		return count;
	}
}

mob_t* mob_list_get_index(mob_list_node_t* head, int idx) {
	mob_list_node_t* current = head;

	for (int i = 0; i < idx; i++) {
		current = current->next;
	}

	return current->value;
}

int rgb_mask(Uint8 r, Uint8 g, Uint8 b) {
	int rgb = r;
	rgb <<= 8;

	rgb |= g;
	rgb <<= 8;

	rgb |= b;
	
	return rgb;
}

void rgb_unmask(int rgb, Uint8* r, Uint8* g, Uint8* b) {
	*r = rgb >> 16;
	*g = (rgb >> 8) & 0b11111111;
	*b = rgb & 0b11111111;
}

bool isInRange(double val, double lower, double upper) {
	return val >= lower && val <= upper;
}

int randIntRangeIncl(int min, int max_incl) {
	return rand() % (max_incl - min + 1) + min;
}

int randIntRangeExcl(int min, int max_excl) {
	return rand() % (max_excl - min) + min;
}

double randDblRangeIncl(double min, double max_incl) {
	double scale = rand() / (double)RAND_MAX; // [0, 1.0]
	return min + scale * (max_incl - min);    // [min, max]
}

void clamp(double* value, double min, double max) {
	if (*value < min) {
		*value = min;
	}
	else if (*value > max) {
		*value = max;
	}
}

typedef struct high_scores {
	char* name1;
	int score1;
	char* name2;
	int score2;
	char* name3;
	int score3;
} high_scores_t;

typedef struct high_scores1 {
	char* names[3];
	int scores[3];
} high_scores1_t;



high_scores_t getHighScores() {
	FILE* file;
	errno_t err;

	err = fopen_s(&file, "high_scores.txt", "r");

	high_scores_t scores = { 0 };

	if (err == 0) {
		// read file
		char buffer[255] = { 0 };
		char* context[255];

		// TODO use array
		char line1_copy[255];
		char line2_copy[255];
		char line3_copy[255];
		char line4_copy[255];
		char line5_copy[255];
		char line6_copy[255];

		fgets(buffer, 255, file);
		strcpy_s(line1_copy, 255, buffer);
		strtok_s(line1_copy, "\n", context);
		scores.name1 = line1_copy;

		fgets(buffer, 255, file);
		strcpy_s(line2_copy, 255, buffer);
		strtok_s(line2_copy, "\n", context);
		scores.score1 = atoi(line2_copy);

		fgets(buffer, 255, file);
		strcpy_s(line3_copy, 255, buffer);
		strtok_s(line3_copy, "\n", context);
		scores.name2 = line3_copy;

		fgets(buffer, 255, file);
		strcpy_s(line4_copy, 255, buffer);
		strtok_s(line4_copy, "\n", context);
		scores.score2 = atoi(line4_copy);


		fgets(buffer, 255, file);
		strcpy_s(line5_copy, 255, buffer);
		strtok_s(line5_copy, "\n", context);
		scores.name3 = line5_copy;

		fgets(buffer, 255, file);
		strcpy_s(line6_copy, 255, buffer);
		strtok_s(line6_copy, "\n", context);
		scores.score3 = atoi(line6_copy);

		fclose(file);
	}
	else {
		// create new file
		err = fopen_s(&file, "high_scores.txt", "w");
		fputs("name1\n", file);
		fputs("0\n", file);
		fputs("name2\n", file);
		fputs("0\n", file);
		fputs("name3\n", file);
		fputs("0\n", file);
		fclose(file);
	}

	return scores;
}
