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
array<float, 3> baseboundaries = { 20, 20, 20 };
float t = 0.001f;
float g = 9.81f;
float fM_PI = float(M_PI);
int loopExit = 0;
int resx = 1280, resz = 720;

array<float, 3> boundaries = { float(resx / 40), baseboundaries[1], float(resz / 40)};

//origin point and velocity storage for generic physics object
class PhysObj {
private:
	// x, y, z format where z is height (for now)
	array<float, 3> pos;
	array<float, 3> vel;
	vector<float> verts;
	float rad;
public:
	PhysObj() {
		pos = { 0,0,0 };
		vel = { 0,0,0 };
		rad = 0.5f;
		verts = {  + 0.1f / 20,  + 0.1f / 20, 0.0f, 1.0f, 0.0f, 0.0f,
				   + 0.1f / 20,  - 0.1f / 20, 0.0f, 0.0f, 1.0f, 0.0f,
				   - 0.1f / 20,  - 0.1f / 20, 0.0f, 0.0f, 0.0f, 1.0f,
				   - 0.1f / 20,  + 0.1f / 20, 0.0f, 1.0f, 1.0f, 1.0f };
	};

	PhysObj(array<float, 3> posInit, array<float, 3> velInit, float radInit) {
		pos = posInit;
		vel = velInit;
		rad = radInit;
	};

	void writePos(array<float, 3> posNew) {
		pos = posNew;
		//verts = { (pos[0]) + 0.1f / 20, (pos[1]) + 0.1f / 20, (pos[2]), verts[3] , verts[4] , verts[5] ,
		//		  (pos[0]) + 0.1f / 20, (pos[1]) - 0.1f / 20, (pos[2]), verts[8] , verts[9] , verts[10],
		//		  (pos[0]) - 0.1f / 20, (pos[1]) - 0.1f / 20, (pos[2]), verts[13], verts[14], verts[15],
		//		  (pos[0]) - 0.1f / 20, (pos[1]) + 0.1f / 20, (pos[2]), verts[18], verts[19], verts[20] };
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
			else if (Axis == 2) {
				// only do gravity if collision gravity has not occured as it performs its own gravity
				pos[2] = pos[2] + vel[2] * deltaT;
				vel[2] = vel[2] - g * deltaT;
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

const GLchar* fragmentSource = R"glsl(
	#version 460 core
	
	in vec3 colorFrag;

	out vec4 colorOut;

	void main() {
		vec3 color;
		color = colorFrag;
		//color = (1.0f, 1.0f, 1.0f) - colorFrag;
		colorOut = vec4(color, 1.0f);
	};
)glsl";


void errorCallback(int error, const char* description) {
	fprintf(stderr, "Error: %s\n", description);
};

int main() {

	glfwSetErrorCallback(errorCallback);
	if (!glfwInit()) { return 0; };

	//init window object named window with title Sample
	GLFWwindow* window = glfwCreateWindow(resx, resz, "Sample", NULL, NULL);
	if (!window) { cout << "failed window creation" << endl; return 0; };

	//set escape key callback and current context
	glfwSetKeyCallback(window, keyCallback);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);
	gladLoadGL();

	//init object and push to vector
	PhysObj Sphere;
	Sphere.writePos({ 0, 0, 0 });
	Sphere.writeVel({ 5, 0, 0 });
	Sphere.writeRad(1.5);

	objects.insert(objects.begin(), Sphere);
	float verts[24];
	vector<float> vertsVec = objects[0].getVerts();
	copy(vertsVec.begin(), vertsVec.end(), verts);

	for (int l = 0; l < sizeof(verts) / sizeof(float); l++) {
		cout << verts[l] << " ";
	};
	cout << endl;

	//truthfully no clue how any of this works really
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

	//int  success;
	//char infoLog[512];

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	/*glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	};*/

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	/*glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	};*/

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "colorOut");
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), 0);

	GLint colAttrib = glGetAttribLocation(shaderProgram, "colorVert");
	glEnableVertexAttribArray(colAttrib);
	glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));

	cout << glGetError() << endl;

	GLint uniPos = glGetUniformLocation(shaderProgram, "positionSphere");
	GLint shaderBoundary = glGetUniformLocation(shaderProgram, "boundaries");

	array<float, 3> positionNew;
	float time1 = 0, time2 = 0, deltaT = 0;
	int i= 0;
	int width, height;

	//main loop, multithread physics and rendering later preferrably --- I am delusional
	while (!glfwWindowShouldClose(window) && loopExit == 0) {

		if (i % 2 == 0) {
			time1 = float(glfwGetTime());;
		}
		else {
			time2 = float(glfwGetTime());;
		};
		deltaT = abs(time2 - time1);
		i++;
		//cout << deltaT << endl;

		//draw call and frame size
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
		glDrawElements(GL_TRIANGLES, sizeof(elements) / sizeof(GLuint), GL_UNSIGNED_INT, 0);

		physicsSim(deltaT);
		//vertsVec = objects[0].getVerts();
		//copy(vertsVec.begin(), vertsVec.end(), verts);
		//objects[0].writeVerts(vertsVec);

		//positionNew = { 0.0, 0.0, 0.0 };
		positionNew = { objects[0].getPos()[0], objects[0].getPos()[1], objects[0].getPos()[2]};
		glUniform3f(uniPos, positionNew[0]/boundaries[0], positionNew[2] / boundaries[2], positionNew[1] / boundaries[1]);
		glUniform3f(shaderBoundary, boundaries[0], boundaries[2], boundaries[1]);
		//cout << boundaries[0] << " " << boundaries[1] << " " << boundaries[2] << " " << endl;
		//cout << positionNew[0] << " " << positionNew[1] << " " << positionNew[2] << " " << endl;


		boundaries = {float(width / 40), boundaries[1], float(height / 40)};
		//cout << boundaries[0] << " " << boundaries[1] << " " << boundaries[2] << " " << endl;

		glfwPollEvents();
		glfwSwapBuffers(window);

		//Sleep(0.1);
		//cout << glGetError() << endl;
	};

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
};

//set up multithreaded window