#pragma once

#include <vector>
#include <time.h> 
#include <glm/glm.hpp>  
#include <glm/ext.hpp>  
#include <glm/gtx/string_cast.hpp>
#include "box3.h"


struct point_object {
	point_object():pos(0.f) {}
	glm::vec3 pos;
};

struct region_object {
	region_object() {}
	std::vector<glm::vec3> contour;
};

struct lamp :public point_object{
	lamp():height(0){};
	float height;
};

struct tree :public point_object {
	tree():height(0) {};
	float height;
};

struct photographer :public point_object {
	photographer() : height(0) {};
	float height;
};


 
struct track {
	track() {}
	std::vector<glm::vec3> curbs[2]; 

	void compute_length() {
		length = 0.f;
		for (size_t i = 0; i < curbs[0].size();++i)  
			length += glm::length(curbs[0][(i + 1) % curbs[0].size()] - curbs[0][i]);
	}

	float length;
};

struct path {

	std::vector<glm::mat4> frames; // frames[i] is the frame at time i* (1000.f/30.0) in milliseconds

	int T; // how many milliseconds for a lap to complete
};

struct car {

	// local frame of the car. the car front is on -z halfspace
	glm::mat4 frame;

	// box is the bounding box of the car, expressed in "frame"
	box3 box;
	 
	path p;
};


struct scene {

	// terrain specified as an height field
	std::vector< std::vector<float> > height_field;

	track t;

	glm::vec3 sunlight_direction;
	std::vector<tree> trees;
	std::vector<tree> lamps;
	std::vector<tree> photographers;
};


struct race {
	race():sim_time_ratio(60){}

	scene s;
	int clock_start;
	int sim_time;

	int sim_time_ratio; // how long a real second in simulated sunlight time

	void start( int h = -1, int m = -1, int s = -1, int _sim_time_ratio = 60) {
		clock_start = clock();
		if (h != -1) 
			sim_time = (s + m * 60 + h * 3600) * 1000;
		else
			sim_time = ( 10 * 3600) * 1000; // start at then in the morning
		if (_sim_time_ratio != 60)
			sim_time_ratio = _sim_time_ratio;
	}

	void update() {
		for (size_t i = 0; i < cars.size();++i) {
			cars[i].frame = cars[i].p.frames[((clock() - clock_start) /1000* 30) % cars[i].p.frames.size()];
		}
	}

	std::vector<car> cars;
};


