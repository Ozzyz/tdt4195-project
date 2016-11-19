// Local headers
#include "program.hpp"
#include "gloom/gloom.hpp"
#include "gloom/shader.hpp"
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <chrono>

float PI = 3.14159265358979323846;
float distance = 0.5;
bool keys_pressed[1024];
float timer = 0.0;
bool selectPressed = false;
glm::vec3 lastDir;

float red[3] = { 1, 0, 0 };
float blue[3] = { 0, 0, 1 };
float white[3] = { 1, 1, 1 };
float black[3] = { 0, 0, 0 };
float yellow[3] = { 1, 1, 0 };
float purple[3] = { 1, 0, 1 };
float green[3] = { 0, 1, 0 };

struct Shape {
	GLuint VAO;
	int indicesSize;
	glm::mat4 model = glm::mat4(1.0);
	float distanceLeft = 0.0;
	glm::vec3 direction = glm::vec3(0);
	glm::vec2 pos;
};

Shape selector;
Shape* selected;

GLuint setupVAO(float*, int, unsigned int*, int, float*);
glm::mat4 getView();
void check_pressed_keys();

void drawGrid(Shape* grid, int i, int j);
void drawShape(Shape* shape);

Shape getSquare(int x, int y, float* color);
Shape* getGrid(int i, int j);

Shape* extrudeShape(float* vertices, int verticesSize, unsigned int* indices, int indicesSize, float* colors);

Shape* getCircle(glm::vec3 center, float* color);
Shape* getTriangle(glm::vec3 center, float* color);
Shape* getHexagon(glm::vec3 center, float* color);
Shape* getParallelogram(glm::vec3 center, float* color);
Shape* getArrow(glm::vec3 center, float* color);
Shape* getStar(glm::vec3 center, float* color);
void moveShape(Shape* shape, glm::vec3 direction, float time);
void moveSelector(glm::vec3 direction);
void moveSelected(glm::vec3 direction);
void select();
float getTimeDeltaSeconds();

std::vector<Shape*> shapes;

glm::vec3 cameraPos = glm::vec3(0.0, 0.0, 50.0);  //xyz
glm::vec3 cameraDirection = glm::vec3(0.0, 0.0, -1.0);  //vector indication direction camera points, used for movement relative to camera
glm::vec3 up = glm::vec3(0.0, 1.0, 0.0); //global up, used to recalculate cameraRight
glm::vec3 down = glm::vec3(0.0, -1.0, 0.0);
glm::vec3 left = glm::vec3(-1.0, 0.0, 0.0);
glm::vec3 right = glm::vec3(1.0, 0.0, 0.0);
glm::vec3 cameraRight = glm::cross(up, cameraDirection);  //right vector relative to camera, for relative movement
glm::vec2 orientation(0.0, 0.0);  //store the camera rotation on the x and y axis
glm::vec3 zaxis(0, 0, 1);

void runProgram(GLFWwindow* window)
{
    // Set GLFW callback mechanism(s)
    glfwSetKeyCallback(window, keyboardCallback);

    // Enable depth (Z) buffer (accept "closest" fragment)
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Configure miscellaneous OpenGL settings
	glEnable(GL_CULL_FACE);

    // Set default colour after clearing the colour buffer
    glClearColor(0.3f, 0.3f, 0.4f, 1.0f);

	// Setup shader
	Gloom::Shader shader;
	shader.makeBasicShader("../gloom/shaders/mvp.vert", "../gloom/shaders/colors.frag");
	// Boolean for more simple activation or deactivaton of shader when testing
	bool shaderON = true;

	selector = getSquare(0, 0, black);
	Shape* grid = getGrid(8, 5);

	selector.model = glm::translate(glm::vec3(0, 0, 0.6))*glm::scale(glm::vec3(0.2));

	shapes.push_back(getCircle(glm::vec3(7, 4, distance), red));
	shapes.push_back(getTriangle(glm::vec3(5, 2, distance), purple));
	shapes.push_back(getHexagon(glm::vec3(0, 0, distance), white));
	shapes.push_back(getHexagon(glm::vec3(1, 1, distance), black));
	shapes.push_back(getParallelogram(glm::vec3(4, 1, distance), green));
	shapes.push_back(getArrow(glm::vec3(6, 3, distance), yellow));
	shapes.push_back(getStar(glm::vec3(2, 2, distance), blue));

    // Rendering Loop
    while (!glfwWindowShouldClose(window))
    {
		// Clear colour and depth buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw your scene here
		if (shaderON){
			shader.activate();
			
			check_pressed_keys();

			cameraRight = glm::cross(cameraDirection, up); // update global cameraRight vector for strafing

			glm::mat4 view = getView();
			
			glm::mat4 projection = glm::perspective(glm::radians(60.0), 1.0, 1.0, 200.0);
			//glm::mat4 projection = glm::ortho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);

			glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(4, 1, GL_FALSE, glm::value_ptr(projection));
		}

		drawGrid(grid, 8, 5);

		glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(selector.model));
		drawShape(&selector);

		float time = getTimeDeltaSeconds();
		timer += time;
		
		for (Shape* shape : shapes) {
			moveShape(shape, shape->direction, time);
			glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(shape->model));
			drawShape(shape);
		}

		if (shaderON){
		shader.deactivate();
		}

        // Handle other events
        glfwPollEvents();

        // Flip buffers
        glfwSwapBuffers(window);
    }

	if (shaderON){
	shader.destroy();
	}
}


GLuint setupVAO(float* vertices, int verticesSize, unsigned int* indices, int indicesSize, float* colors) {
	// Generates a new Vertex Array Object
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Creates a vertexbuffer
	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, verticesSize * sizeof(float), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	GLuint colorbuffer;
	glGenBuffers(1, &colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, verticesSize * sizeof(float), colors, GL_STATIC_DRAW);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// Creates a indicebuffer
	GLuint indicebuffer;
	glGenBuffers(1, &indicebuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indicebuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize * sizeof(unsigned int), indices, GL_STATIC_DRAW);

	return VertexArrayID;
}


glm::mat4 getView() {
	// Translate all objects in the worldspace relative to camera
	glm::mat4 translate = glm::translate(-cameraPos);
	// Scale objects, in this case by a static number 
	float scaleFactor = 2;
	glm::mat4 scale = glm::scale(glm::vec3(scaleFactor, scaleFactor, 1.0));

	// Rotate world
	glm::mat4 Xrotation = glm::rotate(orientation.x, cameraRight);
	glm::mat4 Yrotation = glm::rotate(orientation.y, up);

	return Yrotation*Xrotation*translate*scale;
}


void keyboardCallback(GLFWwindow* window, int key, int scancode,
	int action, int mods)
{
	// Use escape key for terminating the GLFW window
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	keys_pressed[key] = (action != GLFW_RELEASE);
}


void check_pressed_keys() {
	// Check what keys_pressed are activated, and react accordingly
	// Note: This means that multiple keys_pressed kan be activated at once.

	// Sets how much the camera moves during each frame if key pressed
	float moveSpeed = 0.05;
	float cameraSpeed = 0.005;

	// WASD will control forward/back/left/right
	if (keys_pressed[GLFW_KEY_W]) {
		cameraPos += moveSpeed*cameraDirection;
	}
	if (keys_pressed[GLFW_KEY_A]) {
		cameraPos -= moveSpeed*cameraRight;
	}
	if (keys_pressed[GLFW_KEY_S]) {
		cameraPos -= moveSpeed*cameraDirection;
	}
	if (keys_pressed[GLFW_KEY_D]) {
		cameraPos += moveSpeed*cameraRight;
	}
	// Arrow keys_pressed control rotation (L&R -> x-axis, U/D -> y-axis)
	if (keys_pressed[GLFW_KEY_LEFT]) {
		cameraDirection = glm::mat3(glm::rotate(cameraSpeed, up)) * cameraDirection;
		orientation.y -= cameraSpeed;
	}
	if (keys_pressed[GLFW_KEY_RIGHT]) {
		cameraDirection = glm::mat3(glm::rotate(-cameraSpeed, up)) * cameraDirection;
		orientation.y += cameraSpeed;
	}
	if (keys_pressed[GLFW_KEY_UP]) {
		cameraDirection = glm::mat3(glm::rotate(cameraSpeed, cameraRight)) * cameraDirection;
		orientation.x -= cameraSpeed;
	}
	if (keys_pressed[GLFW_KEY_DOWN]) {
		cameraDirection = glm::mat3(glm::rotate(-cameraSpeed, cameraRight)) * cameraDirection;
		orientation.x += cameraSpeed;
	}
	// Key presses for up and down movement, space for up and control for down
	if (keys_pressed[GLFW_KEY_SPACE]) {
		cameraPos.y += moveSpeed;
	}
	if (keys_pressed[GLFW_KEY_LEFT_CONTROL]) {
		cameraPos.y -= moveSpeed;
	}

	if (keys_pressed[GLFW_KEY_KP_6]) {
		if (selected) moveSelected(right);
		else moveSelector(right);
		lastDir = right;
	}

	else if (keys_pressed[GLFW_KEY_KP_4]) {
		if (selected) moveSelected(left);
		else moveSelector(left);
		lastDir = left;
	}

	else if (keys_pressed[GLFW_KEY_KP_8]) {
		if (selected) moveSelected(up);
		else moveSelector(up);
		lastDir = up;
	}

	else if (keys_pressed[GLFW_KEY_KP_2]) {
		if (selected) moveSelected(down);
		else moveSelector(down);
		lastDir = down;
	}

	else if (keys_pressed[GLFW_KEY_ENTER]) {
		select();
		selectPressed = true;
	}

	else {
		lastDir = glm::vec3(0);
		selectPressed = false;
	}
}


void drawShape(Shape* shape) {
	glBindVertexArray(shape->VAO);
	glDrawElements(GL_TRIANGLES, shape->indicesSize, GL_UNSIGNED_INT, (void*)0);
}


Shape getSquare(int x, int y, float* color) {
	Shape square;
	float size = 0.5;

	float vertices[] = {
		-size, size, 0,
		-size, -size, 0,
		size, -size, 0,
		size, size, 0,
	};

	unsigned int indices[] = {
		0, 1, 2,
		3, 0, 2
	};

	float colors[] = {
		color[0], color[1], color[2],
		color[0], color[1], color[2],
		color[0], color[1], color[2],
		color[0], color[1], color[2],
	};

	square.indicesSize = sizeof(indices);

	square.VAO = setupVAO(vertices, sizeof(vertices), indices, sizeof(indices), colors);
	square.model = glm::translate(glm::vec3(x, y, 0));
	square.pos = glm::vec2(x, y);

	return square;
}


Shape* getGrid(int i, int j) {
	Shape* grid = new Shape[i * j];
	float color1[3] = { 0, 0, 0.8 };
	float color2[3] = { 0.8, 0, 0 };
	float* color = new float[3];
	color = color2;

	for (int x = 0; x < 8; x++) {
		for (int y = 0; y < 5; y++) {
			if (color == color1) {
				color = color2;
			}

			else if (color == color2) {
				color = color1;
			}

			Shape square = getSquare(x, y, color);
			grid[y * 8 + x] = square;
		}
	}

	return grid;
}


void drawGrid(Shape* grid, int i, int j) {
	for (int k = 0; k < i * j; k++) {
		glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(grid[k].model));
		drawShape(&grid[k]);
	}
}


Shape* extrudeShape(float* vertices, int verticesSize, unsigned int* indices, int indicesSize, float* colors) {
	// Important note: extrudeShape is designed to work with the vertices on the edge being created in a counter clockwise order
	Shape* shape = new Shape;
	std::vector<float> extruded_vertices;
	std::vector<unsigned int> extruded_indices;
	std::vector<float> extruded_colors;

	int vertexCount = verticesSize / 3;

	for (int i = 0; i < verticesSize; i++) {
		extruded_vertices.push_back(vertices[i]);
	}

	for (int i = 0; i < verticesSize; i++) {
		if (i % 3 == 2) extruded_vertices.push_back(vertices[i] - 0.5);
		else extruded_vertices.push_back(vertices[i]);
	}

	for (int j = 0; j < indicesSize; j++) {
		extruded_indices.push_back(indices[j]);
	}

	for (int j = 0; j < indicesSize/3; j++) {
		extruded_indices.push_back(indices[j*3 + 2] + vertexCount);
		extruded_indices.push_back(indices[j*3 + 1] + vertexCount);
		extruded_indices.push_back(indices[j*3 + 0] + vertexCount);
	}

	// indices for drawing extruded sides between last and first vertex
	extruded_indices.push_back(0);
	extruded_indices.push_back(vertexCount-1);
	extruded_indices.push_back(2*vertexCount-1);

	extruded_indices.push_back(vertexCount);
	extruded_indices.push_back(0);
	extruded_indices.push_back(2 * vertexCount - 1);

	// indices for drawing the rest of the extruded sides
	for (int j = 0; j < vertexCount - 1; j++) {
		extruded_indices.push_back(j);
		extruded_indices.push_back(j + vertexCount);
		extruded_indices.push_back(j + 1 + vertexCount);
		
		extruded_indices.push_back(j + 1);
		extruded_indices.push_back(j);
		extruded_indices.push_back(j + 1 + vertexCount);
	}

	for (int u = 0; u < 2; u++) {
		for (int k = 0; k < verticesSize; k++) {
			extruded_colors.push_back(colors[k]);
		}
	}

	shape->indicesSize = extruded_indices.size();
	shape->VAO = setupVAO(&extruded_vertices.front(), extruded_vertices.size(), &extruded_indices.front(), extruded_indices.size(), &extruded_colors.front());

	return shape;
}


Shape* getCircle(glm::vec3 center, float* color) {
	Shape* circle;
	float size = 0.3;
	glm::vec3 rad = { size, 0, 0 };

	const int res = 40;  //number of vertices on circle radius

	const int verticesSize = (res + 1) * 3;
	const int indicesSize = (res - 1) * 3;

	float* vertices = new float[verticesSize];
	unsigned int* indices = new unsigned int[indicesSize];
	float* colors = new float[verticesSize];

	glm::mat3 rotate = glm::mat3(glm::rotate((float)1.6*PI / res, zaxis));
	glm::vec3 pos = glm::vec3(0, 0, 0);

	// set up the vertices, one in center and the rest on the circle circumference 
	for (int i = 0; i < (res + 1); i++) {
		vertices[3 * i] = pos.x;
		vertices[3 * i + 1] = pos.y;
		vertices[3 * i + 2] = pos.z;

		colors[3 * i] = color[0];
		colors[3 * i + 1] = color[1];
		colors[3 * i + 2] = color[2];

		rad = rotate * rad;
		pos = rad;
	}

	//set up indices in counter clockwise drawing order
	for (int j = 0; j < res - 1; j++) {
		indices[j * 3] = j + 2;
		indices[j * 3 + 1] = 0;
		indices[j * 3 + 2] = j + 1;
	}

	circle = extrudeShape(vertices, verticesSize, indices, indicesSize, colors);
	circle->model = glm::translate(center);
	circle->pos = glm::vec2(center);

	delete[] vertices;
	delete[] indices;
	delete[] colors;

	return circle;
}


Shape* getTriangle(glm::vec3 center, float* color) {
	Shape* triangle;
	float size = 0.3;
	const int verticesSize = 3 * 3;
	const int indicesSize = 3;

	float vertices[verticesSize] = {
		0, size, 0,
		-size, -size, 0,
		size, -size, 0
	};

	unsigned int indices[indicesSize] = { 0, 1, 2 };

	float* colors = new float[verticesSize];

	for (int i = 0; i < verticesSize; i++) {
		colors[i] = color[i % 3];
	}

	triangle = extrudeShape(vertices, verticesSize, indices, indicesSize, colors);
	triangle->model = glm::translate(center);
	triangle->pos = glm::vec2(center);

	delete[] colors;

	return triangle;
}


Shape* getHexagon(glm::vec3 center, float* color) {
	Shape* hexagon;
	float size = 0.3;
	const int verticesSize = 6 * 3;
	const int indicesSize = 4 * 3;

	float vertices[verticesSize] = {
		-size, 0, 0,
		-size/2, -size, 0,
		size/2, -size, 0,
		size, 0, 0,
		size/2, size, 0,
		-size/2, size, 0
	};

	unsigned int indices[indicesSize]{
		0, 1, 2,
		2, 3, 4,
		4, 5, 0,
		0, 2, 4
	};

	float* colors = new float[verticesSize];

	for (int i = 0; i < verticesSize; i++) {
		colors[i] = color[i % 3];
	}

	hexagon = extrudeShape(vertices, verticesSize, indices, indicesSize, colors);
	hexagon->model = glm::translate(center);
	hexagon->pos = glm::vec2(center);

	delete[] colors;

	return hexagon;
}


Shape* getParallelogram(glm::vec3 center, float* color) {
	Shape* parallelogram;
	float size = 0.3;
	const int verticesSize = 4 * 3;
	const int indicesSize = 2 * 3;

	float vertices[verticesSize] = {
		-size, -size, 0,
		size/2,-size, 0,
		size, size, 0,
		-size/2, size, 0,
	};

	unsigned int indices[indicesSize]{
		0, 1, 2,
		0, 2, 3,
	};

	float* colors = new float[verticesSize];

	for (int i = 0; i < verticesSize; i++) {
		colors[i] = color[i % 3];
	}

	parallelogram = extrudeShape(vertices, verticesSize, indices, indicesSize, colors);
	parallelogram->model = glm::translate(center);
	parallelogram->pos = glm::vec2(center);

	delete[] colors;

	return parallelogram;
}


Shape* getArrow(glm::vec3 center, float* color) {
	Shape* arrow;
	float size = 0.3;
	const int verticesSize = 6 * 3;
	const int indicesSize = 4 * 3;

	float vertices[verticesSize] = {
		0, size, 0,
		-size, -size, 0,
		-size/2, -size, 0,
		0, size/2, 0,
		size/2, -size, 0,
		size, -size, 0
	};

	unsigned int indices[indicesSize] = { 
		0, 1, 3,
		3, 1, 2,
		3, 4, 5,
		0, 3, 5
	};

	float* colors = new float[verticesSize];

	for (int i = 0; i < verticesSize; i++) {
		colors[i] = color[i % 3];
	}

	arrow = extrudeShape(vertices, verticesSize, indices, indicesSize, colors);
	arrow->model = glm::translate(center);
	arrow->pos = glm::vec2(center);

	delete[] colors;

	return arrow;
}


Shape* getStar(glm::vec3 center, float* color) {
	Shape* star;
	float size = 0.3;
	const int verticesSize = 10 * 3;
	const int indicesSize = 8 * 3;

	glm::vec3 rad = { 0, size, 0 };
	glm::vec3 halfRad;
	glm::mat3 rotate = glm::mat3(glm::rotate((float)2*PI/10, zaxis));
	glm::mat3 scale = glm::mat3(glm::scale(glm::vec3(0.5, 0.5, 0.5)));
	glm::vec3 pos;

	float* vertices = new float[verticesSize];

	for (int i = 0; i < 5; i++) {  // only count to 5 since we do 2 vertices at a time, and all 3 components each iteration
		pos = rad;
		vertices[6 * i] = pos.x; vertices[6 * i + 1] = pos.y; vertices[6 * i + 2] = pos.z;
		rad = rotate * rad;
		halfRad = scale * rad;
		pos = halfRad;
		vertices[6 * i + 3] = pos.x; vertices[6 * i + 4] = pos.y; vertices[6 * i + 5] = pos.z;
		rad = rotate * rad;
	}

	unsigned int indices[indicesSize] = { 
		0, 1, 9,
		2, 3, 1,
		4, 5, 3,
		6, 7, 5,
		8, 9, 7,
		1, 5, 9,
		1, 3, 5,
		9, 5, 7
	};

	float* colors = new float[verticesSize];

	for (int i = 0; i < verticesSize; i++) {
		colors[i] = color[i % 3];
	}

	star = extrudeShape(vertices, verticesSize, indices, indicesSize, colors);
	star->model = glm::translate(center);
	star->pos = glm::vec2(center);

	delete[] vertices;
	delete[] colors;

	return star;
}


void moveShape(Shape* shape, glm::vec3 direction, float time) {
	if (shape->distanceLeft > 0){
		glm::mat3 scale = glm::mat3(glm::scale(glm::vec3(time)));
		glm::vec3 increment = direction*scale;
		glm::mat4 translate = glm::translate(increment);

		translate = translate*shape->model;

		shape->model = translate;
		shape->distanceLeft -= time;
	}
}


void moveSelector(glm::vec3 direction) {
	if (direction != lastDir){
		glm::mat4 translate = glm::translate(direction);
		selector.model = translate*selector.model;
		selector.pos += glm::vec2(direction);
	}
}


void select() {
	if (!selectPressed) {
		if (selected != NULL) {
			selected = NULL;
			return;
		}

		for (Shape* shape : shapes) {
			if (shape->pos.x == selector.pos.x && shape->pos.y == selector.pos.y) {
				selected = shape;
				return;
			}
		}
	}
}


void moveSelected(glm::vec3 direction) {
	selected->distanceLeft = 1.0;
	selected->pos += glm::vec2(direction);
	selected->direction = direction;
	selected = NULL;
}


// In order to be able to calculate when the getTimeDeltaSeconds() function was last called, we need to know the point in time when that happened. This requires us to keep hold of that point in time. 
// We initialise this value to the time at the start of the program.
static std::chrono::steady_clock::time_point _previousTimePoint = std::chrono::steady_clock::now();

// Calculates the elapsed time since the previous time this function was called.
float getTimeDeltaSeconds() {
	// Determine the current time
	std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
	// Look up the time when the previous call to this function occurred.
	std::chrono::steady_clock::time_point previousTime = _previousTimePoint;

	// Calculate the number of nanoseconds that elapsed since the previous call to this function
	long long timeDelta = std::chrono::duration_cast<std::chrono::nanoseconds>(currentTime - _previousTimePoint).count();
	// Convert the time delta in nanoseconds to seconds
	double timeDeltaSeconds = (double)timeDelta / 1000000000.0;

	// Store the previously measured current time
	_previousTimePoint = currentTime;

	// Return the calculated time delta in seconds
	return (float)timeDeltaSeconds;
}