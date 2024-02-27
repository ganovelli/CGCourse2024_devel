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

#include <iostream>
#include <algorithm>
#include <conio.h>
#include <direct.h>
#include "..\common\matrix_stack.h"
#include "..\common\intersection.h"
#include "..\common\trackball.h"

trackball tb[2];
int curr_tb;

/* projection matrix*/
glm::mat4 proj;

/* view matrix */
glm::mat4 view;

matrix_stack stack;
float scaling_factor = 1.0;


/* callback function called when the mouse is moving */
static void cursor_position_callback(GLFWwindow * window, double xpos, double ypos)
{
	tb[curr_tb].mouse_move(proj, view, xpos, ypos);
}

/* callback function called when a mouse button is pressed */
void mouse_button_callback(GLFWwindow * window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		tb[curr_tb].mouse_press(proj, view, xpos, ypos);
	}
	else
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
			tb[curr_tb].mouse_release();
		}
}

/* callback function called when a mouse wheel is rotated */
void scroll_callback(GLFWwindow * window, double xoffset, double yoffset)
{
	if (curr_tb == 0)
		tb[0].mouse_scroll(xoffset, yoffset);
}

void key_callback(GLFWwindow * window, int key, int scancode, int action, int mods) {
	/* every time any key is presse it switch from controlling trackball tb[0] to tb[1] and viceversa */
	if (action == GLFW_PRESS)
		curr_tb = 1 - curr_tb;

}



	int main(int argc, char** argv)
	{
		race r;
		game_builder::default_race(r);

		//r.start();
		//while (true) {
		//	r.update();
		//	std::cout << glm::to_string(r.cars[0].frame) << std::endl;
		//}


		GLFWwindow* window;

		/* Initialize the library */
		if (!glfwInit())
			return -1;

		//	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		//	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

			/* Create a windowed mode window and its OpenGL context */
		window = glfwCreateWindow(800, 800, "game", NULL, NULL);
		if (!window)
		{
			glfwTerminate();
			return -1;
		}
		/* declare the callback functions on mouse events */
		if (glfwRawMouseMotionSupported())
			glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

		glfwSetCursorPosCallback(window, cursor_position_callback);
		glfwSetMouseButtonCallback(window, mouse_button_callback);
		glfwSetScrollCallback(window, scroll_callback);
		glfwSetKeyCallback(window, key_callback);

		/* Make the window's context current */
		glfwMakeContextCurrent(window);

		glewInit();

		printout_opengl_glsl_info();

		renderable fr = shape_maker::cube();

		shader basic_shader;
		basic_shader.create_program("shaders/basic.vert", "shaders/basic.frag");

		/* use the program shader "program_shader" */
		glUseProgram(basic_shader.program);


		/* load the shaders */
		
		/* define the viewport  */
		glViewport(0, 0, 800, 800);

		/* avoid rendering back faces */
		// uncomment to see the plane disappear when rotating it
		glDisable(GL_CULL_FACE);

		tb[0].reset();
		tb[0].set_center_radius(glm::vec3(0, 0, 0), 1.f);
		curr_tb = 0;

		proj = glm::perspective(glm::radians(45.f), 1.f, 1.f, 10.f);
		view = glm::lookAt(glm::vec3(0, 3.f, 4.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
		glUniformMatrix4fv(basic_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
		glUniformMatrix4fv(basic_shader["uView"], 1, GL_FALSE, &view[0][0]);

		fr.bind();

		r.start();
		r.update();

		 matrix_stack stack;
		/* Loop until the user closes the window */
		while (!glfwWindowShouldClose(window))
		{
			/* Render here */
			glClearColor(0.0, 0.3, 0.5, 1.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			check_gl_errors(__LINE__, __FILE__);

			r.update();
			stack.load_identity();
			stack.push();
			stack.mult(tb[0].matrix());

			stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(0.01f, 0.01f, 0.01f)));

			stack.push();
			stack.mult(r.cars[0].frame);
	 		glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);

			glDrawElements(fr().mode, fr().count, fr().itype, 0);
			stack.pop();

			//glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
			//glDrawElements(fr().mode, fr().count, fr().itype, 0);

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

 
