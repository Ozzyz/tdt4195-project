#ifndef SHAPES_HPP
#define SHAPES_HPP
#pragma once
#include "program.hpp"

typedef struct Shape {
	GLuint VAO;
	int indicesSize;
	glm::mat4 model = glm::mat4(1.0);
	float distanceLeft = 0.0;
	glm::vec3 direction = glm::vec3(0);
	glm::vec2 pos;
} Shape;

Shape getSquare(int x, int y, float* color);
Shape* getGrid(int i, int j);

Shape* extrudeShape(float* vertices, int verticesSize, unsigned int* indices, int indicesSize, float* colors);

Shape* getCircle(glm::vec3 center, float* color);
Shape* getTriangle(glm::vec3 center, float* color);
Shape* getHexagon(glm::vec3 center, float* color);
Shape* getParallelogram(glm::vec3 center, float* color);
Shape* getArrow(glm::vec3 center, float* color);
Shape* getStar(glm::vec3 center, float* color);

#endif