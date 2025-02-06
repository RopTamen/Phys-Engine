#version 460 core

in vec2 position;
in vec3 colorVert;

uniform vec2 positionSphere;
uniform vec2 boundaries;

out vec3 colorFrag;

void main() {
	vec2 positionNew;
	positionNew = (position / boundaries) + positionSphere;
	colorFrag = colorVert;
	gl_Position = vec4(positionNew, 0.0, 1.0);
};
