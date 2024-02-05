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
    int printed = 0;
    for (int x = 0; x < this->scene.imageResolution.x; x++) {
        for (int y = 0; y < this->scene.imageResolution.y; y++) {
            Vector3f color = {0, 0, 0};
            
            Ray cameraRay = this->scene.camera.generateRay(x, y);
            Interaction si = this->scene.rayIntersect(cameraRay);

            // Not doing this:    // Might be too dumb to do and even this might not work with some fairly complex scenes
            // Not doing this:    // Iterate through all the triangles and see which triangle has its vertices closest to to the intersection point and on the plane and the normal = sum of normals of the vertices / 3 normalised


            if(si.didIntersect){

                Vector2f uv = this->outputImage.getUVCoordinates(
                    si.p, 
                    si.triangleIntersected.v1, si.triangleIntersected.v2, si.triangleIntersected.v3, 
                    si.triangleIntersected.uv1, si.triangleIntersected.uv2, si.triangleIntersected.uv3
                );
                if(si.intersected_on_surface->hasDiffuseTexture()){
                    if(option == 0){
                        if(!(printed++))
                        std::cout << "Doing Nearest Neighbor Fetch" << std::endl;
                        white_color = si.intersected_on_surface->diffuseTexture.nearestNeighbourFetch(uv.x, uv.y, x, y);
                    }
                    else if(option == 1){
                        if(!(printed++))
                        std::cout << "Doing Bilinear Interpolation" << std::endl;
                        white_color = si.intersected_on_surface->diffuseTexture.bilinearFetch(uv.x, uv.y, x, y);
                        if(x == 900 && y == 750){
                            std::cout << "White color" << std::endl;

                            std::cout << white_color.x << ", " << white_color.y << ", " << white_color.z << std::endl;
                        }
                    }
                }
                else{
                    white_color = si.intersected_on_surface->diffuse;
                }
                // if(uv.x != 0 || uv.y != 0){

                //     std::cout << uv.x << ", " << uv.y << std::endl;
                //     std::cout << "x, y: " << x << ", " << y << std::endl;
                // }
                if(x == 900 && y == 750){
                    std::cout << "Has diffuse structure: " << si.intersected_on_surface->hasDiffuseTexture() << std::endl;
                    std::cout << uv.x << ", " << uv.y << std::endl;
                    std::cout << white_color.x << ", " << white_color.y << ", " << white_color.z << std::endl;
                }

                for(auto& light : this->scene.lights){
                    if(light.lightType == DIRECTIONAL_LIGHT){
                        // Now we will see if the ray intersected in the direction of the light from the point where it intersected with the scene from the viewport
                        Ray shadowRay = Ray(si.p + 0.001 * si.n, light.locationOrDirection);
                        Interaction shadowRayInteraction = this->scene.rayIntersect(shadowRay);

                        if(!shadowRayInteraction.didIntersect){
                            color += shade(light, white_color) * AbsDot(light.locationOrDirection, si.n);
                        }
                    }
                    else if(light.lightType == POINT_LIGHT){
                        Vector3f displacementVector = light.locationOrDirection - si.p;
                        Vector3f direction = Normalize(displacementVector);

                        Ray shadowRay = Ray(si.p + 0.001 * si.n, direction);
                        Interaction shadowRayInteraction = this->scene.rayIntersect(shadowRay);
                        
                        if(!shadowRayInteraction.didIntersect){
                            color += shade(light, white_color) * AbsDot(direction, si.n) / Dot(displacementVector, displacementVector);
                        }
                        if(shadowRayInteraction.didIntersect && Dot(shadowRayInteraction.p - si.p, shadowRayInteraction.p - si.p) > Dot(displacementVector, displacementVector)){
                            color += shade(light, white_color) * AbsDot(direction, si.n) / Dot(displacementVector, displacementVector);
                        }
                    }
                }
                // this->outputImage.writePixelColor(color, x, y);
            }
            this->outputImage.writePixelColor(color, x, y);
        }
    }
    auto finishTime = std::chrono::high_resolution_clock::now();

    return std::chrono::duration_cast<std::chrono::microseconds>(finishTime - startTime).count();
}

int option = 0;

int main(int argc, char **argv)
{
    if (argc != 4) {
        std::cerr << "Usage: ./render <scene_config> <out_path> <interpolation_variant>";
        return 1;
    }
    if(std::stoi(argv[3]) == 0){
        option = 0;
    }
    else if(std::stoi(argv[3]) == 1){
        option = 1;
    }
    else{
        std::cerr << "No such option exists" << std::endl;
        return 1;
    }

    Scene scene(argv[1]);

    Integrator rayTracer(scene);
    auto renderTime = rayTracer.render();
    
    std::cout << "Render Time: " << std::to_string(renderTime / 1000.f) << " ms" << std::endl;
    rayTracer.outputImage.save(argv[2]);

    return 0;
}
