#pragma once

#include "common.h"

enum LightType {
	POINT_LIGHT=0,
	DIRECTIONAL_LIGHT=1,
	NUM_LIGHT_TYPES
};

struct Light {
	LightType lightType;
	Vector3f locationOrDirection;
	Vector3f radiance;

	Light(LightType lightType, Vector3f locationOrDirection, Vector3f radiance);
};

// It's just numbers so I don't think it should be problem to just be able to load them. Yeah, I think so.
std::vector<Light> loadLights(nlohmann::json sceneConfig);