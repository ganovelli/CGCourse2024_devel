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
		const unsigned int& Z = r.s.ter.height_field.size();
		const unsigned int& X = r.s.ter.height_field[0].size();

		terrain ter = r.s.ter;

		std::vector<float>   hf3d;
		for (int iz = 0; iz < Z; ++iz)
			for (int ix = 0; ix < X; ++ix) {
				hf3d.push_back(ter.rect_xz[0] + (ix / float(X)) * ter.rect_xz[2]);
				hf3d.push_back(r.s.ter.height_field[ix][iz]);
				hf3d.push_back(ter.rect_xz[1] + (iz / float(Z)) * ter.rect_xz[3]);
			}

		for (int iz = 0; iz < Z-1; ++iz)
			for (int ix = 0; ix < X-1; ++ix) {
				
				buffer_id.push_back((iz * Z) + ix);
				buffer_id.push_back((iz * Z) + ix + 1);
				buffer_id.push_back((iz + 1) * Z + ix + 1);

				buffer_id.push_back((iz * Z) + ix);
				buffer_id.push_back((iz + 1) * Z + ix + 1);
				buffer_id.push_back((iz + 1) * Z + ix);
			}

		r_hf.add_vertex_attribute<float>(&hf3d[0], X * Z * 3, 0, 3);
		r_hf.add_indices<unsigned int>(&buffer_id[0],  buffer_id.size(), GL_TRIANGLES);
	}

};