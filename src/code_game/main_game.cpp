#define NANOSVG_IMPLEMENTATION	// Expands implementation
#include "3dparty/nanosvg/src/nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "3dparty/nanosvg/src/nanosvgrast.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_write.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include "..\common\debugging.h"
#include "..\common\frame_buffer_object.h"
#include "..\common\renderable.h"
#include "..\common\shaders.h"
#include "..\common\simple_shapes.h"
#include "..\common\carousel\carousel.h"
#include "..\common\carousel\carousel_to_renderable.h"
#include "..\common\carousel\carousel_loader.h"

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

race r;
renderable r_track;
renderable r_terrain;
shader basic_shader;
frame_buffer_object fbo;
renderable r_cube;

//#define RECTIFY
void rectify_terrain() {
	glEnable(GL_DEPTH_TEST);
	const glm::vec4 & ts = r.ter().rect_xz;
	float r = 1.f / 256.f;
	glm::mat4 ortho = glm::ortho( ts[0], ts[2]-r,  ts[1], ts[3] - r,  0.f, 255.f/50.f);
	glm::mat4 view = glm::lookAt(glm::vec3(ts[0], 0.f, ts[1]), glm::vec3(ts[0],  100, ts[1]), glm::vec3(0, 0,  1));

	glViewport(0, 0, 256, 256);
	glDisable(GL_CULL_FACE);
	glUseProgram(basic_shader.program);
	fbo.create(256, 256); // TODO, get the actual size of the terrain in pixels
	glClearColor(0, 0.2, 0.8, 1);
	glBindFramebuffer(GL_FRAMEBUFFER,fbo.id_fbo);
	glClear(GL_COLOR_BUFFER_BIT  | GL_DEPTH_BUFFER_BIT);
	
	glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &glm::mat4(1.f)[0][0]);
	glUniformMatrix4fv(basic_shader["uView"] , 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(basic_shader["uProj"] , 1, GL_FALSE, &ortho[0][0]);

	glPointSize(1);
	glUniform3f(basic_shader["uColor"], 1, 0, 0.0);
 	r_terrain.bind();
 	glDrawArrays(GL_POINTS, 0,r_terrain.vn);

	glDepthFunc(GL_ALWAYS);
	glUniform3f(basic_shader["uColor"], 0, 1, 0.0);
	r_track.bind();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, r_track.vn);
	

	unsigned char * data = new unsigned char[256 * 256 ];
	 unsigned char * datac = new unsigned char[256 * 256 * 4];
	//glBindTexture(GL_TEXTURE_2D, fbo.id_depth);
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glReadPixels(0, 0, 256, 256, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, data);
	stbi_write_png("terrain_2048r.png", 256, 256, 1, data, 0);
	// glReadPixels(0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, datac);
	// stbi_write_png("terrain256x256_c.png", 256, 256, 4, datac, 0);
	delete [] data;
	// delete[] datac;

	glBindFramebuffer(GL_FRAMEBUFFER,0);
}

bool  pick(float xpos, float ypos, glm::vec3 & hit) {
	float depthvalue;
	glReadPixels((int)xpos, 800 - (int)ypos, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depthvalue);
	if (depthvalue == 1.f)
		return false;

	glm::vec4 ndc = glm::vec4(-1.f + xpos / 1000.f * 2, -1.f + (800 - ypos) / 800.f * 2.f, -1.f + depthvalue*2.f, 1.f);
	glm::vec4 hit1 = glm::inverse(proj*view)*ndc;
	hit1 /= hit1.w;

	// from viewport to world space with unProject
	hit =  glm::unProject(glm::vec3(xpos, 800 - ypos, depthvalue), view, proj, glm::vec4(0, 0, 1000, 800));
	return true;
}

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
		if (mods & GLFW_MOD_SHIFT == 1)
		{
			glm::vec3 c;
			if (!pick(xpos, ypos, c))
				return;
			tb[curr_tb].set_center_radius(c, 1.f);
			glm::mat4 view_inv = glm::inverse(view);
			glm::vec4 c_vs = view*glm::vec4(c.x, c.y, c.z, 1.0);
			c_vs.z = 0.f;
			glm::vec4 newpos = view_inv*c_vs;
			view_inv[3] = newpos;
			view = glm::inverse(view_inv);
		} 
		else
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

bool from_car = false;
void key_callback(GLFWwindow * window, int key, int scancode, int action, int mods) {
	/* every time any key is presse it switch from controlling trackball tb[0] to tb[1] and viceversa */
	//if (action == GLFW_PRESS)
	//	curr_tb = 1 - curr_tb;
	if (action == GLFW_PRESS)
	from_car = !from_car;


}



	int main(int argc, char** argv)
	{
#ifdef RECTIFY
		carousel_loader::load("small_test.svg", "terrain_2048.png",r);
#else
		carousel_loader::load("small_test.svg", "terrain_2048r.png", r);
#endif
		//add 10 cars
		for (int i = 0; i < 10; ++i)		
			r.add_car();

		GLFWwindow* window;

		/* Initialize the library */
		if (!glfwInit())
			return -1;

		/* Create a windowed mode window and its OpenGL context */
		glfwWindowHint(GLFW_DEPTH_BITS, 8);
		window = glfwCreateWindow(800, 800, "CarOusel", NULL, NULL);
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

		renderable fram = shape_maker::frame();

	    r_cube = shape_maker::cube();

		
		r_track.create();
		game_to_renderable::to_track(r, r_track);

		
		r_terrain.create();
		game_to_renderable::to_heightfield(r, r_terrain);
		
		renderable r_trees;
		r_trees.create();
		game_to_renderable::to_tree(r, r_trees);

		renderable r_lamps;
		r_lamps.create();
		game_to_renderable::to_lamps(r, r_lamps);

		 
		basic_shader.create_program("shaders/basic.vert", "shaders/basic.frag");

		/* use the program shader "program_shader" */
		glUseProgram(basic_shader.program);
		
		/* define the viewport  */
		glViewport(0, 0, 800, 800);

		tb[0].reset();
		tb[0].set_center_radius(glm::vec3(0, 0, 0), 0.5f);
		curr_tb = 0;

		proj = glm::perspective(glm::radians(45.f), 1.f, 0.1f, 10.f);
		view = glm::lookAt(glm::vec3(0, 1.f, 1.5), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.0, 1.f, 0.f));
		glUniformMatrix4fv(basic_shader["uProj"], 1, GL_FALSE, &proj[0][0]);
		glUniformMatrix4fv(basic_shader["uView"], 1, GL_FALSE, &view[0][0]);

		r.start(11,0,0,600);
		r.update();

		 matrix_stack stack;

		 glEnable(GL_DEPTH_TEST);

#ifdef RECTIFY
 rectify_terrain();
  exit(0);
#endif
		/* Loop until the user closes the window */
  while (!glfwWindowShouldClose(window))
  {
	  if (from_car) {
		  view = glm::inverse(glm::translate(r.cars()[0].frame, glm::vec3(0.f, 2.f, 0.f)));
		  proj = glm::perspective(glm::radians(45.f), 1.f, 0.1f, 100.f);
	  }
	  else {
		  proj = glm::perspective(glm::radians(45.f), 1.f, 0.1f, 10.f);
		  view = glm::lookAt(glm::vec3(0, 1.f, 1.5), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.0, 1.f, 0.f));

	  }

	  /* Render here */
	  glClearColor(0.3f, 0.3f, 0.3f, 1.f);
	  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	  check_gl_errors(__LINE__, __FILE__);

	  r.update();
	  stack.load_identity();
	  stack.push();
	  stack.mult(tb[0].matrix());
	  glUniformMatrix4fv(basic_shader["uView"], 1, GL_FALSE, &view[0][0]);
	  glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
	  glUniform3f(basic_shader["uColor"], -1.f, 0.6f, 0.f);
	  fram.bind();
	  glDrawArrays(GL_LINES, 0, 6);

	  glColor3f(0, 0, 1);
	  glBegin(GL_LINES);
	  glVertex3f(0, 0, 0);
	  glVertex3f(r.sunlight_direction().x, r.sunlight_direction().y, r.sunlight_direction().z);
	  glEnd();


	  float s = 1.f / r.bbox().diagonal();
	  glm::vec3 c = r.bbox().center();

	  if (!from_car){
		  stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(s)));
			stack.mult(glm::translate(glm::mat4(1.f), -c));
		}
			 
		  	glEnable(GL_POLYGON_OFFSET_FILL);
		   	glPolygonOffset(20.0, 10.0);
			glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
			glUniform3f(basic_shader["uColor"], -1, 1, 1.0);
			r_terrain.bind();
			glDrawElements(r_terrain().mode, r_terrain().count, r_terrain().itype,0);
			glDisable(GL_POLYGON_OFFSET_FILL);
			glDisable(GL_POLYGON_OFFSET_LINE);

			 r_track.bind();
			 glPointSize(1.0);
			 glUniform3f(basic_shader["uColor"], 0.2f, 0.3f, 0.2f);
//			 glDrawArrays(GL_POINTS, 0, r_track.vn);

			for (unsigned int ic = 0; ic < r.cars().size(); ++ic) {
				stack.push();
				stack.mult(r.cars()[ic].frame);
				stack.mult(glm::translate(glm::mat4(1.f), glm::vec3(0,0.1,0.0)));
				glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
				glUniform3f(basic_shader["uColor"], -1.f, 0.6f, 0.f);
				fram.bind();
				glDrawArrays(GL_LINES, 0, 6);
				stack.pop();
			}

			fram.bind();
			for (unsigned int ic = 0; ic < r.cameramen().size(); ++ic) {
				stack.push();
				stack.mult(r.cameramen()[ic].frame);
				stack.mult(glm::scale(glm::mat4(1.f), glm::vec3(4, 4,4)));
				glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
				glUniform3f(basic_shader["uColor"], -1.f, 0.6f, 0.f);
				glDrawArrays(GL_LINES, 0, 6);
				stack.pop();
			}
			glUniformMatrix4fv(basic_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);
	

			glPointSize(1.0);


			r_trees.bind();
			glUniform3f(basic_shader["uColor"], 0.f, 1.0f, 0.f);
			glDrawArrays(GL_LINES, 0, r_trees.vn);


			r_lamps.bind();
			glUniform3f(basic_shader["uColor"], 1.f, 1.0f, 0.f);
			glDrawArrays(GL_LINES, 0, r_lamps.vn);




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

 
