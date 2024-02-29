#pragma once
#include "renderable.h"
#include "game.h"



struct game_to_renderable {

	static void to_track(const race & r, renderable& r_t)  {
		
		std::vector<glm::vec3> buffer_pos;
		for (unsigned int i = 0; i <= r.s.t.curbs[0].size();++i) {
			buffer_pos.push_back(r.s.t.curbs[0][i%(r.s.t.curbs[0].size())]);
			buffer_pos.push_back(r.s.t.curbs[1][i%(r.s.t.curbs[0].size())]);
		}

		r_t.add_vertex_attribute<float>(& buffer_pos[0].x, buffer_pos.size() * 3, 0, 3);
	}

	static void to_heightfield(const race& r, renderable& r_hf) {
		std::vector<unsigned int > buffer_id;
		const unsigned int& Y = r.s.height_field.size();
		const unsigned int& X = r.s.height_field[0].size();

		std::vector<float>   hf3d;
		for (int iy = 0; iy < Y; ++iy)
			for (int ix = 0; ix < X; ++ix) {
				hf3d.push_back(-20 + (ix / float(X)) * 40.f);
				hf3d.push_back(r.s.height_field[iy][ix]);
				hf3d.push_back(-20 + (iy / float(Y) * 40.f));
			}

		for (int iy = 0; iy < Y-1; ++iy)
			for (int ix = 0; ix < X-1; ++ix) {
				
				buffer_id.push_back((iy * Y) + ix);
				buffer_id.push_back((iy * Y) + ix + 1);
				buffer_id.push_back((iy + 1) * Y + ix + 1);

				buffer_id.push_back((iy * Y) + ix);
				buffer_id.push_back((iy + 1) * Y + ix + 1);
				buffer_id.push_back((iy + 1) * Y + ix);
			}

		r_hf.add_vertex_attribute<float>(&hf3d[0], X * Y * 3, 0, 3);
		r_hf.add_indices<unsigned int>(&buffer_id[0],  buffer_id.size(), GL_TRIANGLES);
	}

};