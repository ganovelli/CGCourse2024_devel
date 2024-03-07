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

struct stick_object :public point_object{
	stick_object():height(0){};
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

	// store one frame every 1/30 of second
	// frames[i] is the frame at time i* (1000.f/30.0) in milliseconds
	std::vector<glm::mat4> frames; 

	int T; // how many milliseconds for a lap to complete
};

struct car {

	// local frame of the car. the car front is on -z halfspace
	glm::mat4 frame;

	// box is the bounding box of the car, expressed in "frame"
	box3 box;
	 
	// the path followed by the car
	path p;
};


struct terrain {

	// terrain specified as an height field
	std::vector< std::vector<float> > height_field;

	// rectangle in xz where the terrain is located (minx,miny,sizex,sizey)
	glm::vec4 rect_xz;
	glm::ivec2 size_pix;

	float y(float x, float z) {
		
		int yy = (z - rect_xz[1]) ;
		int xx = (x - rect_xz[0]) ;
		float sy = rect_xz[3] / size_pix[1];
		float sx = rect_xz[1] / size_pix[0];
		// continue add interpolation

		return height_field[(z - rect_xz[1]) / rect_xz[3] * size_pix[1]][(x - rect_xz[0]) / rect_xz[2] * size_pix[0]];
	}

};

struct scene {

	terrain ter;
	track t;

	glm::vec3 sunlight_direction;
	std::vector<stick_object> trees;
	std::vector<stick_object> lamps;
	std::vector<stick_object> photographers;
};


struct race {
	race():sim_time_ratio(60){}

	scene s;
	int clock_start;

	// simulation sunlight time in milliseconds
	int sim_time;

	// how long a real second in simulated sunlight time
	int sim_time_ratio; 

	void start( int h = -1, int m = -1, int s = -1, int _sim_time_ratio = 60) {
		clock_start = clock();
		if (h != -1) 
			sim_time = (s + m * 60 + h * 3600) * 1000;
		else
			sim_time = ( 10 * 3600) * 1000; // start at ten in the morning
		if (_sim_time_ratio != 60)
			sim_time_ratio = _sim_time_ratio;
	}

	void update() {
		for (size_t i = 0; i < cars.size();++i) {
			int ii = ((int)((clock() - clock_start) / 1000.f * 30.f)) % cars[i].p.frames.size();
			std::cout << ii << std::endl;
			cars[i].frame = cars[i].p.frames[ii];
		}
	}

	std::vector<car> cars;
};


