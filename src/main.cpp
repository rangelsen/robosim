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
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl.h>
#include <imgui/imgui_impl_opengl3.h>

///////////////////////////////////////////////////////////////////////////////
void render_gui(SDL_Window* window, float* q, float* q_min, float* q_max, int n_joints) {

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);
	ImGui::NewFrame();

	ImGui::Begin("Joint control");

	for (int i = 0; i < n_joints; i++) {

		ImGui::Text("Joint %d: %f", i, q[i]);

		char slider_label[64];
		sprintf(slider_label, "joint %d", i + 1);
		ImGui::SliderFloat(slider_label, &q[i], q_min[i], q_max[i]);
	}

	ImGui::End();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {

	static const unsigned int N_JOINTS = 7;
	float q[N_JOINTS] = {0.0f, -1.0f, 0.0f, -2.6f, 0.0f, -1.57f, 0.0f};
	float q_MIN[N_JOINTS] = {-2.8973, -1.7628, -3.0718, -2.8973, -0.0175, -2.8973};
	float q_MAX[N_JOINTS] = {2.8973, 1.7628, 2.8973, -0.0698, 2.8973, 3.7525, 2.8973};

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

		render_gui(display->WindowHandle(), q, q_MIN, q_MAX, N_JOINTS);

		display->Update();
	}

	delete panda_shader;
	delete vcamera;
	delete display;

    return 0;
}

/// @file

