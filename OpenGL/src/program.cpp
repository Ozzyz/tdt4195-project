// Local headers
#include "program.hpp"
#include "Shapes.hpp"
#include <sstream>
#include <fstream>
#include <vector>
#include <chrono>

float distance = 0.5;  // Distance between shapes (front face) and the board face
bool keys_pressed[1024];  // Keep track of keys pressed
float timer = 0.0;  // Timer for use with time based animations
bool selectPressed = false;  // Boolean to allow for toggle functionality of the select button
glm::vec3 lastDir;  // Maintain the last direction a piece was moved to allow step by step movements

float red[3] = { 1, 0, 0 };
float blue[3] = { 0, 0, 1 };
float white[3] = { 1, 1, 1 };
float black[3] = { 0, 0, 0 };
float yellow[3] = { 1, 1, 0 };
float purple[3] = { 1, 0, 1 };
float green[3] = { 0, 1, 0 };

Shape selector;  // Shape for the selector tool
Shape* selected; // Shape ponter storing the currently selected shape

glm::mat4 getView(); // Returns the view matrix for camera control
void check_pressed_keys();

void drawGrid(Shape* grid, int i, int j);
void drawShape(Shape* shape);

void moveShape(Shape* shape, glm::vec3 direction, float time);
void moveSelector(glm::vec3 direction);
void moveSelected(glm::vec3 direction);
void select();

float getTimeDeltaSeconds();

void splitAll(const std::string &s, char delim, std::vector<std::string> &elems);
std::vector<std::string> split(const std::string &s, char delim);

std::vector<Shape*> shapes;

glm::vec3 cameraPos = glm::vec3(7.0, 5.0, 20.0);  //xyz
glm::vec3 cameraDirection = glm::vec3(0.0, 0.0, -1.0);  //vector indication direction camera points, used for movement relative to camera
glm::vec3 up = glm::vec3(0.0, 1.0, 0.0); //global up, used to recalculate cameraRight
glm::vec3 down = glm::vec3(0.0, -1.0, 0.0);
glm::vec3 left = glm::vec3(-1.0, 0.0, 0.0);
glm::vec3 right = glm::vec3(1.0, 0.0, 0.0);
glm::vec3 cameraRight = glm::cross(up, cameraDirection);  //right vector relative to camera, for relative movement
glm::vec2 orientation(0.0, 0.0);  //store the camera rotation on the x and y axis


void runProgram(GLFWwindow* window)
{
	// Get input file
	std::ifstream in_file("../gloom/src/easy01.png.txt");
	std::string line;

	// For each line
	while (getline(in_file, line)) {
		// Split line on each space 
		std::vector<std::string> line_content = split(line, ' ');
		// Extract contents
		std::string name = line_content.at(0);
		float x = atof(line_content.at(1).c_str()) - 50;
		x = round(x / 100);
		float y = atof(line_content.at(2).c_str())-50;
		y = round((400-y) / 100);  // The grid y-axis is inverted compared to image pixel position

		if (name == "Pacman") {
			shapes.push_back(getCircle(glm::vec3(x, y, distance), red));
		}

		else if (name == "Triangle") {
			shapes.push_back(getTriangle(glm::vec3(x, y, distance), purple));
		}

		else if (name == "Hexagon") {
			shapes.push_back(getHexagon(glm::vec3(x, y, distance), white));
		}

		else if (name == "Star") {
			shapes.push_back(getStar(glm::vec3(x, y, distance), blue));
		}

		else if (name == "Paralellogram") {
			shapes.push_back(getParallelogram(glm::vec3(x, y, distance), green));
		}

		else if (name == "Arrow") {
			shapes.push_back(getArrow(glm::vec3(x, y, distance), yellow));
		}
	}


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

		drawGrid(grid, 8, 5);  // Draw an 8x5 grid

		glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(selector.model));  // Poass selector model matrix to shader
		drawShape(&selector);  // Draw selector tool

		float time = getTimeDeltaSeconds();  // Get time passed each frame
		timer += time;  // Update timer
		
		// Iterate through the shape vector, update their movement animation and draw them
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

	// Keys pressed for movement of selector tool or shape
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

	// Key pressed for selection
	else if (keys_pressed[GLFW_KEY_ENTER]) {
		select();
		selectPressed = true;
	}

	// Reset the direction and selection boolean of neither of the keys are pressed
	else {
		lastDir = glm::vec3(0);
		selectPressed = false;
	}
}

// Moves a shape in it's given direction if it still has some distance from the target
void moveShape(Shape* shape, glm::vec3 direction, float time) {
	if (shape->distanceLeft > 0){
		glm::mat3 scale = glm::mat3(glm::scale(glm::vec3(time)));  // Scale the movement depending on the time passed
		glm::vec3 increment = direction*scale;
		glm::mat4 translate = glm::translate(increment);

		translate = translate*shape->model;

		shape->model = translate;
		shape->distanceLeft -= time;  // Shrink the distance a shape has left to travel by the amount of time passed
	}
}

// Moves the selector one position in the given direction
void moveSelector(glm::vec3 direction) {
	glm::vec2 pos = selector.pos + glm::vec2(direction);  // pre calculate position moved to
	if (direction != lastDir) {
		if (0 <= pos.x && pos.x <= 7 && 0 <= pos.y && pos.y <= 4) {  // restrics movement to inside of board
			glm::mat4 translate = glm::translate(direction);
			selector.model = translate*selector.model;
			selector.pos = pos;
		}
	}
}

// Selects a piece if present on the current selector position. If one is already selected, unselect
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


// Stores the desired destination for the currently selected shape for use with the moveShape function
void moveSelected(glm::vec3 direction) {
	glm::vec2 pos = selected->pos + glm::vec2(direction);
	if (0 <= pos.x && pos.x <= 7 && 0 <= pos.y && pos.y <= 4) {
		for (Shape* shape : shapes) {
			if (shape->pos.x == pos.x && shape->pos.y == pos.y) { // restrics movement to occupied field
				return;
			}
		}
		selected->distanceLeft = 1.0;
		selected->pos = pos;
		selected->direction = direction;
		selected = NULL;
	}
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


std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	splitAll(s, delim, elems);
	return elems;
}

void splitAll(const std::string &s, char delim, std::vector<std::string> &elems) {
	std::stringstream ss;
	ss.str(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
}