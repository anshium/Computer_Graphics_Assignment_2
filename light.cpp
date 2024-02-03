#include "light.h"

Light::Light(LightType lightType, Vector3f locationOrDirection, Vector3f radiance)
    : lightType(lightType),
    locationOrDirection(locationOrDirection),
    radiance(radiance){}

std::vector<Light> loadLights(nlohmann::json sceneConfig){
    std::vector<Light> lightVector;
    int light_index = 0;

    // std::cout << "Here " << __LINE__ << std::endl;

	// Directional Lights
    auto directionalLights = sceneConfig["directionalLights"];
    // std::cout << "Here " << __LINE__ << std::endl;

    for(auto& directionalLight : directionalLights){
    // std::cout << "Here " << __LINE__ << std::endl;
        auto& direction = directionalLight["direction"];
        auto& radiance = directionalLight["radiance"];

        Vector3f directionVector;
        directionVector[0] = direction[0];
        directionVector[1] = direction[1];
        directionVector[2] = direction[2];

        Vector3f radianceVector;
        radianceVector[0] = radiance[0];
        radianceVector[1] = radiance[1];
        radianceVector[2] = radiance[2];

    // std::cout << "Here " << __LINE__ << std::endl;
        Light light(
            DIRECTIONAL_LIGHT, 
            directionVector,
            radianceVector
        );
    // std::cout << "Here " << __LINE__ << std::endl;

        lightVector.push_back(light);

    }

    // std::cout << "Here " << __LINE__ << std::endl;
    // Point Lights
    auto pointLights = sceneConfig["pointLights"];

    for(auto& pointLight : pointLights){
        auto& location = pointLight["location"];
        auto& radiance = pointLight["radiance"];

        Vector3f locationVector;
        locationVector[0] = location[0];
        locationVector[1] = location[1];
        locationVector[2] = location[2];

    // std::cout << "Here " << __LINE__ << std::endl;
        Vector3f radianceVector;
        radianceVector[0] = radiance[0];
        radianceVector[1] = radiance[1];
        radianceVector[2] = radiance[2];

        Light light(
            POINT_LIGHT, 
            locationVector,
            radianceVector
        );
    // std::cout << "Here " << __LINE__ << std::endl;
        lightVector.push_back(light);
    // std::cout << "Here " << __LINE__ << std::endl;
    }

    // std::cout << "Here " << __LINE__ << std::endl;
    return lightVector;
}