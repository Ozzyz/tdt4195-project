#version 430 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 vertexColor;

out vec4 fragmentColor;
mat4 m;

void main(){
	m[0] = vec4(1.0, 0.0, 0.0, 0.0);
	m[1] = vec4(0.0, 1.0, 0.0, 0.0);
	m[2] = vec4(0.0, 0.0, 1.0, 0.0);
	m[3] = vec4(0.0, 0.0, 0.0, 1.0);

    fragmentColor = vertexColor;

	gl_Position = m*vec4(position, 1.0);
}
