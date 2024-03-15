#pragma once
#include "game.h"
#include "path.h"

struct game_loader {
	game_loader() {}

	static int load(const char * svgFile, const char* terrain_image,race & r) {

		r.ter.rect_xz = glm::vec4(-1,-1,101,101);
		r.ter.size_pix = glm::vec2(100, 100);
		r.ter.height_field.resize(100);
		for(unsigned int i =0; i < r.ter.height_field.size();++i)
			r.ter.height_field[i].resize(100,0.f);


		struct NSVGimage* image;
		struct ::NSVGrasterizer* rast = ::nsvgCreateRasterizer();
		image = nsvgParseFromFile("small_test.svg", "px", 96);
		printf("size: %f x %f\n", image->width, image->height);
		
		r.bbox.add(glm::vec3(0.f, 0.f, 0.f));
		r.bbox.add(glm::vec3(image->width, 0.f, image->height));

		for (NSVGshape* shape = image->shapes; shape != NULL; shape = shape->next) {
			printf("id %s\n", shape->id);

			if (std::string(shape->id).find("tree") != std::string::npos) {
				NSVGpath * npath = shape->paths;
				r.trees.push_back(stick_object());
				stick_object& tree = r.trees.back();
				tree.pos = glm::vec3(npath->pts[0],r.ter.y(npath->pts[0], npath->pts[1]), npath->pts[1]);
				tree.height = 2.0;
			}
			else
			if (std::string(shape->id).find("lamps") != std::string::npos) {
				NSVGpath* npath = shape->paths;
				r.lamps.push_back(stick_object());
				stick_object& lamp = r.trees.back();
//				lamp.pos = glm::vec3(npath->pts[0], r.ter.y(npath->pts[0], npath->pts[1]), npath->pts[1]);
				lamp.pos = glm::vec3(npath->pts[0], 0.f, npath->pts[1]);
				lamp.height = 2.0;
			}
			else
				if (std::string(shape->id).find("track") != std::string::npos) {
					int ip = 0;
					std::vector<glm::vec3> samples_pos, samples_tan;
					for (NSVGpath* path = shape->paths; path != NULL; path = path->next) {
						std::vector<glm::vec3> controlPoints;

						for (int i = 0; i < path->npts ; i += 1)  
							controlPoints.push_back(glm::vec3( path->pts[i * 2], 0, path->pts[i * 2 + 1]));
//						controlPoints.push_back(controlPoints[0]);
						bezier_path::regular_sampling(controlPoints, 1.f, samples_pos,samples_tan);
					}

					for (unsigned int i = 0;i < samples_pos.size();++i) {

						glm::vec3 d =glm::vec3 (-samples_tan[i].z, 0, samples_tan[i].x);
						d = glm::normalize(d);
						r.t.curbs[0].push_back(samples_pos[i] + d * 2.f);
						r.t.curbs[1].push_back(samples_pos[i] - d * 2.f);
					}


					
				}
				else
					if (std::string(shape->id).find("car") != std::string::npos) {
						int ip = 0;
					}
		}
		// Delete
		nsvgDelete(image);

		car c;
		c.frame = glm::mat4(1.f);
		c.p.T = 60000; // one minute for one laps
		r.cars.push_back(car());
		for (size_t i = 0; i < 1800; ++i) 
			r.cars[0].p.frames.push_back(glm::mat4(1.f) );
		return 1;

//		// r.height_field ...
//		float rad = 20.f;
//		r.ter.rect_xz = glm::vec4(-30, -30, 60, 60);
//		r.terize_pix = glm::vec2(100, 100);
//		r.ter.height_field.resize(100);
//		for(unsigned int i =0; i < r.ter.height_field.size();++i)
//			r.ter.height_field[i].resize(100);
//
//		for(int iz = 0; iz < 100; ++iz)
//			for (int ix = 0; ix < 100; ++ix) 
//				r.ter.height_field[ix][iz] = 2.0*sin(glm::radians(ix*4.f))* cos(glm::radians(iz * 4.f) );
////				r.ter.height_field[ix][iz] = ix*iz /500.f;
//
//
//		for (size_t i = 0; i < 360; ++i) {
//			glm::mat4 R = glm::rotate(glm::mat4(1.f), glm::radians(float(i)), glm::vec3(0, 1, 0));
//			r.t.curbs[0].push_back(( R * glm::vec4(1.f, 0.f, 0.f,0.f))  * rad);
//			r.t.curbs[1].push_back(( R * glm::vec4(1.f, 0.f, 0.f, 0.f)) * (rad+2.f));
//			float y = r.ter.y(r.t.curbs[0].back().x, r.t.curbs[0].back().z);
//			r.t.curbs[0].back().y =y;
//			y = r.ter.y(r.t.curbs[1].back().x, r.t.curbs[1].back().z);
//			r.t.curbs[1].back().y = y;
//
//		}
//		r.cars.resize(1);
//		r.cars[0].p.T = 60000; // one minute for one laps
//
//		// starting frame of the car
//		glm::mat4 frame(1.f);
//		frame[3] = glm::vec4(rad, 0.0, 0.0, 1.0);
//
//		// 30 frames per second for one minute
//		for (size_t i = 0; i < 1800; ++i) {
//			glm::mat4 R = glm::translate(glm::rotate(glm::mat4(1.f),glm::radians(float(i) / 1800 * 360), glm::vec3(0, 1, 0)), glm::vec3(1, 0, 0));
//			r.cars[0].p.frames.push_back(R * frame );
//			glm::vec4  & orig = r.cars[0].p.frames.back()[3];
//			
//			float y = r.ter.y(orig.x, orig.z);
//			orig.y = y;
//		}
	}
};