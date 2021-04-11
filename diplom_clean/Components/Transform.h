#pragma once
#ifndef TRANSFORM_COMPONENT_DECL
#define TRANSFORM_COMPONENT_DECL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>


struct TransformComponent {
	glm::vec3 translation;
	glm::vec3 rotation_axis;
	glm::vec3 scale;
	float rotation_radians = 0.f;

	TransformComponent() = default;
	TransformComponent(const TransformComponent&) = default;

	TransformComponent(const glm::vec3& translation)
		: translation(translation) 
	{
		scale = glm::vec3(1.f);
		rotation_axis = glm::vec3(1.f);
	}

	TransformComponent(const glm::vec3& translation, const glm::vec3& scale)
		: translation(translation), scale(scale) 
	{
		rotation_axis = glm::vec3(1.f);
	}

	TransformComponent(const glm::vec3& translation, float rotation, glm::vec3 axis)
		: translation(translation), rotation_radians(glm::radians(rotation)), rotation_axis(axis) 
	{
		scale = glm::vec3(1.f);
	}

	TransformComponent(const glm::vec3& translation, const glm::vec3 scale, float rotation, glm::vec3 axis)
		: translation(translation), scale(scale), rotation_radians(glm::radians(rotation)), rotation_axis(axis) {}

	void SetRotation(float degrees, glm::vec3 rotation_axis) {
		rotation_radians = glm::radians(degrees);
		rotation_axis = rotation_axis;
	}

	glm::mat4 GetModelMatrix() {
		glm::mat4 model(1.f);

		model = glm::translate(model, translation);
		model = glm::rotate(model, rotation_radians, rotation_axis);
		model = glm::scale(model, scale);

		return model;
	}
};

#endif // !TRANSFORM_COMPONENT_DECL
