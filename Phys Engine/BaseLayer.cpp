#include <iostream>
#include <vector>
#include <array>
#include <Windows.h>
#include <conio.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <thread>
#include <atomic>
//#include "glad.c"
#include <KHR/khrplatform.h>
#include <glad/glad.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <GLFW/glfw3.h>

using namespace std;
atomic<bool> isRunning(true);

//environemnt variables, in meters
array<float, 3> boundaries = { 20, 20, 20 };
float t = 0.001f;
float g = 9.81f;
float glTime = float(glfwGetTime());
float fM_PI = float(M_PI);
int loopExit = 0;
int resx = 1280, resy = 720;

//origin point and velocity storage for generic physics object
class PhysObj {
private:
	// x, y, z format where z is height (for now)
	array<float, 3> pos;
	array<float, 3> vel;
	vector<float> verts;
	float r;
public:
	PhysObj() {
		pos = { 0,0,0 };
		vel = { 0,0,0 };
		rad = 0.5f;
		verts = {  +0.1f / 20,  +0.1f / 20, 0.0f, 1.0f, 0.0f, 0.0f,
				   +0.1f / 20,  -0.1f / 20, 0.0f, 0.0f, 1.0f, 0.0f,
				   -0.1f / 20,  -0.1f / 20, 0.0f, 0.0f, 0.0f, 1.0f,
				   -0.1f / 20,  +0.1f / 20, 0.0f, 1.0f, 1.0f, 1.0f };
	};

	PhysObj(array<float, 3> posInit, array<float, 3> velInit, float radInit) {
		pos = posInit;
		vel = velInit;
		rad = radInit;
	};

	void writePos(array<float, 3> posNew) {
		pos = posNew;
		verts = { (pos[0] ) + (0.1f / 20), (pos[2] ) + (0.1f / 20), verts[2] , verts[3] , verts[4] ,
				  (pos[0] ) + (0.1f / 20), (pos[2] ) - (0.1f / 20), verts[7] , verts[8] , verts[9] ,
				  (pos[0] ) - (0.1f / 20), (pos[2] ) - (0.1f / 20), verts[12], verts[13], verts[14],
				  (pos[0] ) - (0.1f / 20), (pos[2] ) + (0.1f / 20), verts[17], verts[18], verts[19] };
	};

	void writeVel(array<float, 3> velNew) {
		vel = velNew;
	};

	void writeRad(float radNew) {
		rad = radNew;
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

	float getRad() {
		return rad;
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
	float rad, outOfBounds, withinBounds, fracout, fracin;

	for (int iter = 0; iter < objects.size(); iter++) {
		pos = objects[iter].getPos();
		vel = objects[iter].getVel();
		rad = objects[iter].getRad();

		//if object were to move past boundary in next frame, update position to vel - remaining distance to wall, invert velocity, account for gravity inbetween timestep on top and bottom Axisisions
		for (int Axis = 0; Axis <= 2; Axis++) {
			if (abs(pos[Axis] + copysign(rad, pos[Axis]) + vel[Axis] * deltaT) >= boundaries[Axis]) {
				
				cout << "vel: " << vel[Axis] * deltaT << "  bound: " << boundaries[Axis] << "  pos: " << pos[Axis] << endl;

				// position + radius + velocity on the next step subtracted by the boundary to establish amount of overlap
				outOfBounds = abs(pos[Axis]) + rad + abs(vel[Axis] * deltaT) - boundaries[Axis];
				// remaining velocity distance that would not result in the object OOB
				withinBounds = abs(vel[Axis] * deltaT) - outOfBounds;
				cout << Axis << " : out " << outOfBounds << "    :    in " << withinBounds << endl;

				// fractions of the above values
				fracout = (outOfBounds / abs(vel[Axis] * deltaT));
				fracin = (withinBounds / abs(vel[Axis] * deltaT));
				cout << Axis << " : fracout " << fracout << "    :    fracin " << fracin << endl;

				if (Axis == 2) {
					// adjust position according to velocity before collisions and remaining velocity afterwards
					pos[2] = pos[2] - copysign(withinBounds - outOfBounds, pos[Axis]);
					// adjust z velocity by the fraction inside and outside the boundary with gravity
					vel[2] = vel[2] - (g * deltaT) * fracin + (g * deltaT) * fracout;
					vel[2] = -vel[2];
				}
				else {
					// adjust all other positions by distances in and out and velocity just flips
					pos[Axis] = pos[Axis] - copysign(withinBounds - outOfBounds, pos[Axis]);
					vel[Axis] = -vel[Axis];
				};
			}
			else {
				// objects continue along their path
				pos[Axis] = pos[Axis] + vel[Axis] * deltaT;
			};
		};
		objects[iter].writePos(pos);
		objects[iter].writeVel(vel);
	};
};

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	};
};

// vertex shader definition, make into separate file to read to a string later
const GLchar* vertexSource = R"glsl(
	#version 460 core

	in vec3 position;
	in vec3 colorVert;

	uniform vec3 positionSphere;
	uniform vec3 boundaries;

	out vec3 colorFrag;

	void main() {
		vec3 positionNew;
		positionNew = (position / boundaries * 400) + positionSphere;
		colorFrag = colorVert;
		gl_Position = vec4(positionNew, 1.0);
	};
)glsl";

// fragment shader definition, make into separate file to read to a string later
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

// struct for shaderSetup data
struct shaderSetupStrc {
	GLFWwindow* windowStrc;
	GLint posUniformStrc;
	GLint boundaryUniformStrc;
};

// takes the vertex and element information and compiles the shaders and window, returning the window, position, and boundary bindings
shaderSetupStrc shaderSetup(float verts[], int vertsSize, GLuint elements[], int elementSize) {
	// create struct object
	shaderSetupStrc data;

	//set escape key callback and current context
	//glfwMakeContextCurrent(&windowFunc);
	glfwSwapInterval(0);
	gladLoadGL();

	// init vertex array object, stores vertices
	cout << endl;

	//truthfully no clue how any of this works really
	cout << endl;

	//truthfully no clue how any of this works really
	cout << endl;
	// init vertex buffer object, stores information position
	//truthfully no clue how any of this works really
	cout << endl;

	glBufferData(GL_ARRAY_BUFFER, vertsSize, verts, GL_DYNAMIC_DRAW);
	GLuint vao;
	// init element buffer object, stores the order to draw indices
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, elementSize, elements, GL_STATIC_DRAW);

	// compile vertext and fragment shaders from above shader definitions
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

	//int  success;
	//char infoLog[512];

	// init element buffer object, stores the order to draw indices
	GLuint ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

	//int  success;
	//char infoLog[512];
	// compile shader program by attaching the vertex and fragment shaders, 
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// binds the attribute "position" in the vertex shader to posAttrib to be used in defining the array
	glCompileShader(fragmentShader);

	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	glDeleteShader(fragmentShader);

	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "colorOut");
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

	glUseProgram(shaderProgram);

	glDeleteShader(vertexShader);
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));

	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), 0);

	// binds the attribute "colorVert" in the vertex shader to colAttrib to be used in defining the array
	GLint colAttrib = glGetAttribLocation(shaderProgram, "colorVert");
	glEnableVertexAttribArray(colAttrib);
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));

void renderThreadFunc(GLFWwindow* window) {

	float verts[24];
	vector<float> vertsVec = objects[0].getVerts();
	copy(vertsVec.begin(), vertsVec.end(), verts);

	// print out all vertex data for error checking
	for (int l = 0; l < sizeof(verts) / sizeof(float); l++) {
		cout << verts[l] << " ";
	};
	cout << endl;
	cout << sizeof(verts) << endl;

	// element list for vertex draw order
	GLuint elements[] = {
		0, 1, 2, 2, 3, 0
	};

	// declare GLFW variable and array sizes
	int vertsSize = sizeof(verts);
	int elementSize = sizeof(elements);
	//GLFWwindow* window = &windowRef;
	GLint uniPos, shaderBoundary;
	glfwMakeContextCurrent(window);

	// pass arrays and variables to shaderSetup, return struct to shaderFuncOutput and decompose
	shaderSetupStrc shaderFuncOutput = shaderSetup(verts, vertsSize, elements, elementSize);
	//window = shaderFuncOutput.windowStrc;
	uniPos = shaderFuncOutput.posUniformStrc;
	shaderBoundary = shaderFuncOutput.boundaryUniformStrc;

	array<float, 3> positionNew;

	// binds the inputs positionSphere and boundaries to temporary variable to be passed to main, they can be modified by the CPU to be sent to the shader
	GLint uniPosFunc = glGetUniformLocation(shaderProgram, "positionSphere");
	GLint shaderBoundaryFunc = glGetUniformLocation(shaderProgram, "boundaries");
	//main loop, multithread physics and rendering later preferrably --- I am delusional
	while (isRunning) {
	//data.windowStrc = &windowFunc;
	data.posUniformStrc = uniPosFunc;
			time1 = float(glfwGetTime());;
			i = 1;
	//main loop, multithread physics and rendering later preferrably --- I am delusional
	while (!glfwWindowShouldClose(window) && loopExit == 0) {
			time2 = float(glfwGetTime());;
			i = 0;

			time1 = float(glfwGetTime());;
	float time1 = 0, time2 = 0, deltaT = 0;
	int i = 0;
			time2 = float(glfwGetTime());;

	//main loop, multithread physics and rendering later preferrably --- I am delusional
	while (!glfwWindowShouldClose(window) && loopExit == 0) {

		if (i % 2 == 0) {
			time1 = float(glfwGetTime());;
		}
		else {
			time2 = float(glfwGetTime());;
		};
		deltaT = abs(time2 - time1);

		// run physics calculations on each frame, no returns as values are changed within the physics objects through the objects vector
		//vertsVec = objects[0].getVerts();
		//copy(vertsVec.begin(), vertsVec.end(), verts);
		//objects[0].writeVerts(vertsVec);

		//positionNew = { 0.0, 0.0, 0.0 };

		//cout << boundaries[0] << " " << boundaries[1] << " " << boundaries[2] << " " << endl;
		//cout << positionNew[0] << " " << positionNew[1] << " " << positionNew[2] << " " << endl;
		//objects[0].writeVerts(vertsVec);

		boundaries = { float(width / 40), boundaries[1], float(height / 40) };
		//cout << boundaries[0] << " " << boundaries[1] << " " << boundaries[2] << " " << endl;
		glUniform3f(uniPos, positionNew[0]/boundaries[0], positionNew[2] / boundaries[2], positionNew[1] / boundaries[1]);
		glUniform3f(shaderBoundary, boundaries[0], boundaries[2], boundaries[1]);
		//cout << boundaries[0] << " " << boundaries[1] << " " << boundaries[2] << " " << endl;
		//cout << positionNew[0] << " " << positionNew[1] << " " << positionNew[2] << " " << endl;
		//copy(vertsVec.begin(), vertsVec.end(), verts);
		positionNew = { objects[0].getPos()[0], objects[0].getPos()[1], objects[0].getPos()[2]};
		boundaries = {float(width / 40), boundaries[1], float(height / 40)};
		//cout << boundaries[0] << " " << boundaries[1] << " " << boundaries[2] << " " << endl;
		//cout << boundaries[0] << " " << boundaries[1] << " " << boundaries[2] << " " << endl;
		//cout << positionNew[0] << " " << positionNew[1] << " " << positionNew[2] << " " << endl;

		positionNew = { objects[0].getPos()[0], objects[0].getPos()[2] };
		glUniform2f(uniPos, positionNew[0]/boundaries[0], positionNew[1]/boundaries[2]);
		glUniform2f(shaderBoundary, boundaries[0], boundaries[2]);

		boundaries = {float(width / 40), boundaries[1], float(height / 40)};
		//cout << boundaries[0] << " " << boundaries[1] << " " << boundaries[2] << " " << endl;

		glfwSwapBuffers(window);

		//Sleep(0.1);
		//cout << glGetError() << endl;
	};
}


int main() {

	//init object and push to vector
	PhysObj Sphere;
	Sphere.writePos({ 0, 0, 0 });
	Sphere.writeVel({ 5, 0, 0 });
	Sphere.writeRad(1.5);

	// insert physics objects into objects vector, currently can only handle 1 object
	objects.insert(objects.begin(), Sphere);

	glfwSetErrorCallback(errorCallback);
	if (!glfwInit()) return 0;

	//init window object named windowFunc with title Sample
	GLFWwindow* windowMain = glfwCreateWindow(resx, resz, "Sample", NULL, NULL);
	if (!windowMain) { cout << "failed window creation" << endl; return 0; };

	glfwSetKeyCallback(windowMain, keyCallback);
	glfwMakeContextCurrent(NULL);

	thread Render_Thread(renderThreadFunc, windowMain);
	
	while (!glfwWindowShouldClose(windowMain)) {
		glfwPollEvents();
	};
	
	isRunning = false;
	if (Render_Thread.joinable()) {
		Render_Thread.join();
	};

	glfwDestroyWindow(windowMain);
	glfwTerminate();
	return 0;
};

