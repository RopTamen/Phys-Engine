#version 460 core
	
in vec4 colorFrag;

out vec4 colorOut;

void main() {
	//vec4 color;
	//color = colorFrag;
	//color = (1.0f, 1.0f, 1.0f) - colorFrag;
	colorOut = colorFrag;
	colorOut = vec4(0.5, 0.8, 0.1, 0.5);
};