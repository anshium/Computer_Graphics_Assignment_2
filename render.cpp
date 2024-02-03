#include "render.h"
#include "shade.h"

Integrator::Integrator(Scene &scene)
{
    this->scene = scene;
    this->outputImage.allocate(TextureType::UNSIGNED_INTEGER_ALPHA, this->scene.imageResolution);
}

long long Integrator::render()
{
    Vector3f white_color = {1, 1, 1};
    auto startTime = std::chrono::high_resolution_clock::now();
    for (int x = 0; x < this->scene.imageResolution.x; x++) {
        for (int y = 0; y < this->scene.imageResolution.y; y++) {
            Vector3f color = {0, 0, 0};
            
            Ray cameraRay = this->scene.camera.generateRay(x, y);
            Interaction si = this->scene.rayIntersect(cameraRay);

            if(si.didIntersect){
                for(auto& light : this->scene.lights){
                    if(light.lightType == DIRECTIONAL_LIGHT){
                        // Now we will see if the ray intersected in the direction of the light from the point where it intersected with the scene from the viewport
                        Ray shadowRay = Ray(si.p + 0.001 * si.n, light.locationOrDirection);
                        Interaction shadowRayInteraction = this->scene.rayIntersect(shadowRay);

                        if(!shadowRayInteraction.didIntersect){
                            color += shade(light, white_color) * AbsDot(light.locationOrDirection, si.n);
                        }
                    }
                }
                this->outputImage.writePixelColor(color, x, y);
            }
            this->outputImage.writePixelColor(color, x, y);
        }
    }
    auto finishTime = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::microseconds>(finishTime - startTime).count();
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        std::cerr << "Usage: ./render <scene_config> <out_path>";
        return 1;
    }
    Scene scene(argv[1]);

    Integrator rayTracer(scene);
    auto renderTime = rayTracer.render();
    
    std::cout << "Render Time: " << std::to_string(renderTime / 1000.f) << " ms" << std::endl;
    rayTracer.outputImage.save(argv[2]);

    return 0;
}
