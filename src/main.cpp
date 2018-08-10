#include <iostream>
#include <stdio.h>
#include <stddef.h>
#include <Eigen/Eigen>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <GL/glew.h>
#include <pthread.h>

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
#include <zmq.hpp>

////////////////////////////////////////////////////////////////////////////////
#define N_JOINTS 7

static float q_[N_JOINTS] = {0.0f, -1.0f, 0.0f, -2.6f, 0.0f, -1.57f, 0.0f};
static const float q_MIN_[N_JOINTS]   = {-2.8973, -1.7628, -3.0718, -2.8973, -0.0175, -2.8973};
static const float q_MAX_[N_JOINTS]   = {2.8973, 1.7628, 2.8973, -0.0698, 2.8973, 3.7525, 2.8973};
static bool is_running_ = true;

///////////////////////////////////////////////////////////////////////////////
void render_gui(SDL_Window* window, float* q, const float* q_min, const float* q_max, int n_joints) {

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
void* read_joints(void* arg) {

	zmq::context_t zmq_ctx(1);
	zmq::socket_t zmq_socket(zmq_ctx, ZMQ_SUB);
	zmq_socket.setsockopt(ZMQ_SUBSCRIBE, "joint", 5);
	// zmq_socket.connect("ipc:///tmp/socket");
	zmq_socket.connect("tcp://127.0.0.1:5555");

	printf("Communication set up\n");

	while (is_running_) {

		zmq::message_t msg;
		zmq_socket.recv(&msg);

		char msg_buffer[512];
		memset(msg_buffer, 0, 512);
		memcpy(msg_buffer, (char*) msg.data(), msg.size() + 1);
		msg_buffer[msg.size()] = '\0';
		printf("recv msg: %s\n", msg_buffer);

		if (strcmp(msg_buffer, "exit") == 0) {

			is_running_ = false;
			break;
		}

		char* str = strtok(msg_buffer, ",");

		for (int i = 0; i < N_JOINTS; i++) {

			printf("str: %s\n", str);
			str = strtok(NULL, ",");
		}

		float* q = (float*) arg;
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv) {

	pthread_t comm_thread;
	pthread_create(&comm_thread, NULL, read_joints, q_);

	Display* display = new Display();

	Camera* vcamera = new Camera(glm::vec3(0.0f), 0.0f, 0.0f);
	
	Model panda_model("res/Models/Panda/panda_linked.dae");

	Shader* panda_shader = new Shader("src/Shaders/panda",
		ShaderAttributes::MODEL_ATTRIBUTES);
	
	while (!display->IsClosed()) {

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		InputHandler::HandleInputs(display, vcamera);

		std::vector<glm::mat4> kinematic_chain =
			robot_get_kinematic_chain(q_, N_JOINTS);

		panda_model.Draw(panda_shader, vcamera, &kinematic_chain);

		render_gui(display->WindowHandle(), q_, q_MIN_, q_MAX_, N_JOINTS);

		display->Update();
	}

	delete panda_shader;
	delete vcamera;
	delete display;

    return 0;
}

/// @file

