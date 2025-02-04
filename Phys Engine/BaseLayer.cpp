#include <iostream>
#include <vector>
#include <array>
#include <Windows.h>
#include <conio.h>
#include <math.h>
using namespace std;

//origin point and velocity storage for generic physics object
class PhysObj {
private:
	// x, y, z format where z is height
	array<float, 3> pos;
	array<float, 3> vel;
	float r;
public:
	PhysObj() {
		pos = { 0,0,0 };
		vel = { 0,0,0 };
		r = 0;
	};

	PhysObj(array<float, 3> posInit, array<float, 3> velInit, float rInit) {
		pos = posInit;
		vel = velInit;
		r = rInit;
	};

	void writePos(array<float, 3> posNew) {
		pos = posNew;
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
};

//environemnt variables, in meters
array<float, 3> boundaries = { 100, 100, 100 };
float t = 0.05f;
float g = 9.81f;
int loopExit = 0;

void mainLoop(int i, vector<PhysObj> objects) {
	//cout << "loop:" << i << "size:" << objects.size() << endl;

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

	if (loopExit == 0) {
		Sleep(t*1000);
		i = i + 1;
		mainLoop(i, objects);
	};
};

int main() {
	//init object and push to vector
	vector<PhysObj> objects;
	PhysObj Sphere;
	Sphere.writePos({ 0, 0, 0 });
	Sphere.writeVel({ 0, 5, 15 });
	Sphere.writer(0.5);

	objects.insert(objects.begin(), Sphere);

	int i = 0;
	mainLoop(i, objects);
};

