#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include "..\common\debugging.h"
#include "..\common\renderable.h"
#include "..\common\shaders.h"
#include "..\common\simple_shapes.h"
#include "..\common\game.h"
#include "..\common\game_builder.h"

int main(void)
{

	race r;
	game_builder::default_race(r);

	r.start();
	while (true) {
		r.update();

		std::cout << glm::to_string(r.cars[0].frame) << std::endl;

	}
 //   GLFWwindow* window;

 //   /* Initialize the library */
 //   if (!glfwInit())
 //       return -1;

 //   /* Create a windowed mode window and its OpenGL context */
 //   window = glfwCreateWindow(1000, 800, "indexed", NULL, NULL);
 //   if (!window)
 //   {
 //       glfwTerminate();
 //       return -1;
 //   }

 //   
 //   /* Make the window's context current */
 //   glfwMakeContextCurrent(window);

 //   glewInit();

 //   printout_opengl_glsl_info();

 //   /* define a cube */
 //   shape cube;
 //   shape_maker::cube(cube, 0.0f, 1.0f, 0.0f);

 //   renderable r;
 //   cube.to_renderable(r);

 //   shader basic_shader;
 //   basic_shader.create_program("shaders/basic.vert", "shaders/basic.frag");
 //  
	///* use the program shader "program_shader" */
	//glUseProgram(basic_shader.program);

 //   /* use the array and element buffers */
 //   r.bind();

 //   glClearColor(1, 0, 0,1);
 //   /* Loop until the user closes the window */
 //   while (!glfwWindowShouldClose(window))
 //   {
 //       /* Render here */
 //       glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

 //       //glDrawArrays(GL_TRIANGLES, 0, 3);
 //       glDrawElements(r().mode, r().count, r().itype, NULL);
 //       check_gl_errors(__LINE__,__FILE__);
 //
 //       /* Swap front and back buffers */
 //       glfwSwapBuffers(window);

 //       /* Poll for and process events */
 //       glfwPollEvents();
 //   }
	//glUseProgram(0);

 //   glfwTerminate();
 //   return 0;

}
