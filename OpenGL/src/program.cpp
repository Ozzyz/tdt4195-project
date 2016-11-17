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

float PI = 3.14159265358979323846;
float distance = 0.5;

struct Shape {
	GLuint VAO;
	int indicesSize;
};

GLuint setupVAO(float*, int, unsigned int*, int, float*);
glm::mat4 getView();
void check_pressed_keys();
Shape getSquare(int x, int y, float*);
Shape* getGrid(int i, int j);
void drawGrid(Shape* grid, int i, int j);
void drawShape(Shape shape);
Shape extrudeShape(float* vertices, int verticesSize, unsigned int* indices, int indicesSize, float* colors);
Shape getCircle(glm::vec3 center, float* color);
Shape getTriangle(glm::vec3 center, float* color);
Shape getHexagon(glm::vec3 center, float* color);
Shape getparallelogram(glm::vec3 center, float* color);

glm::vec3 cameraPos = glm::vec3(0.0, 0.0, 50.0);  //xyz
glm::vec3 cameraDirection = glm::vec3(0.0, 0.0, -1.0);  //vector indication direction camera points, used for movement relative to camera
glm::vec3 up = glm::vec3(0.0, 1.0, 0.0); //global up, used to recalculate cameraRight
glm::vec3 cameraRight = glm::cross(up, cameraDirection);  //right vector relative to camera, for relative movement
glm::vec2 orientation(0.0, 0.0);  //store the camera rotation on the x and y axis
glm::vec3 zaxis(0, 0, 1);

bool keys_pressed[1024];
float timer = 0.0;

float circleColor[3] = { 1, 0, 0 };


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
	Shape* grid = getGrid(8, 5);
	Shape circle = getCircle(glm::vec3(7, 4, distance), circleColor);
	Shape triangle = getTriangle(glm::vec3(5, 2, distance), circleColor);
	Shape hexagon = getHexagon(glm::vec3(0, 0, distance), circleColor);
	Shape parallelogram = getparallelogram(glm::vec3(4, 1, distance), circleColor);
	
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

		glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0)));

		drawGrid(grid, 8, 5);
		drawShape(circle);
		drawShape(triangle);
		drawShape(hexagon);
		drawShape(parallelogram);


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
}


void drawShape(Shape shape) {
	glBindVertexArray(shape.VAO);
	glDrawElements(GL_TRIANGLES, shape.indicesSize, GL_UNSIGNED_INT, (void*)0);
}


Shape getSquare(int x, int y, float* color) {
	Shape square;
	float size = 0.5;

	float vertices[] = {
		x - size, y + size, 0,
		x - size, y - size, 0,
		x + size, y - size, 0,
		x + size, y + size, 0,
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
		drawShape(grid[k]);
	}
}


Shape extrudeShape(float* vertices, int verticesSize, unsigned int* indices, int indicesSize, float* colors) {
	// Important note: extrudeShape is designed to work with the vertices on the edge being created in a counter clockwise order
	Shape shape;
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

	shape.indicesSize = extruded_indices.size();

	shape.VAO = setupVAO(&extruded_vertices.front(), extruded_vertices.size(), &extruded_indices.front(), extruded_indices.size(), &extruded_colors.front());

	return shape;
}


Shape getCircle(glm::vec3 center, float* color) {
	Shape circle;
	float size = 0.3;
	glm::vec3 rad = { size, 0, 0 };

	const int res = 40;  //number of vertices on circle radius

	const int verticesSize = (res + 1) * 3;
	const int indicesSize = (res - 1) * 3;

	float* vertices = new float[verticesSize];
	unsigned int* indices = new unsigned int[indicesSize];
	float* colors = new float[verticesSize];

	glm::mat3 rotate = glm::mat3(glm::rotate((float)1.8*PI / res, zaxis));
	glm::vec3 pos = center;

	// set up the vertices, one in center and the rest on the circle circumference 
	for (int i = 0; i < (res + 1); i++) {
		vertices[3 * i] = pos.x;
		vertices[3 * i + 1] = pos.y;
		vertices[3 * i + 2] = pos.z;

		colors[3 * i] = color[0];
		colors[3 * i + 1] = color[1];
		colors[3 * i + 2] = color[2];

		rad = rotate * rad;
		pos = center + rad;
	}

	//set up indices in counter clockwise drawing order
	for (int j = 0; j < res - 1; j++) {
		indices[j * 3] = j + 2;
		indices[j * 3 + 1] = 0;
		indices[j * 3 + 2] = j + 1;
	}

	circle = extrudeShape(vertices, verticesSize, indices, indicesSize, colors);

	delete[] vertices;
	delete[] indices;
	delete[] colors;

	return circle;
}


Shape getTriangle(glm::vec3 center, float* color) {
	Shape triangle;
	float size = 0.3;
	int verticesSize = 3 * 3;
	int indicesSize = 3;

	float* vertices = new float[verticesSize];

	vertices[0] = center.x; vertices[1] = center.y + size; vertices[2] = center.z;
	vertices[3] = center.x - size; vertices[4] = center.y - size; vertices[5] = center.z;
	vertices[6] = center.x + size; vertices[7] = center.y - size; vertices[8] = center.z;

	unsigned int* indices = new unsigned int[indicesSize];

	indices[0] = 0; indices[1] = 1; indices[2] = 2;

	float* colors = new float[verticesSize];

	for (int i = 0; i < verticesSize; i++) {
		colors[i] = color[i % 3];
	}

	triangle = extrudeShape(vertices, verticesSize, indices, indicesSize, colors);

	delete[] vertices;
	delete[] indices;
	delete[] colors;

	return triangle;
}


Shape getHexagon(glm::vec3 center, float* color) {
	Shape hexagon;
	float size = 0.3;
	const int verticesSize = 6 * 3;
	const int indicesSize = 4 * 3;

	float vertices[verticesSize] = {
		center.x - size, center.y, center.z,
		center.x - size/2, center.y - size, center.z,
		center.x + size/2, center.y - size, center.z,
		center.x + size, center.y, center.z,
		center.x + size/2, center.y + size, center.z,
		center.x - size/2, center.y + size, center.z
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

	delete[] colors;

	return hexagon;
}


Shape getparallelogram(glm::vec3 center, float* color) {
	Shape parallelogram;
	float size = 0.3;
	const int verticesSize = 4 * 3;
	const int indicesSize = 2 * 3;

	float vertices[verticesSize] = {
		center.x - size, center.y - size, center.z,
		center.x + size/2, center.y - size, center.z,
		center.x + size, center.y + size, center.z,
		center.x - size/2, center.y + size, center.z,
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

	delete[] colors;

	return parallelogram;
}