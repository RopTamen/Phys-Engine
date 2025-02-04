#include <iostream>
#include <vector>
#include <array>
#include <Windows.h>
#include <conio.h>
#include <math.h>
//#include "glad.c"
#include <KHR/khrplatform.h>
#include <glad/glad.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <GLFW/glfw3.h>

using namespace std;


//environemnt variables, in meters
array<float, 3> boundaries = { 100, 100, 100 };
float t = 0.006f;
float g = 9.81f;
int loopExit = 0;
double glTime = glfwGetTime();

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
		r = 0;
		verts = { 0, 0.5, -0.5, -0.5, 0.5, -0.5 };
	};

	PhysObj(array<float, 3> posInit, array<float, 3> velInit, float rInit) {
		pos = posInit;
		vel = velInit;
		r = rInit;
	};

	void writePos(array<float, 3> posNew) {
		pos = posNew;
		verts = { pos[0], pos[2] + 0.5f, pos[0] - 0.5f, pos[2] - 0.5f, pos[0] + 0.5f, pos[2] - 0.5f};
	};

	void writeVel(array<float, 3> velNew) {
		vel = velNew;
	};

	void writer(float rNew) {
		r = rNew;
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
void physicsSim() {
	for (int iter = 0; iter < objects.size(); iter++) {
		array<float, 3> pos = objects[iter].getPos();
		array<float, 3> vel = objects[iter].getVel();
		float r = objects[iter].getr();

		//if object moves past boundary, move it back and invert velocity, sits at -100 for 2 frames for some reason, not sure why
		for (int coll = 0; coll < pos.size(); coll++) {
			if (pos[coll] - r <= -boundaries[coll] || pos[coll] + r >= boundaries[coll]) {
				vel[coll] = -vel[coll];
				pos[coll] = pos[coll] - (pos[coll] / abs(pos[coll])) * (abs(pos[coll]) - boundaries[coll]);
			};
		};

		//update position based on velocity, then update velocity
		pos = { pos[0] + vel[0], pos[1] + vel[1], pos[2] + vel[2] };
		vel = { vel[0], vel[1], vel[2] - (g * t) };

		objects[iter].writePos(pos);
		objects[iter].writeVel(vel);

		cout << "xpos: " << objects[iter].getPos()[0] << " " << "ypos: " << objects[iter].getPos()[1] << " " << "zpos: " << objects[iter].getPos()[2] << endl;
	};
};

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	};
};

void errorCallback(int error, const char* description) {
	fprintf(stderr, "Error: %s\n", description);
};

int main() {
	glfwSetErrorCallback(errorCallback);
	if (!glfwInit()) { return 0; };

	//init window object named window with title Sample
	GLFWwindow* window = glfwCreateWindow(800, 800, "Sample", NULL, NULL);
	if (!window) { cout << "failed window creation" << endl; return 0; };

	//set escape key callback and current context
	glfwSetKeyCallback(window, keyCallback);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	gladLoadGL();

	//init object and push to vector
	PhysObj Sphere;
	Sphere.writePos({ 0, 0, 0 });
	Sphere.writeVel({ 0, 5, 15 });
	Sphere.writer(0.5);

	objects.insert(objects.begin(), Sphere);

	int i = 0;

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	float* ptr = objects[0].getVerts().data();
	glBufferData(GL_ARRAY_BUFFER, objects[0].getVerts().size(), ptr, GL_DYNAMIC_DRAW);

	//main loop
	while (!glfwWindowShouldClose(window) && loopExit == 0) {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		//render above
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		physicsSim();
		
		Sleep(t * 1000.0f);
		i = i + 1;

		
		glfwSwapBuffers(window);
		glfwPollEvents();
	};
	
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
};

