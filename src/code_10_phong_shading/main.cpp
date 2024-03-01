#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <algorithm>
#include "..\common\debugging.h"
#include "..\common\renderable.h"
#include "..\common\shaders.h"
#include "..\common\simple_shapes.h"
#include "..\common\matrix_stack.h"
#include "..\common\intersection.h"
#include "..\common\trackball.h"


#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#include "..\common\gltf_loader.h"

/*
GLM library for math  https://github.com/g-truc/glm
it's a header-only library. You can just copy the folder glm into 3dparty
and set the path properly.
*/
#include <glm/glm.hpp>  
#include <glm/ext.hpp>  
#include <glm/gtx/string_cast.hpp>

int width, height;

/* light direction in world space*/
glm::vec4 Ldir;

trackball tb[2];
int curr_tb;

/* projection matrix*/
glm::mat4 proj;


/* view matrix and view_frame*/
glm::mat4 view, view_frame;


glm::vec2 viewport_to_view(float pX, float pY) {
	glm::vec2 res;
	res.x = (pX / float(width)) * 2.f - 1.f;
	res.y = ((height - pY) / float(height)) * 2.f - 1.f;
	return res;
}


/* callback function called when the mouse is moving */
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	tb[curr_tb].mouse_move(proj, view, xpos, ypos);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (curr_tb == 0)
		tb[0].mouse_scroll(xoffset, yoffset);
}

void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	/* every time any key is presse it switch from controlling trackball tb[0] to tb[1] and viceversa */
	if (action == GLFW_PRESS)
		curr_tb = 1 - curr_tb;
}

void window_size_callback(GLFWwindow* window, int _width, int _height)
{
	width = _width;
	height = _height;
	glViewport(0, 0, width, height);
	proj = glm::perspective(glm::radians(40.f), width / float(height), 1.5f, 20.f);
}

int main(int argc , char ** argv)
{
	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	glfwWindowHint(GLFW_SAMPLES, 4);
	/* Create a windowed mode window and its OpenGL context */
	width = 1000;
	height = 800;
	window = glfwCreateWindow(width, height, "code_10_phong_shading", NULL, NULL);
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
	glfwSetKeyCallback(window, keyboard_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	glewInit();
	glEnable(GL_MULTISAMPLE);

	printout_opengl_glsl_info();

	// declare a gltf_loader
	gltf_loader gltfL;

	box3 bbox;
	std::vector <renderable> obj;

	// load a gltf scene into a vector of objects of type renderable "obj"
	// alo return a box containing the whole scene
	gltfL.load_to_renderable(argv[1], obj, bbox);


	/* load the shaders */
	shader basic_shader;
	basic_shader.create_program("shaders/basic.vert", "shaders/basic.frag");
	shader flat_shader;
	flat_shader.create_program("shaders/flat.vert", "shaders/flat.frag");

	/* Set the uT matrix to Identity */
	glUseProgram(basic_shader.program);
	//	glUniformMatrix4fv(basic_shader["uT"], 1, GL_FALSE, &glm::mat4(1.0)[0][0]);
	glUseProgram(0);

	/* create a  quad   centered at the origin with side 2*/
	renderable r_quad = shape_maker::quad();


	/* create a  cube   centered at the origin with side 2*/
	renderable r_cube = shape_maker::cube(0.5f, 0.3f, 0.0f);

	/* create a  sphere   centered at the origin with radius 1*/
	renderable r_sphere = shape_maker::sphere(5);


	/* create 3 lines showing the reference frame*/
	renderable r_frame = shape_maker::frame(4.0);

	check_gl_errors(__LINE__, __FILE__);

	/* Transformation to setup the point of view on the scene */
	proj = glm::perspective(glm::radians(40.f), width / float(height), 1.5f, 20.f);
	view = glm::lookAt(glm::vec3(0, 6, 8.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	view_frame = glm::inverse(view);

	glEnable(GL_DEPTH_TEST);

	matrix_stack stack;

	/* define the viewport  */
	glViewport(0, 0, 1000, 800);

	glUseProgram(basic_shader.program);
	glUniformMatrix4fv(basic_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
	glUniformMatrix4fv(basic_shader["uView"], 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &glm::mat4(1.f)[0][0]);
	glUniform3f(basic_shader["uColor"], 1.0, 0.0, 0.0);

	glUseProgram(flat_shader.program);

	glUniformMatrix4fv(flat_shader["uView"], 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(flat_shader["uModel"], 1, GL_FALSE, &glm::mat4(1.f)[0][0]);
	glUniform4f(flat_shader["uColor"], 1.0, 1.0, 1.0, 1.0);

	glDepthFunc(GL_LESS);

	/* set the trackball position */
	tb[0].set_center_radius(glm::vec3(0, 0, 0), 2.f);
	tb[1].set_center_radius(glm::vec3(0, 0, 0), 2.f);
	curr_tb = 0;

/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glClearColor(0.f, 0.f, 0.f, 1.f);

		stack.push();
		
		stack.push();
		stack.mult(tb[curr_tb].matrix());

		float scale = 1.f / bbox.diagonal();
		//transate and scale so the the whole scene is included in the unit cube centered in 
		// the origin in workd space
		stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(scale, scale, scale)));
		stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(-bbox.center())));

		// render each renderable
		for (unsigned int i = 0; i < obj.size(); ++i) {
			obj[i].bind();
			stack.push();
			// each object had its own transformation that was read in the gltf file
			stack.mult(obj[i].transform);

			glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
			glUniform3f(basic_shader["uColor"], 1.0, 0.0, 0.0);
			glDrawElements(obj[i]().mode, obj[i]().count, obj[i]().itype, 0);
			stack.pop();
		}
		stack.pop();

		glUseProgram(basic_shader.program);
		glUniformMatrix4fv(basic_shader["uProj"], 1, GL_FALSE, &proj[0][0]);


		/* DRAW the trackball */
#ifdef TRACKBALL
		r_cube.bind();
		//// draw a frame (red = x, green = y, blue = z )
		stack.push();
		stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(0.05, 1.0, 0.05)));
		stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(0, 1.0, 0)));
		glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		glUniform3f(basic_shader["uColor"], 0.0, 1.0, 0.0);
		glDrawElements(r_cube().mode, r_cube().count, r_cube().itype, 0);
		stack.pop();

		stack.push();
		stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(1, 0.05, 0.05)));
		stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(1.0, 0.0, 0)));
		glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		glUniform3f(basic_shader["uColor"], 1.0, 0.0, 0.0);
		glDrawElements(r_cube().mode, r_cube().count, r_cube().itype, 0);
		stack.pop();

		stack.push();
		stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(0.05f, 0.05f, 1.f)));
		stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(0.0, 0.0, 1.f)));
		glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		glUniform3f(basic_shader["uColor"], 0.0, 0.0, 1.0);
		glDrawElements(r_cube().mode, r_cube().count, r_cube().itype, 0);
		stack.pop();

		// draw one sphere
		glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
		glUniform3f(basic_shader["uColor"], 0.5, 0.5, 0.5);
		r_sphere.bind();
		glDrawElements(r_sphere().mode, r_sphere().count, r_sphere().itype, 0);
#endif
		///* end draw trackball */


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