#pragma once
#include <tinygltf/tiny_gltf.h>
#include "renderable.h"
#include "texture.h"
#include "debugging.h"

struct gltf_loader {

	tinygltf::Model model;
	tinygltf::TinyGLTF loader;

	std::string err;
	std::string warn;

    std::vector<texture> positions;
    int n_vert, n_tri;
 

    static std::string GetFilePathExtension(const std::string& FileName) {
        if (FileName.find_last_of(".") != std::string::npos)
            return FileName.substr(FileName.find_last_of(".") + 1);
        return "";
    }

    void load(std::string input_filename) {
        std::string ext = GetFilePathExtension(input_filename);

        bool ret = false;
        if (ext.compare("glb") == 0) {
            // assume binary glTF.
            ret =
                loader.LoadBinaryFromFile(&model, &err, &warn, input_filename.c_str());
        }
        else {
            // assume ascii glTF.
            ret = loader.LoadASCIIFromFile(&model, &err, &warn, input_filename.c_str());
        }

        if (!warn.empty()) {
            printf("Warn: %s\n", warn.c_str());
        }

        if (!err.empty()) {
            printf("ERR: %s\n", err.c_str());
        }
        if (!ret) {
            printf("Failed to load .glTF : %s\n", input_filename.c_str());
            exit(-1);
        }
    }

    template <class type>
    void copy_triplet(int* dst, type* src) {
        *(int*)&dst[0] = (int)*((type*)&src[0]);
        *(int*)&dst[1] = (int)*((type*)&src[1]);
        *(int*)&dst[2] = (int)*((type*)&src[2]);
    }

    // take a model and fill the buffers to be passed to the compute shader (for ray tracing)
    bool create_renderable( std::vector<renderable> & rs, box3 & bbox) {

        unsigned char* _data_vert[2] = { 0,0 };
        unsigned char * _data = 0;
        int texture_height, max_texture_width = 2048;
        GLuint  texId;

        tinygltf::Mesh* mesh_ptr = 0;
        assert(model.scenes.size() > 0);

        // just look for the first mesh 
        int scene_to_display = model.defaultScene > -1 ? model.defaultScene : 0;
        const tinygltf::Scene& scene = model.scenes[scene_to_display];
        for (size_t i = 0; i < model.nodes.size(); i++) 
			if (model.nodes[i].mesh > -1) {
				mesh_ptr = &model.meshes[model.nodes[i].mesh];

				rs.push_back(renderable());
				renderable & r = rs.back();

				r.create();
				 
				const std::vector<double> & m = model.nodes[i].matrix;
				if(!m.empty())
					r.transform = glm::mat4(m[0], m[1], m[2], m[3],
										m[4], m[5], m[6], m[7],
										m[8], m[9], m[10], m[11],
										m[12], m[13], m[14], m[15]);

				tinygltf::Mesh & mesh = *mesh_ptr;
				for (size_t i = 0; i < mesh.primitives.size(); i++) {
					const tinygltf::Primitive& primitive = mesh.primitives[i];

					if (primitive.indices < 0) return false;

					std::map<std::string, int>::const_iterator it(primitive.attributes.begin());
					std::map<std::string, int>::const_iterator itEnd(primitive.attributes.end());

					for (; it != itEnd; it++) {
						assert(it->second >= 0);
						const tinygltf::Accessor& accessor = model.accessors[it->second];
						int n_chan = 1;
						if (accessor.type == TINYGLTF_TYPE_SCALAR) {
							n_chan = 1;
						}
						else if (accessor.type == TINYGLTF_TYPE_VEC2) {
							n_chan = 2;
						}
						else if (accessor.type == TINYGLTF_TYPE_VEC3) {
							n_chan = 3;
						}
						else if (accessor.type == TINYGLTF_TYPE_VEC4) {
							n_chan = 4;
						}
						else {
							assert(0);
						}
						// it->first would be "POSITION", "NORMAL", "TEXCOORD_0", ...
						int attr_index = -1;
						if (it->first.compare("POSITION") == 0) attr_index = 0;
						if (it->first.compare("COLOR") == 0)    attr_index = 1;
						if (it->first.compare("NORMAL") == 0)   attr_index = 2;

						if (attr_index != -1) {

							// Compute byteStride from Accessor + BufferView combination.
							int byteStride =
								accessor.ByteStride(model.bufferViews[accessor.bufferView]);
							assert(byteStride != -1);

							n_vert = accessor.count;

							int buffer = model.bufferViews[accessor.bufferView].buffer;
							int bufferviewOffset = model.bufferViews[accessor.bufferView].byteOffset;

							r.add_vertex_attribute<float>((float*)& model.buffers[buffer].data[bufferviewOffset + accessor.byteOffset], 3 * n_vert, attr_index, 3);

							// if the are the position compute the object bounding box
							if (attr_index == 0) {
								float * v_ptr = (float*)& model.buffers[buffer].data[bufferviewOffset + accessor.byteOffset];
								for (unsigned int iv = 0; iv < n_vert; ++iv)
									r.bbox.add(glm::vec3(*(v_ptr + iv * 3), *(v_ptr + iv * 3 + 1), *(v_ptr + iv * 3 + 2)));
								bbox.add(r.bbox);
							}
						}
					}

					const tinygltf::Accessor& indexAccessor =
						model.accessors[primitive.indices];

					int mode = -1;
					if (primitive.mode == TINYGLTF_MODE_TRIANGLES) {
						mode = GL_TRIANGLES;
					}
					//else if (primitive.mode == TINYGLTF_MODE_TRIANGLE_STRIP) {
					//    mode = GL_TRIANGLE_STRIP;
					//}
					//else if (primitive.mode == TINYGLTF_MODE_TRIANGLE_FAN) {
					//    mode = GL_TRIANGLE_FAN;
					//}
					//else if (primitive.mode == TINYGLTF_MODE_POINTS) {
					//    mode = GL_POINTS;
					//}
					//else if (primitive.mode == TINYGLTF_MODE_LINE) {
					//    mode = GL_LINES;
					//}
					//else if (primitive.mode == TINYGLTF_MODE_LINE_LOOP) {
					//    mode = GL_LINE_LOOP;
					//}
					else {
						assert(0);
					}

					// Compute byteStride from Accessor + BufferView combination.
					int byteStride =
						indexAccessor.ByteStride(model.bufferViews[indexAccessor.bufferView]);
					assert(byteStride != -1);

					// one long texture, just a stub implementation
					int buffer = model.bufferViews[indexAccessor.bufferView].buffer;
					int bufferviewOffset = model.bufferViews[indexAccessor.bufferView].byteOffset;

					n_tri = indexAccessor.count / 3;

					check_gl_errors(__LINE__, __FILE__);
					switch (indexAccessor.componentType) {
					case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:     r.add_indices<unsigned char>((unsigned char*)&model.buffers[buffer].data[bufferviewOffset + indexAccessor.byteOffset], indexAccessor.count, GL_TRIANGLES); break;
					case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:    r.add_indices<unsigned short>((unsigned short*)&model.buffers[buffer].data[bufferviewOffset + indexAccessor.byteOffset], indexAccessor.count, GL_TRIANGLES); break;
					case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:      r.add_indices<unsigned int>((unsigned int*)&model.buffers[buffer].data[bufferviewOffset + indexAccessor.byteOffset], indexAccessor.count, GL_TRIANGLES); break;
					}

					check_gl_errors(__LINE__, __FILE__);
				}
			//	return true;
			}
        return true;
    }

};