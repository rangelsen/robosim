#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "robot.hpp"

static const std::vector<glm::vec3> ROTATION_AXES_ {

	glm::vec3(0.0f, 1.0f, 0.0f),
	glm::vec3(0.0f, 0.0f, 1.0f),
	glm::vec3(0.0f, 1.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f)
};

////////////////////////////////////////////////////////////////////////////////
std::vector<glm::mat4> robot_get_kinematic_chain(float* q, int n_joints) {

	std::vector<glm::mat4> kin_chain;
	
	for (int i = 0; i < n_joints; i++) {


		glm::mat4 transform(1.0f);
		/*
		if (i == 0) {

			transform = glm::rotate(transform, q[i], ROTATION_AXES_[i]);
		}
		else {

			transform = kin_chain[i - 1];
			transform = glm::rotate(transform, q[i], ROTATION_AXES_[i]);
		}
		*/

		kin_chain.push_back(transform);
	}

	/*
	glm::mat4 transform(1.0f);
	kin_chain.push_back(glm::rotate(transform, q[0], glm::vec3(0.0f, 1.0f, 0.0f)));

	transform = glm::mat4(1.0f);
	kin_chain.push_back(glm::rotate(transform, q[1], );

	transform = glm::mat4(1.0f);
	kin_chain.push_back(transform);

	transform = glm::mat4(1.0f);
	kin_chain.push_back(transform);

	transform = glm::mat4(1.0f);
	kin_chain.push_back(transform);

	transform = glm::mat4(1.0f);
	kin_chain.push_back(transform);

	transform = glm::mat4(1.0f);
	kin_chain.push_back(transform);

	transform = glm::mat4(1.0f);
	kin_chain.push_back(transform);

	transform = glm::mat4(1.0f);
	kin_chain.push_back(transform);

	for (int i = 1; i < n_joints; i++) {

		kin_chain[i] = kin_chain[i - 1] * kin_chain[i];
	}
	*/

	return kin_chain;
}

/// @file

