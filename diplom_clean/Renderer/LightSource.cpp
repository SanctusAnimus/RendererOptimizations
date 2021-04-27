#include "LightSource.h"


LightSource::LightSource(Rendering::LightData preset) {
	data = preset;
}


LightSource::LightSource(glm::vec3 position, glm::vec3 color) 
{
	data.Position = glm::vec4(position, 1.0);
	data.Color = glm::vec4(color, 1.0);

	data.Linear = 0.7f;
	data.Quadratic = 1.6f;

	const float maxBrightness = std::fmaxf(std::fmaxf(data.Color.r, data.Color.g), data.Color.b);
	float radius = (-data.Linear + (float)std::sqrt(data.Linear * data.Linear - 4.f * data.Quadratic * (1.0 - (256.0f / 5.0f) * maxBrightness))) / (2.0f * data.Quadratic);
	data.Radius = radius;
}


void LightSource::Render() {

}


bool LightSource::IsVisibleFrom(glm::mat4& mvp) {
	glm::vec4 worldspace = mvp * data.Position;
	return abs(worldspace.x) < worldspace.w + data.Radius &&
		abs(worldspace.y) < worldspace.w + data.Radius &&
		0 < worldspace.z &&
		abs(worldspace.z) < worldspace.w + data.Radius;
}