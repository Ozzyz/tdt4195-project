#version 430 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 vertexColor;
uniform layout(location = 2) mat4 model;
uniform layout(location = 3) mat4 view;
uniform layout(location = 4) mat4 projection;

out vec4 fragmentColor;

void main(){

    fragmentColor = vertexColor;

	gl_Position = projection*view*model*vec4(position, 1.0);
}
