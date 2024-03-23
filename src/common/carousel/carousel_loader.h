#pragma once
#include "carousel.h"
#include "..\path.h"

struct game_loader {
	game_loader() {}

	static race* & r() {   static race * r; return r; }

	static void push_stick_object(NSVGpath* npath,float h, std::vector<stick_object> & vso) {
		stick_object  so; 
		so.pos = glm::vec3(npath->pts[0], r()->ter.y(npath->pts[0], npath->pts[1]), npath->pts[1]);
		so.height = 2.0;
		vso.push_back(so);
	}

	static void push_cameraman(NSVGpath* npath, float rd, std::vector<cameraman>& vc) {
		cameraman  so;
		so.frame = glm::mat4(1.f);
		so.frame[3] = glm::vec4(npath->pts[0], r()->ter.y(npath->pts[0], npath->pts[1]), npath->pts[1], 1.0);
		so.locked = false;
		so.radius = rd;
		vc.push_back(so);
	}

	static void regular_sampling(const NSVGpath * path, double delta, std::vector<glm::vec3>& samples_pos, std::vector<glm::vec3>& samples_tan, float* tot = 0) {
		int ip = 0;
		std::vector<glm::vec3> controlPoints;
		for (int i = 0; i < path->npts; i += 1)
			controlPoints.push_back(glm::vec3(path->pts[i * 2], 0, path->pts[i * 2 + 1]));
		bezier_path::regular_sampling(controlPoints, 0.1f, samples_pos, samples_tan);
	}

	static int load(const char * svgFile, const char* terrain_image,race & r) {
		game_loader::r() = &r;

		int sx, sy,comp;
		unsigned char* data = stbi_load( terrain_image, &sx, &sy, &comp, 1);

		r.ter.size_pix[0] = sx;
		r.ter.size_pix[1] = sy;

		//stbi_write_png("read_texture.png", sx, sy, 1, data, 0);

		r.ter.height_field.resize(sx*sy);
		memcpy_s(&r.ter.height_field[0], sx * sy , data, sx * sy );

		struct NSVGimage* image;
		struct ::NSVGrasterizer* rast = ::nsvgCreateRasterizer();
		image = nsvgParseFromFile("small_test.svg", "px", 96);
		printf("size: %f x %f\n", image->width, image->height);
		
		r.bbox.add(glm::vec3(0.f, 0.f, 0.f));
		r.bbox.add(glm::vec3(image->width, 0.f, image->height));
		r.ter.rect_xz = glm::vec4(0, 0, image->width, image->height);

		for (NSVGshape* shape = image->shapes; shape != NULL; shape = shape->next) {
			printf("id %s\n", shape->id);

			if (std::string(shape->id).find("tree") != std::string::npos)  
				push_stick_object(shape->paths, 2.f, r.trees);
			else
			if (std::string(shape->id).find("lamp") != std::string::npos) 
				push_stick_object(shape->paths, 2.f, r.lamps);
			else
			if (std::string(shape->id).find("cameraman") != std::string::npos)
				push_cameraman(shape->paths, 15.f, r.cameramen);
			else
				if (std::string(shape->id).find("track") != std::string::npos) {
					int ip = 0;
					std::vector<glm::vec3> samples_pos, samples_tan;
					regular_sampling(shape->paths, 0.1f, samples_pos, samples_tan);

					for (unsigned int i = 0;i < samples_pos.size();++i) {
						glm::vec3 d =glm::vec3 (-samples_tan[i].z, 0, samples_tan[i].x);
						d = glm::normalize(d);
						r.t.curbs[0].push_back(r.ter.p(samples_pos[i] + d * 2.f));
						r.t.curbs[1].push_back(r.ter.p(samples_pos[i] - d * 2.f));
					}
					
				}
				else
					if (std::string(shape->id).find("carpath") != std::string::npos) {
						int ip = 0;
						std::vector<glm::vec3> samples_pos, samples_tan;
						for (NSVGpath* path = shape->paths; path != NULL; path = path->next) {
							int ip = 0;
							std::vector<glm::vec3> samples_pos, samples_tan;

							float tot_length;
							regular_sampling(shape->paths, 1.f, samples_pos, samples_tan, &tot_length);
							samples_pos.clear();
							samples_tan.clear();

							float delta = tot_length / 60000 * 33.f;
							regular_sampling(shape->paths, delta, samples_pos, samples_tan, &tot_length);

							r.carpaths.push_back(::path());
							for (unsigned int i = 0; i < samples_pos.size(); ++i) {
								r.carpaths.back().frames.push_back(glm::mat4(1.f));
								r.carpaths.back().frames.back()[3] = glm::vec4(r.ter.p(samples_pos[i]), 1.0);
								glm::vec3 tn = glm::normalize(samples_tan[i]);
								glm::vec3  z = glm::normalize(r.ter.p(samples_pos[i] - tn) -  r.ter.p(samples_pos[i]));

								glm::vec3 d = glm::vec3(-samples_tan[i].z, 0, samples_tan[i].x);
								d = glm::normalize(d);
								glm::vec3  x = glm::normalize(r.ter.p(samples_pos[i] + d) - r.ter.p(samples_pos[i]));
								glm::vec3 y = glm::cross(z, x);
								r.carpaths.back().frames.back()[0] = glm::vec4(x, 0); 
								r.carpaths.back().frames.back()[1] = glm::vec4(y, 0);
								r.carpaths.back().frames.back()[2] = glm::vec4(z, 0);

							}

							//
						}

					}
		}

		nsvgDelete(image);
		return 1;
	}
};