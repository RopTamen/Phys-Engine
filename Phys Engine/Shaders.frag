#version 460 core

in vec3 colorFrag;

out vec4 colorOut;

void main() {
	vec3 color;
	color = colorFrag;
	color = (1.0f, 1.0f, 1.0f) - colorFrag;
	colorOut = vec4(color, 1.0f);
	//colorOut = vec4(1.0f, 1.0f, 1.0f, 1.0f);
};