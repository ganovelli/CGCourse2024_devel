#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include "..\common\debugging.h"
#include "..\common\renderable.h"
#include "..\common\shaders.h"
#include "..\common\simple_shapes.h"

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

int main(void)
{


    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1000, 800, "code_05_my_first_car", NULL, NULL);
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
	renderable r_cube; 
	//r_cube = shape_maker::cube();

	gltf_loader gltfL;
	gltfL.load("torus.glb");
	gltfL.create_renderable(r_cube);

	glm::mat4 proj = glm::perspective(glm::radians(45.f), 1.33f, 0.1f, 100.f);
	glm::mat4 view = glm::lookAt(glm::vec3(0, 5, 10.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

	glUseProgram(basic_shader.program);

	glEnable(GL_DEPTH_TEST);
	float angle = 0;
	/* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
		/* one full rotation every 6 seconds*/
		angle = (60.f*clock() / CLOCKS_PER_SEC);

		glm::mat4 R = glm::rotate(glm::mat4(1.f), glm::radians(angle), glm::vec3(0.f, 1.f, 0.f));

        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		glUniformMatrix4fv(basic_shader["uView"], 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(basic_shader["uRot"], 1, GL_FALSE, &R[0][0]);

        glUseProgram(basic_shader.program);
		glUniformMatrix4fv(basic_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
		
		/* render box and cylinders so that the look like a car */
		r_cube.bind();

		/*draw the cube tranformed into the car's body*/
		glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &glm::mat4(1.f)[0][0]);
		glUniform3f(basic_shader["uColor"], 1.0,0.0,0.0);
		glDrawElements(r_cube().mode, r_cube().count, r_cube().itype, 0);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }
	glUseProgram(0);
    glfwTerminate();
    return 0;
}
