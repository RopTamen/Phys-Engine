#include <iostream>
#include <vector>
#include <array>
#include <Windows.h>
#include <conio.h>
#define _USE_MATH_DEFINES
#include <math.h>
//#include "glad.c"
#include <KHR/khrplatform.h>
#include <glad/glad.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <GLFW/glfw3.h>

using namespace std;

//environemnt variables, in meters
array<float, 3> boundaries = { 20, 20, 20 };
float t = 0.001f;
float g = 9.81f;
int loopExit = 0;
float glTime = float(glfwGetTime());
float fM_PI = float(M_PI);
int resx = 1280, resy = 720;

//origin point and velocity storage for generic physics object
class PhysObj {
private:
	// x, y, z format where z is height
	array<float, 3> pos;
	array<float, 3> vel;
	float r;
	vector<float> verts;
public:
	PhysObj() {
		pos = { 0,0,0 };
		vel = { 0,0,0 };
		r = 0.5f;
		verts = { (pos[0] / boundaries[0]) + (0.1f / (boundaries[0] / 20)), (pos[2] / boundaries[2]) + (0.1f / (boundaries[2] / 20)), 1.0f, 0.0f, 0.0f,
				  (pos[0] / boundaries[0]) + (0.1f / (boundaries[0] / 20)), (pos[2] / boundaries[2]) - (0.1f / (boundaries[2] / 20)), 0.0f, 1.0f, 0.0f,
				  (pos[0] / boundaries[0]) - (0.1f / (boundaries[0] / 20)), (pos[2] / boundaries[2]) - (0.1f / (boundaries[2] / 20)), 0.0f, 0.0f, 1.0f,
				  (pos[0] / boundaries[0]) - (0.1f / (boundaries[0] / 20)), (pos[2] / boundaries[2]) + (0.1f / (boundaries[2] / 20)), 1.0f, 1.0f, 1.0f };
		verts = { (pos[0]) + (0.1f / 20), (pos[2]) + (0.1f / 20), 1.0f, 0.0f, 0.0f,
				  (pos[0]) + (0.1f / 20), (pos[2]) - (0.1f / 20), 0.0f, 1.0f, 0.0f,
				  (pos[0]) - (0.1f / 20), (pos[2]) - (0.1f / 20), 0.0f, 0.0f, 1.0f,
				  (pos[0]) - (0.1f / 20), (pos[2]) + (0.1f / 20), 1.0f, 1.0f, 1.0f };
	};

	PhysObj(array<float, 3> posInit, array<float, 3> velInit, float rInit) {
		pos = posInit;
		vel = velInit;
		r = rInit;
	};

	void writePos(array<float, 3> posNew) {
		pos = posNew;
		verts = { (pos[0] / boundaries[0]) + (0.1f / (boundaries[0] / 20)), (pos[2] / boundaries[2]) + (0.1f / (boundaries[2] / 20)), verts[2] , verts[3] , verts[4] ,
				  (pos[0] / boundaries[0]) + (0.1f / (boundaries[0] / 20)), (pos[2] / boundaries[2]) - (0.1f / (boundaries[2] / 20)), verts[7] , verts[8] , verts[9] ,
				  (pos[0] / boundaries[0]) - (0.1f / (boundaries[0] / 20)), (pos[2] / boundaries[2]) - (0.1f / (boundaries[2] / 20)), verts[12], verts[13], verts[14],
				  (pos[0] / boundaries[0]) - (0.1f / (boundaries[0] / 20)), (pos[2] / boundaries[2]) + (0.1f / (boundaries[2] / 20)), verts[17], verts[18], verts[19] };
		verts = { (pos[0] ) + (0.1f / 20), (pos[2] ) + (0.1f / 20), verts[2] , verts[3] , verts[4] ,
				  (pos[0] ) + (0.1f / 20), (pos[2] ) - (0.1f / 20), verts[7] , verts[8] , verts[9] ,
				  (pos[0] ) - (0.1f / 20), (pos[2] ) - (0.1f / 20), verts[12], verts[13], verts[14],
				  (pos[0] ) - (0.1f / 20), (pos[2] ) + (0.1f / 20), verts[17], verts[18], verts[19] };
	};

	void writeVel(array<float, 3> velNew) {
		vel = velNew;
	};

	void writer(float rNew) {
		r = rNew;
	};

	void writeVerts(vector<float> vertsNew) {
		verts = vertsNew;
	};

	array<float, 3> getPos() {
		return pos;
	};

	array<float, 3> getVel() {
		return vel;
	};

	float getr() {
		return r;
	};

	vector<float> getVerts() {
		return verts;
	};
};

vector<PhysObj> objects;

//main sim
void physicsSim(float deltaT) {
	array<float, 3> pos;
	array<float, 3> vel;
	float r, outbound, inbound, fracout, fracin;

	for (int iter = 0; iter < objects.size(); iter++) {
		pos = objects[iter].getPos();
		vel = objects[iter].getVel();
		r = objects[iter].getr();

		//if object were to move past boundary in next frame, update position to vel - remaining distance to wall, invert velocity, account for gravity inbetween timestep
		for (int coll = 0; coll < pos.size(); coll++) {
			if (pos[coll] - r + vel[coll] * deltaT <= -boundaries[coll] || pos[coll] + r + vel[coll] * deltaT >= boundaries[coll]) {
				outbound = abs(vel[coll] * deltaT) - (boundaries[coll] - abs(pos[coll])) + r;
				inbound = abs(vel[coll] * deltaT) - outbound;
				fracout = (outbound / abs(vel[coll] * deltaT));
				fracin = (inbound / abs(vel[coll] * deltaT));

				if (coll == 2) {
					pos[coll] = copysign(1.0, pos[coll]) * (boundaries[coll] - outbound - r);
					vel[coll] = vel[coll] - (g * deltaT) * fracin;
					vel[coll] = -vel[coll];
					vel[coll] = vel[coll] - (g * deltaT) * fracout;
				}
				else {
					vel[coll] = -vel[coll];
				};
			}
			else {
				pos[coll] = pos[coll] + vel[coll] * deltaT;
			};
		};

		//update position based on velocity, then update velocity
		//pos = { pos[0] + vel[0], pos[1] + vel[1], pos[2] + vel[2] };
		vel = { vel[0], vel[1], (vel[2] - g * deltaT) };

		objects[iter].writePos(pos);
		objects[iter].writeVel(vel);

		//cout << "xpos: " << objects[iter].getPos()[0] << " " << "ypos: " << objects[iter].getPos()[1] << " " << "zpos: " << objects[iter].getPos()[2] << endl;
	};
};

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	};
};


const GLchar* vertexSource = R"glsl(
	#version 460 core

	in vec2 position;
	in vec3 colorVert;

	uniform vec2 positionSphere;
	uniform vec2 boundaries;

	out vec3 colorFrag;

	void main() {
		vec2 positionNew;
		positionNew = (position / boundaries) * 400 + positionSphere;
		colorFrag = colorVert;
		gl_Position = vec4(positionNew, 0.0, 1.0);
	};
)glsl";

const GLchar* fragmentSource = R"glsl(
	#version 460 core
	
	in vec3 colorFrag;

	out vec4 colorOut;

	void main() {
		vec3 color;
		color = colorFrag;
		color = (1.0, 1.0, 1.0) - colorFrag;
		colorOut = vec4(color, 1.0);
	};
)glsl";


void errorCallback(int error, const char* description) {
	fprintf(stderr, "Error: %s\n", description);
};

int main() {

	glfwSetErrorCallback(errorCallback);
	if (!glfwInit()) { return 0; };

	//init window object named window with title Sample
	GLFWwindow* window = glfwCreateWindow(resx, resy, "Sample", NULL, NULL);
	if (!window) { cout << "failed window creation" << endl; return 0; };

	//set escape key callback and current context
	glfwSetKeyCallback(window, keyCallback);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);
	gladLoadGL();

	boundaries = { float(resx / 40), boundaries[1], float(resy / 40) };

	//init object and push to vector
	PhysObj Sphere;
	Sphere.writePos({ 0, 0, 0 });
	Sphere.writeVel({ 5, 0, 25 });
	Sphere.writer(1.5);

	objects.insert(objects.begin(), Sphere);
	vector<float> vertsVec = objects[0].getVerts();
	//vertsVec = {  0.5f,  0.5f, 1.0f, 0.0f, 0.0f,
	//			  0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
	//			 -0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
	//			 -0.5f,  0.5f, 0.5f, 0.5f, 0.5f };
	float verts[20];
	copy(vertsVec.begin(), vertsVec.end(), verts);

	//const GLchar* vertexSource = ("./Shaders.vert");
	//const GLchar* fragmentSource = ("./Shaders.frag");

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	cout << sizeof(verts) << endl;

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);

	GLuint elements[] = { 
		0, 1, 2, 2, 3, 0
	};
	GLuint ebo;
	glGenBuffers(1, &ebo);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "colorOut");
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), 0);

	GLint colAttrib = glGetAttribLocation(shaderProgram, "colorVert");
	glEnableVertexAttribArray(colAttrib);
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(2*sizeof(float)));


	//cout << glGetError() << endl;

	GLint uniPos = glGetUniformLocation(shaderProgram, "positionSphere");
	GLint shaderBoundary = glGetUniformLocation(shaderProgram, "boundaries");

	float time1 = 0, time2 = 0, deltaT = 0;
	int i= 0;
	array<float, 2> positionNew;

	//main loop
	while (!glfwWindowShouldClose(window) && loopExit == 0) {
		glTime = float(glfwGetTime());

		if (i % 2 == 0) {
			time1 = glTime;
		}
		else {
			time2 = glTime;
		};
		i++;

		//cout << i / glTime << endl;

		deltaT = abs(time2 - time1);
		//cout << deltaT << endl;

		//render above
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
		glDrawElements(GL_TRIANGLES, sizeof(elements) / sizeof(GLuint), GL_UNSIGNED_INT, 0);

		physicsSim(deltaT);
		vertsVec = objects[0].getVerts();
		//vertsVec = { vertsVec[0] , vertsVec[1] , (cos(glTime) + 1) / 2, (cos(glTime + (4 * fM_PI) / 3) + 1) / 2, (cos(glTime + (2 * fM_PI) / 3) + 1) / 2,
		//		     vertsVec[5] , vertsVec[6] , (cos(glTime + (2 * fM_PI) / 3) + 1) / 2, (cos(glTime) + 1) / 2, (cos(glTime + (4 * fM_PI) / 3) + 1) / 2,
		//		     vertsVec[10], vertsVec[11], (cos(glTime + (4 * fM_PI) / 3) + 1) / 2, (cos(glTime + (2 * fM_PI) / 3) + 1) / 2, (cos(glTime) + 1) / 2 };
		copy(vertsVec.begin(), vertsVec.end(), verts);
		objects[0].writeVerts(vertsVec);

		positionNew = { objects[0].getPos()[0], objects[0].getPos()[2] };
		//cout << positionNew[0] << " " << positionNew[1] << endl;
		glUniform2f(uniPos, positionNew[0]/boundaries[0], positionNew[1]/boundaries[2]);
		glUniform2f(shaderBoundary, boundaries[0], boundaries[2]);

		boundaries = {float(width / 40), boundaries[1], float(height / 40) };

		glfwPollEvents();
		glfwSwapBuffers(window);

		//Sleep(0.1);

		//cout << glGetError() << endl;
	};

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
};

