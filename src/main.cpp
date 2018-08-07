#include <iostream>
#include <stdio.h>
#include <stddef.h>
#include <Eigen/Eigen>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <GL/glew.h>

#include <Graphics/Display.hpp>
#include <Graphics/Camera.hpp>
#include <Graphics/Shader.hpp>
#include <Graphics/ShaderAttributes.hpp>
#include <Graphics/Cube.hpp>
#include <Input/InputHandler.hpp>
#include <model/model.h>
#include <robot/robot.hpp>

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {

	static const unsigned int N_JOINTS = 7;
	float q[N_JOINTS] = {1.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

	Display* display = new Display();

	Camera* vcamera = new Camera(glm::vec3(0.0f), 0.0f, 0.0f);
	
	Model panda_model("res/Models/Panda/panda_linked.dae");

	Shader* panda_shader = new Shader("src/Shaders/panda",
		ShaderAttributes::MODEL_ATTRIBUTES);
	
	while (!display->IsClosed()) {

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		InputHandler::HandleInputs(display, vcamera);

		std::vector<glm::mat4> kinematic_chain =
			robot_get_kinematic_chain(q, N_JOINTS);

		panda_model.Draw(panda_shader, vcamera, &kinematic_chain);

		display->Update();
	}

	delete panda_shader;
	delete vcamera;
	delete display;

    return 0;
}

/// @file

