#pragma once
#include "game.h"

struct game_builder {
	game_builder() {}

	static void default_race(race& r) {
		// r.s.height_field ...
		float rad = 20.f;

		r.s.height_field.resize(100);
		for(unsigned int i =0; i < r.s.height_field.size();++i) 
			r.s.height_field[i].resize(100);

		for(int iy = 0; iy < 100; ++iy)
			for (int ix = 0; ix < 100; ++ix) 
				r.s.height_field[iy][ix] = sin(glm::radians(ix*20.f))* cos(glm::radians(iy * 20.f) );


		for (size_t i = 0; i < 360; ++i) {
			glm::mat4 R = glm::rotate(glm::mat4(1.f), glm::radians(float(i)), glm::vec3(0, 1, 0));
			r.s.t.curbs[0].push_back(( R * glm::vec4(1.f, 0.f, 0.f,0.f))  * rad);
			r.s.t.curbs[1].push_back(( R * glm::vec4(1.f, 0.f, 0.f, 0.f)) * (rad+2.f));
		}
		r.cars.resize(1);
		r.cars[0].p.T = 60000; // one minute for one laps

		// starting frame of the car
		glm::mat4 frame(1.f);
		frame[3] = glm::vec4(rad, 0.0, 0.0, 1.0);

		// 30 frames per second for one minute
		for (size_t i = 0; i < 1800; ++i) {
			glm::mat4 R = glm::rotate(glm::mat4(1.f), glm::radians(float(i)/1800*360), glm::vec3(0, 1, 0));
			r.cars[0].p.frames.push_back(R * frame );
		}
	}
};