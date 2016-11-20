#include "Shapes.hpp"

float PI = 3.14159265358979323846;
glm::vec3 zaxis(0, 0, 1);
GLuint setupVAO(float* vertices, int verticesSize, unsigned int* indices, int indicesSize, float* colors);

// Extracts VAO and indicesSize from a shape so it can be drawn
void drawShape(Shape* shape) { 
	glBindVertexArray(shape->VAO);
	glDrawElements(GL_TRIANGLES, shape->indicesSize, GL_UNSIGNED_INT, (void*)0);
}

//Note: All the vertices around the edge of any shape are constructed in a counter clockwise order, which is important for the extrusion algorithm

// Returns a simple square shape
Shape getSquare(int x, int y, float* color) {
	Shape square;
	float size = 0.5;  // size is the distance from center to sides

	// Vertices of each corner relative to center
	float vertices[] = {
		-size, size, 0,
		-size, -size, 0,
		size, -size, 0,
		size, size, 0,
	};

	// Indices for triangles drawn in counter clockwise order
	unsigned int indices[] = {
		0, 1, 2,
		3, 0, 2
	};

	// Apply same color to every vertex
	float colors[] = {
		color[0], color[1], color[2],
		color[0], color[1], color[2],
		color[0], color[1], color[2],
		color[0], color[1], color[2],
	};

	square.indicesSize = sizeof(indices);  // Store the amount of indices for use when drawing

	square.VAO = setupVAO(vertices, sizeof(vertices), indices, sizeof(indices), colors);  // Create the VAO and store it in the shape
	square.model = glm::translate(glm::vec3(x, y, 0));  // Create a model matrix displacing the square from the center
	square.pos = glm::vec2(x, y);  // Update position to reflect displacement

	return square;
}


// Returns an array containing all squares of the grid
Shape* getGrid(int i, int j) {
	Shape* grid = new Shape[i * j];  // Array containing all squares in correct order
	float color1[3] = { 0, 0, 0.8 };
	float color2[3] = { 0.8, 0, 0 };
	float* color;  // Pointer for alternating between the colors for each square
	color = color2;

	for (int x = 0; x < 8; x++) {
		for (int y = 0; y < 5; y++) {
			if (color == color1) {  // Use opposite color of previous square
				color = color2;
			}

			else if (color == color2) {
				color = color1;
			}

			Shape square = getSquare(x, y, color);  // Get a square and place it in the array
			grid[y * 8 + x] = square;
		}
	}

	return grid;
}


// Draws all squares in a grid array
void drawGrid(Shape* grid, int i, int j) {
	for (int k = 0; k < i * j; k++) {
		glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(grid[k].model));  // Pass each grid model matrix on to the shader
		drawShape(&grid[k]);  // Draw each square
	}
}


Shape* extrudeShape(float* vertices, int verticesSize, unsigned int* indices, int indicesSize, float* colors) {
	Shape* shape = new Shape;  // Instantiate a shape to store VAO and indicesSize
	// Store vectors indices and colors in vectors so we can push to the back
	std::vector<float> extruded_vertices;
	std::vector<unsigned int> extruded_indices;
	std::vector<float> extruded_colors;

	int vertexCount = verticesSize / 3;

	//Add all original vertices to the vector
	for (int i = 0; i < verticesSize; i++) {
		extruded_vertices.push_back(vertices[i]);
	}

	//Add copy of the original vertex displaced further back on the z axis
	for (int i = 0; i < verticesSize; i++) {
		if (i % 3 == 2) extruded_vertices.push_back(vertices[i] - 0.5);  // Every third element is a z value and is displaced further back
		else extruded_vertices.push_back(vertices[i]);  // Maintain x and y values
	}

	//Add original indices for front face
	for (int j = 0; j < indicesSize; j++) {
		extruded_indices.push_back(indices[j]);
	}

	//Reversed draw order for the back face
	for (int j = 0; j < indicesSize / 3; j++) {
		extruded_indices.push_back(indices[j * 3 + 2] + vertexCount);
		extruded_indices.push_back(indices[j * 3 + 1] + vertexCount);
		extruded_indices.push_back(indices[j * 3 + 0] + vertexCount);
	}

	// Due to the way vertices are set up each vertex on the front face with index j will have it's copy on the back face with index j+vertexCount
	// Indices for drawing extruded sides between last and first vertex, counter clockwise order seen from the outside
	extruded_indices.push_back(0);
	extruded_indices.push_back(vertexCount - 1);
	extruded_indices.push_back(2 * vertexCount - 1);

	extruded_indices.push_back(vertexCount);
	extruded_indices.push_back(0);
	extruded_indices.push_back(2 * vertexCount - 1);

	// Indices for drawing the rest of the extruded sides
	for (int j = 0; j < vertexCount - 1; j++) {
		extruded_indices.push_back(j);
		extruded_indices.push_back(j + vertexCount);
		extruded_indices.push_back(j + 1 + vertexCount);

		extruded_indices.push_back(j + 1);
		extruded_indices.push_back(j);
		extruded_indices.push_back(j + 1 + vertexCount);
	}

	//Simply push every original vertex color twice onto the vector
	for (int u = 0; u < 2; u++) {
		for (int k = 0; k < verticesSize; k++) {
			extruded_colors.push_back(colors[k]);
		}
	}

	// Update indicesSize and VAO of the shape
	shape->indicesSize = extruded_indices.size();
	shape->VAO = setupVAO(&extruded_vertices.front(), extruded_vertices.size(), &extruded_indices.front(), extruded_indices.size(), &extruded_colors.front());

	return shape;
}


Shape* getCircle(glm::vec3 center, float* color) {
	Shape* circle;
	float size = 0.3;  // Determines the radius of the circle
	glm::vec3 rad = { size, 0, 0 };  // Vector representing the radian in some direction

	const int polyCount = 40;  // Amount of polygons (triangles) used to make the circle

	// Pre-calculate the size of the arrays from the given polyCount
	const int verticesSize = (polyCount + 1) * 3;
	const int indicesSize = (polyCount - 1) * 3;

	float* vertices = new float[verticesSize];
	unsigned int* indices = new unsigned int[indicesSize];
	float* colors = new float[verticesSize];

	// Rotation matrix used to rotate the radian for placing next vertex on the circle circumference
	glm::mat3 rotate = glm::mat3(glm::rotate((float)1.6*PI / polyCount, zaxis));
	glm::vec3 pos = glm::vec3(0, 0, 0);

	// set up the vertices, one in center and the rest on the circle circumference 
	for (int i = 0; i < (polyCount + 1); i++) {
		vertices[3 * i] = pos.x;
		vertices[3 * i + 1] = pos.y;
		vertices[3 * i + 2] = pos.z;

		colors[3 * i] = color[0];
		colors[3 * i + 1] = color[1];
		colors[3 * i + 2] = color[2];

		// Rotate radian and update position for next vertex
		rad = rotate * rad;
		pos = rad;
	}

	//set up indices in counter clockwise drawing order around the circle
	for (int j = 0; j < polyCount - 1; j++) {
		indices[j * 3] = j + 2;
		indices[j * 3 + 1] = 0;
		indices[j * 3 + 2] = j + 1;
	}

	circle = extrudeShape(vertices, verticesSize, indices, indicesSize, colors);  // Extrude the circle and retrieve it's Shape
	circle->model = glm::translate(center);  // Displace circle from center
	circle->pos = glm::vec2(center);  // Update circle position

	delete[] vertices;
	delete[] indices;
	delete[] colors;

	return circle;
}


Shape* getTriangle(glm::vec3 center, float* color) {
	Shape* triangle;
	float size = 0.3;  // Distance from center to top vertex
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
		-size / 2, -size, 0,
		size / 2, -size, 0,
		size, 0, 0,
		size / 2, size, 0,
		-size / 2, size, 0
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
		size/2, -size, 0,
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

	glm::vec3 rad = { 0, size, 0 };  // The radian of an imaginary circle around the star touching it's touching the edges
	glm::vec3 halfRad;  // Radian scaled to half the size pointing to the inside corners of the star
	glm::mat3 rotate = glm::mat3(glm::rotate((float)2 * PI / 10, zaxis));  // Rotate the radian between vertices
	glm::mat3 scale = glm::mat3(glm::scale(glm::vec3(0.5)));  // Scale matrix for determining half the radius
	glm::vec3 pos;

	float* vertices = new float[verticesSize];

	for (int i = 0; i < 5; i++) {  // only count to 5 since we do 2 vertices at a time (one at full radian distance and one at half), and all 3 components each iteration
		pos = rad;  // Determine position of vertex from radian
		vertices[6 * i] = pos.x; vertices[6 * i + 1] = pos.y; vertices[6 * i + 2] = pos.z;
		rad = rotate * rad;  // Rotate radian
		halfRad = scale * rad;
		pos = halfRad;  // Determine position of vertex from half the radian
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