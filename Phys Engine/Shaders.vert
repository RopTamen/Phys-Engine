#version 460 core

in vec2 position;
in vec3 colorVert;

uniform vec2 positionSphere;

out vec3 colorFrag;

void main() {
	vec2 positionNew;
	positionNew = position + positionSphere;
	//positionNew.x = position.x;
	//positionNew.y = position.y * -1.0;
	colorFrag = colorVert;
	gl_Position = vec4(positionNew, 0.0, 1.0);
};