#version 460 core

in vec3 position;
in vec3 colorVert;

uniform vec3 positionSphere;
uniform vec3 boundaries;

out vec3 colorFrag;

void main() {
	vec3 positionNew;
	positionNew = (position / boundaries) + positionSphere;
	colorFrag = colorVert;
	gl_Position = vec4(positionNew, 1.0);
};