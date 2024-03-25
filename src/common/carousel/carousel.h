#pragma once

#include <vector>
#include <time.h> 
#include <glm/glm.hpp>  
#include <glm/ext.hpp>  
#include <glm/gtx/string_cast.hpp>
#include "..\box3.h"

struct game_loader;
class race;

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

struct cameraman {
	glm::mat4 frame;

	// lock if a car is closer than radius
	float radius;

	// which car is looking at
	int target_car;
	
	// is looking to  a car
	bool locked;
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

class race;

struct car {
	friend race;

	// local frame of the car. the car front is on -z halfspace
	glm::mat4 frame;

	// box is the bounding box of the car, expressed in "frame" coordinates
	box3 box;
	 
private:
	// on which path is the car moving
	int id_path;

	// starting point
	int delta_i;
};


struct  terrain {
	
	// terrain specified as an height field
	std::vector<unsigned char> height_field;

	// rectangle in xz where the terrain is located (minx,miny,sizex,sizey)
	glm::vec4 rect_xz;

	// size in pixels of the height field image
	glm::ivec2 size_pix;


	float  hf(const unsigned int i, const unsigned int j) const {
		return height_field[ (size_pix[0]-1-i) * size_pix[0] + j]/50.f;
	}

 
	// given a 3d point, return its orthogonal projection into the terrain (that is, along the y direction)
	glm::vec3  p(glm::vec3 p_in) {
		return glm::vec3(p_in.x, y(p_in.x, p_in.z), p_in.z);
	}

	// given the x and z coordinates, returns the height of the terrain
	float y(float x, float z) {
		float yy = (z - rect_xz[1]) ;
		float xx = (x - rect_xz[0]) ;
		float sx = rect_xz[2] / size_pix[0];
		float sy = rect_xz[3] / size_pix[1];

		float i_min = xx / sx;
		float j_min = yy / sy;

		int i = static_cast<int>(floor(i_min));
		int j = static_cast<int>(floor(j_min));

		float u = i_min - i;
		float v = j_min - j;

		float value =	hf(i	, j		) * (1.f - u) * (1.f - v) +
						hf(i	, j + 1 ) * (1.f - u) * v +
						hf(i + 1, j		) * u * (1.f - v) +
						hf(i + 1, j + 1 ) * u * v;
		return	value;
	}

};


class race {
	friend game_loader;
public:
	race():sim_time_ratio(60){}

	terrain ter;
	
	track t;

	box3 bbox;
	glm::vec3 sunlight_direction;
	std::vector<stick_object> trees;
	std::vector<stick_object> lamps;
	std::vector<cameraman> cameramen;
	std::vector<car> cars;

private:

	std::vector<path> carpaths;

	int clock_start;

	// simulation sunlight time in milliseconds
	int sim_time;

	// how long a real second in simulated sunlight time
	int sim_time_ratio; 

public:

	void start( int h = -1, int m = -1, int s = -1, int _sim_time_ratio = 60) {
		clock_start = clock();
		if (h != -1) 
			sim_time = (s + m * 60 + h * 3600) * 1000;
		else
			sim_time = ( 10 * 3600) * 1000; // start at ten in the morning
		if (_sim_time_ratio != 60)
			sim_time_ratio = _sim_time_ratio;
	}

	void add_car(int id_path, float delta = -1) {
		if (id_path >= carpaths.size()) {
			std::cout << "car path > " << carpaths.size() - 1 << "\n";
			exit(-1);
		}
		car c; 
		c.box.add(glm::vec3(-1, 1.5, -2));
		c.box.add(glm::vec3( 1, 0,    2));
		c.id_path = id_path;
		 
		c.delta_i = (int) floor(((delta == -1) ? rand() / float(RAND_MAX):delta) * (carpaths[id_path].frames.size() - 2));

		cars.push_back(c);
	}

	void add_car(float delta = -1) {
		int id = (int) floor((rand() / float(RAND_MAX)) *  carpaths.size());
		add_car(id,delta);
	}

	void update() {
		int cs = clock() - clock_start;
		for (size_t i = 0; i < cars.size();++i) {
			int ii = ((int)((cs) / 1000.f * 30.f)+ cars[i].delta_i) % carpaths[cars[i].id_path].frames.size();
			//std::cout << ii << std::endl;
			cars[i].frame = carpaths[cars[i].id_path].frames[ii];
		}
		int day_ms = 3600000 * 24;
		 
		int daytime = (  this->sim_time + cs * sim_time_ratio) % (day_ms);
		glm::mat4 R = glm::rotate(glm::mat4(1.f), glm::radians(360.f * daytime / float(day_ms)), glm::vec3(1, 0, 0));
		sunlight_direction = R * glm::vec4(0.f, -1.f, 0.f,0.f);

		// update cameramen frames
		for (unsigned int ic = 0; ic < cameramen.size(); ++ic) {
			cameraman& c = cameramen[ic];
			glm::vec3 cp = *(glm::vec3*)&c.frame[3];

			if (!c.locked) {
				for (unsigned int ica = 0; ica < cars.size(); ++ica)
					if (glm::length(cp - *(glm::vec3*)&cars[ica].frame[3]) < c.radius) {
						c.target_car = ica;
						c.locked = true;
						break;
					}
			}

			if (c.locked) {
				glm::vec3 tcp = *(glm::vec3*)&cars[c.target_car].frame[3];
				if (glm::length(cp - tcp) < c.radius)
				{
					c.frame = glm::lookAt(cp, tcp, glm::vec3(0, 1, 0));
					c.frame = glm::inverse(c.frame);
				}
				else
					c.locked = false;
			}
		}
	}

};


