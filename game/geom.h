#pragma once
#include <math.h>
#include "game.h"

void scale_vector(vec2d_t* vector, double scale);
void scale_to_unit_vector(vec2d_t* vector);
double vector_magnitude(vec2d_t* vector);
void rotate_vector(vec2d_t* vector, double rads);
double deg_to_rad(double deg);
double get_vector_theta_rads(vec2d_t* vector);
void add_vector(vec2d_t* vector, double x, double y);
void rotate_vector_about_pt(vec2d_t* vector, double x, double y, double rads);
void clamp_vector_magnitude(vec2d_t* vector, double max);


void scale_vector(vec2d_t* vector, double scale) {
	vector->x = vector->x * scale;
	vector->y = vector->y * scale;
}

void scale_to_unit_vector(vec2d_t* vector) {
	double mag = vector_magnitude(vector);
	double scale = 1.0 / mag;
	scale_vector(vector, scale);
}

double vector_magnitude(vec2d_t* vector) {
	return sqrt(pow(vector->x, 2) + pow(vector->y, 2));
}

void rotate_vector(vec2d_t* vector, double rads) {
	double x = vector->x * cos(rads) - vector->y * sin(rads);
	double y = vector->x * sin(rads) + vector->y * cos(rads);

	vector->x = x;
	vector->y = y;
}

double deg_to_rad(double deg) {
	return deg * M_PI / 180;
}

double get_vector_theta_rads(vec2d_t* vector) {
	return atan(vector->y / vector->x);
}

void add_vector(vec2d_t* vector, double x, double y) {
	vector->x += x;
	vector->y += y;
}

void rotate_vector_about_pt(vec2d_t* vector, double x, double y, double rads) {
	vector->x -= x;
	vector->y -= y;

	rotate_vector(vector, rads);

	vector->x += x;
	vector->y += y;
}

void clamp_vector_magnitude(vec2d_t* vector, double max) {
	double mag = vector_magnitude(vector);

	if (mag > max) {
		scale_vector(vector, max / mag);
	}
}

