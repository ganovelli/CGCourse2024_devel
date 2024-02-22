#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include "..\common\debugging.h"
#include "..\common\renderable.h"
#include "..\common\shaders.h"
#include "..\common\simple_shapes.h"
#include "..\common\matrix_stack.h"

#define TINYGLTF_IMPLEMENTATION
#include "..\common\gltf_loader.h"

/* 
GLM library for math  https://github.com/g-truc/glm 
it's a header-only library. You can just copy the folder glm into 3dparty
and set the path properly. 
*/
#include <glm/glm.hpp>  
#include <glm/ext.hpp>  



//#include <stb_image.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>


void out_mat(glm::mat4 m) {
	std::cout << std::endl;
	for (int i=0; i < 4; ++i) {
		for (int j=0; j < 4; ++j)
			std::cout << m[i][j] << " ";
		std::cout << std::endl;
	}

}

int main(int arcgc, char**argv)
{


    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1000, 800, "code_08_gltf_loader", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    
    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glewInit();

    printout_opengl_glsl_info();

	shader basic_shader;
    basic_shader.create_program("shaders/basic.vert", "shaders/basic.frag");

	check_gl_errors(__LINE__, __FILE__);

	/* create a  cube   centered at the origin with side 2*/
	std::vector <renderable> obj; 
	renderable r_cube = shape_maker::cube();

	gltf_loader gltfL;
	gltfL.load(argv[1]);
	box3 bbox;
	gltfL.create_renderable(obj,bbox);

	glm::mat4 proj = glm::perspective(glm::radians(45.f), 1.f, 0.2f, 20.f);
	glm::mat4 view = glm::lookAt(glm::vec3(0, 1, 2.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

	glUseProgram(basic_shader.program);
	glUniformMatrix4fv(basic_shader["uView"], 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(basic_shader["uProj"], 1, GL_FALSE, &proj[0][0]);

	matrix_stack stack;

	glEnable(GL_DEPTH_TEST);
	float angle = 0;
	/* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
		/* one full rotation every 6 seconds*/
		angle = (60.f*clock() / CLOCKS_PER_SEC);
		
		glUniformMatrix4fv(basic_shader["uRot"], 1, GL_FALSE, &glm::rotate(glm::mat4(1.f), glm::radians(angle), glm::vec3(0.f, 1.f, 0.f))[0][0]);
		stack.load_identity();
		stack.push();
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		stack.push();
		float scale = 1.f / bbox.diagonal();
 		stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(scale, scale, scale)));
		stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(-bbox.center())));
		for (unsigned int i = 0; i < obj.size(); ++i) {
			obj[i].bind();
			stack.push();
			stack.mult(obj[i].transform);
			/*draw the cube tranformed into the car's body*/
			glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
			glUniform3f(basic_shader["uColor"], 1.0, 0.0, 0.0);
			glDrawElements(obj[i]().mode, obj[i]().count, obj[i]().itype, 0);
			stack.pop();
		}
		stack.pop();
		
		//r_cube.bind();
		////stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(0.2, 0.2, 0.2)));
		//glUniform3f(basic_shader["uColor"], 1.0, 0.0, 0.0);
		//glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &glm::mat4(1.f)[0][0]);
		//glDrawElements(r_cube().mode, r_cube().count, r_cube().itype, 0);
		
		stack.pop();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }
	glUseProgram(0);
    glfwTerminate();
    return 0;
}
