#version 460 core

in vec3 position;
in vec3 colorVert;

uniform vec3 positionSphere;
uniform vec3 boundaries;

out vec4 colorFrag;

void main() {
	vec3 positionNew;
	float alpha;
	positionNew = (position / boundaries * 400) + positionSphere;
	//alpha = ((positionSphere.z + boundaries.z) / (2 * boundaries.z));
	alpha = 0;
	colorFrag = vec4(colorVert, alpha);
	gl_Position = vec4(positionNew, 1.0);
};