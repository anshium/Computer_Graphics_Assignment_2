#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

#define TINYEXR_IMPLEMENTATION
#include "tinyexr/tinyexr.h"

#define EPSILON 0.001

Texture::Texture(std::string pathToImage)
{
    size_t pos = pathToImage.find(".exr");

    if (pos > pathToImage.length()) {
        this->type = TextureType::UNSIGNED_INTEGER_ALPHA;
        pos = pathToImage.find(".png");

        if (pos > pathToImage.length()) 
            this->loadJpg(pathToImage);
        else
            this->loadPng(pathToImage);
    }
    else {
        this->type = TextureType::FLOAT_ALPHA;
        this->loadExr(pathToImage);
    }
}

void Texture::allocate(TextureType type, Vector2i resolution)
{
    this->resolution = resolution;
    this->type = type;

    if (this->type == TextureType::UNSIGNED_INTEGER_ALPHA) {
        uint32_t* dpointer = (uint32_t*) malloc(this->resolution.x * this->resolution.y * sizeof(uint32_t));
        this->data = (uint64_t)dpointer;
    }
    else if (this->type == TextureType::FLOAT_ALPHA) {
        float* dpointer = (float*)malloc(this->resolution.x * this->resolution.y * 4 * sizeof(float));
        this->data = (uint64_t)dpointer;
    }
}

void Texture::writePixelColor(Vector3f color, int x, int y)
{
    if (this->type == TextureType::UNSIGNED_INTEGER_ALPHA) {
        uint32_t* dpointer = (uint32_t*)this->data;

        uint32_t r = static_cast<uint32_t>(std::min(color.x * 255.0f, 255.f));
        uint32_t g = static_cast<uint32_t>(std::min(color.y * 255.0f, 255.f)) << 8;
        uint32_t b = static_cast<uint32_t>(std::min(color.z * 255.0f, 255.f)) << 16;
        uint32_t a = 255 << 24;

        uint32_t final = r | g | b | a;

        dpointer[y * this->resolution.x + x] = final;
    }
}

/*
Reads the color defined at integer coordinates 'x,y'.
The top left corner of the texture is mapped to '0,0'.
*/
Vector3f Texture::loadPixelColor(int x, int y) {
    Vector3f rval(0.f, 0.f, 0.f);
    if (this->type == TextureType::UNSIGNED_INTEGER_ALPHA) {
        uint32_t* dpointer = (uint32_t*)this->data;

        uint32_t val = dpointer[y * this->resolution.x + x];
        uint32_t r = (val >> 0) & 255u;
        uint32_t g = (val >> 8) & 255u;
        uint32_t b = (val >> 16) & 255u;

        rval.x = r / 255.f;
        rval.y = g / 255.f;
        rval.z = b / 255.f;
    }

    return rval;
}

void Texture::loadJpg(std::string pathToJpg)
{
    Vector2i res;
    int comp;
    unsigned char* image = stbi_load(pathToJpg.c_str(), &res.x, &res.y, &comp, STBI_rgb_alpha);
    int textureID = -1;
    if (image) {
        this->resolution = res;
        this->data = (uint64_t)image;

        /* iw - actually, it seems that stbi loads the pictures
            mirrored along the y axis - mirror them here */
        for (int y = 0; y < res.y / 2; y++) {
            uint32_t* line_y = (uint32_t*)this->data + y * res.x;
            uint32_t* mirrored_y = (uint32_t*)this->data + (res.y - 1 - y) * res.x;
            int mirror_y = res.y - 1 - y;
            for (int x = 0; x < res.x; x++) {
                std::swap(line_y[x], mirrored_y[x]);
            }
        }
    }
    else {
        std::cerr << "Could not load .jpg texture from " << pathToJpg << std::endl;
        std::cerr << stbi_failure_reason() << std::endl;
        exit(1);
    }
}

void Texture::loadPng(std::string pathToPng)
{
    Vector2i res;
    int comp;
    unsigned char* image = stbi_load(pathToPng.c_str(), &res.x, &res.y, &comp, STBI_rgb_alpha);
    int textureID = -1;
    if (image) {
        this->resolution = res;
        this->data = (uint64_t)image;

        /* iw - actually, it seems that stbi loads the pictures
            mirrored along the y axis - mirror them here */
        for (int y = 0; y < res.y / 2; y++) {
            uint32_t* line_y = (uint32_t*)this->data + y * res.x;
            uint32_t* mirrored_y = (uint32_t*)this->data + (res.y - 1 - y) * res.x;
            int mirror_y = res.y - 1 - y;
            for (int x = 0; x < res.x; x++) {
                std::swap(line_y[x], mirrored_y[x]);
            }
        }
    }
    else {
        std::cerr << "Could not load .png texture from " << pathToPng << std::endl;
        std::cerr << stbi_failure_reason() << std::endl;
        exit(1);
    }
}

void Texture::loadExr(std::string pathToExr)
{
    int width;
    int height;
    const char* err = nullptr; // or nullptr in C++11
    
    float* data;
    int ret = LoadEXR(&data, &width, &height, pathToExr.c_str(), &err);
    this->data = (uint64_t)data;

    if (ret != TINYEXR_SUCCESS) {
        std::cerr << "Could not load .exr texture map from " << pathToExr << std::endl;
        exit(1);
    }
    else {
        this->resolution = Vector2i(width, height);
    }
}

void Texture::save(std::string path)
{
    size_t pos = path.find(".png");

    if (pos > path.length()) {
        this->saveExr(path);
    }
    else {
        this->savePng(path);
    }
}

void Texture::saveExr(std::string path)
{
    if (this->type == TextureType::FLOAT_ALPHA) {
        uint64_t hostData = this->data;

        const char* err = nullptr;
        SaveEXR((float*)hostData, this->resolution.x, this->resolution.y, 4, 0, path.c_str(), &err);
        
        if (err == nullptr)
            std::cout << "Saved EXR: " << path << std::endl;
        else
            std::cerr << "Could not save EXR: " << err << std::endl;
    }
    else {
        std::cerr << "Cannot save to EXR: texture is not of type float." << std::endl;
    }
}

void Texture::savePng(std::string path) 
{
    if (this->type == TextureType::UNSIGNED_INTEGER_ALPHA) {
        uint64_t hostData = this->data;
        const uint32_t* data = (const uint32_t*)hostData;

        std::vector<uint32_t> pixels;
        for (int y = 0; y < this->resolution.y; y++) {
            const uint32_t* line = data + (this->resolution.y - 1 - y) * this->resolution.x;
            for (int x = 0; x < this->resolution.x; x++) {
                pixels.push_back(line[x] | (0xff << 24));
            }
        }

        stbi_write_png(path.c_str(), this->resolution.x, this->resolution.y, 4, data, this->resolution.x * sizeof(uint32_t));

        std::cout << "Saved PNG: " << path << std::endl;
    }
    else {
        std::cerr << "Cannot save to PNG: texture is not of type uint32." << std::endl;
    }
}

double computeArea(Vector3f v1, Vector3f v2, Vector3f v3){
    Vector3f side1 = (v2 - v3);
    Vector3f side2 = (v3 - v1);
    Vector3f side3 = (v1 - v2);
    
    // double s = (side1.Length() + side2.Length() + side3.Length()) / 2;



    // double area = std::sqrt(std::abs(s * (s - side1.Length()) * (s - side2.Length()) * (s - side3.Length())));

    float area = Cross(side1, side2).Length() / 2;


    return area;
}

// Get UV Coordinates at intersection point using barycentric coordinates
Vector2f Texture::getUVCoordinates(Vector3f intersection_point, Vector3f v1, Vector3f v2, Vector3f v3, Vector2f u1, Vector2f u2, Vector2f u3){
    float main_triangle_area = computeArea(v1, v2, v3);
    if(main_triangle_area <= 0){
        std::cout << "SQRT of Negative quantity" << std::endl;
    }
    float alpha    = computeArea(intersection_point, v2, v3) / main_triangle_area;
    float beta     = computeArea(v1, intersection_point, v3) / main_triangle_area;
    float gamma    = computeArea(v1, v2, intersection_point) / main_triangle_area;

    // if(std::isnan(alpha)){
    //     alpha = 0;
    // }
    // if(std::isnan(beta)){
    //     beta = 0;
    // }
    // if(std::isnan(gamma)){
    //     gamma = 0;
    // }

    // Assuming u1, u2, u3 are defined and you want to print their values as well
    #ifdef DEBUG 
    std::cout << "IP: " << "x: " << intersection_point.x << ", y: " << intersection_point.y << ", z: " << intersection_point.z << std::endl; 


    std::cout << "v1: " << "x: " << v1.x << ", y: " << v1.y << ", z: " << v1.z << std::endl; 
    std::cout << "v2: " << "x: " << v2.x << ", y: " << v2.y << ", z: " << v2.z << std::endl;
    std::cout << "v3: " << "x: " << v3.x << ", y: " << v3.y << ", z: " << v3.z << std::endl;
    // Assuming u1, u2, u3 are defined and you want to print their values as well
    std::cout << "v1: " << "Length: " << (v1 - v2).Length() << std::endl;
    std::cout << "v2: " << "Length: " << (v2 - v3).Length() << std::endl;
    std::cout << "v3: " << "Length: " << (v3 - v1).Length() << std::endl;
    // Printing the values
    std::cout << "Main Triangle Area: " << main_triangle_area << std::endl;
    std::cout << "Alpha: " << alpha << std::endl;
    std::cout << "Beta: " << beta << std::endl;
    std::cout << "Gamma: " << gamma << std::endl;

    // Assuming u1, u2, u3 are defined and you want to print their values as well
    std::cout << "u1: " << "x: " << u1.x << ", y: " << u1.y << std::endl;
    std::cout << "u2: " << "x: " << u2.x << ", y: " << u2.y << std::endl;
    std::cout << "u3: " << "x: " << u3.x << ", y: " << u3.y << std::endl;
    #endif
    Vector2f uv = alpha * u1 + beta * u2 + gamma * u3;

    // uv.x = clamp(uv.x, 0.0f, 1.0f);
    // uv.y = clamp(uv.y, 0.0f, 1.0f);

    return uv;
}

// I am guessing the 
// Fetches the color of the पास का पडोसी |
Vector3f Texture::nearestNeighbourFetch(float u, float v, int x, int y){
    Vector3f color = {1, 1, 1};


    // Assuming that u and v both lie from 0 to 1
    // Let's check that:

    // if(!(u >= 0 && u <= 1 && v >=0 && v <= 1)){
    //     std::cout << "Error in UV Bounds" << std::endl;
    //     exit(1);
    // }

    // convert the u, v coordinates to texel coordinates
    float tx = u;
    float ty = v;

    // find corners
    Vector2f topCornerLeft;
    // Made it -2 to get image similar to gt, had -1 before. IDK why this works.
    topCornerLeft.x = clamp(floor(tx * (this->resolution.x - 1)), 0.0f, (float)(this->resolution.x - 2));
    topCornerLeft.y = clamp(floor(ty * (this->resolution.y - 1)), 0.0f, (float)(this->resolution.y - 2));

    // if(x == 900 && y == 700){
        // std::cout << "This is here" << std::endl;
        // // std::cout << u << ", ";
        // // std::cout << v << ", ";
        // std::cout << topCornerLeft.x << ", " << topCornerLeft.y;
        // std::cout << tx * this->resolution.x << ", ";
        // std::cout << topCornerLeft.y << std::endl;
    // }

    Vector2f topCornerRight;
    topCornerRight = {topCornerLeft.x + 1, topCornerLeft.y};

    Vector2f bottomCornerLeft = {topCornerLeft.x, topCornerLeft.y + 1};
    Vector2f bottomCornerRight = {topCornerLeft.x + 1, topCornerLeft.y + 1};

    Vector2f middle_vector = {(tx * (this->resolution.x - 2)), (ty * (this->resolution.y - 2))};

    Vector2f pass_wala_padosi;

    float min_distance = 1e30;

    if((middle_vector - topCornerLeft).Length() < min_distance){
        pass_wala_padosi = topCornerLeft;
        min_distance = (middle_vector - topCornerLeft).Length();
    }
    if((middle_vector - topCornerRight).Length() < min_distance){
        pass_wala_padosi = topCornerRight;
        min_distance = (middle_vector - topCornerRight).Length();
    }
    if((middle_vector - bottomCornerLeft).Length() < min_distance){
        pass_wala_padosi = bottomCornerLeft;
        min_distance = (middle_vector - bottomCornerLeft).Length();
    }
    if((middle_vector - bottomCornerRight).Length() < min_distance){
        pass_wala_padosi = bottomCornerRight;
        min_distance = (middle_vector - bottomCornerRight).Length();
    }
    
    std::vector<Vector2f> corners;
    corners.push_back(topCornerLeft);
    corners.push_back(topCornerRight);
    corners.push_back(bottomCornerLeft);
    corners.push_back(bottomCornerRight);

    // if(x == 1000 && y == 1000){
    //     for(int i = 0; i < corners.size(); i++){
    //         std::cout << corners[i].x << ", " << corners[i].y << std::endl;
    //     }
    // }    

    std::vector<Vector3f> colors;
    for(int i = 0; i < corners.size(); i++)
    colors.push_back(this->loadPixelColor(corners[i].x, corners[i].y));

    color = this->loadPixelColor((int)pass_wala_padosi.x, (int)pass_wala_padosi.y);
    // if(x == 700 && y == 700){
    //     u_int32_t* data = (uint32_t*)this->data;
    //     for(int i = 0; i < this->resolution.x; i++){
    //         for(int j = 0; j < this->resolution.y; j++){
    //             std::cout << data[j * this->resolution.x + x] << " ";
    //         }
    //     }
    //     for(int i = 0; i < this->resolution.x; i++){
    //         for(int j = 0; j < this->resolution.y; j++){
    //             color = this->loadPixelColor(i, j);
    //             if(color.x != 0 || color.y != 0 || color.z != 0){
    //                 std::cout << "Color: " << color.x << ", " << color.y << ", " << color.z << std::endl;
    //             }
    //         }
    //     }
    //     std::cout << "PWP: " << pass_wala_padosi.x << ", " << pass_wala_padosi.y << std::endl;
    //     std::cout << "Resolution: " << this->resolution.x << ", " << this->resolution.y << std::endl;
    //     std::cout << "Color: " << color.x << ", " << color.y << ", " << color.z << std::endl;
    // }

    if(x == 900 && y == 750){
        std::cout << "Printing this information" << std::endl;
        for(int i = 0; i < colors.size(); i++)
        std::cout << colors[i].x << ", " << colors[i].y << ", " << colors[i].z << std::endl;

        std::cout << color.x << ", " << color.y << ", " << color.z << std::endl;
    }

    return color;
}

Vector3f Texture::bilinearFetch(float u, float v, int x, int y){
    Vector3f color = {1, 1, 1};

    // convert the u, v coordinates to texel coordinates
    float tx = u;
    float ty = v;

    // find corners
    Vector2f topCornerLeft;
    topCornerLeft.x = clamp(floor(tx * (this->resolution.x - 1)), 0.0f, (float)this->resolution.x - 1);
    topCornerLeft.y = clamp(floor(ty * (this->resolution.y - 1)), 0.0f, (float)this->resolution.y - 1);

    Vector2f topCornerRight;
    topCornerRight = {topCornerLeft.x + 1, topCornerLeft.y};

    Vector2f bottomCornerLeft = {topCornerLeft.x, topCornerLeft.y + 1};
    Vector2f bottomCornerRight = {topCornerLeft.x + 1, topCornerLeft.y + 1};

    Vector2f middle_vector = Vector2f((tx * (this->resolution.x - 1)), (ty * (this->resolution.y - 1)));


    float min_distance = 1e30;

    std::vector<Vector2f> corners;
    corners.push_back(topCornerLeft);
    corners.push_back(topCornerRight);
    corners.push_back(bottomCornerLeft);
    corners.push_back(bottomCornerRight);

    Vector3f cu, cl;

    std::vector<Vector3f> colors;
    for(int i = 0; i < corners.size(); i++)
    colors.push_back(this->loadPixelColor(corners[i].x, corners[i].y));

    cu = (topCornerRight.x - middle_vector.x) * this->loadPixelColor(topCornerLeft.x, topCornerLeft.y)
        +
         (middle_vector.x - topCornerLeft.x) * this->loadPixelColor(topCornerRight.x, topCornerRight.y);

    cl = (bottomCornerRight.x - middle_vector.x) * this->loadPixelColor(bottomCornerLeft.x, bottomCornerLeft.y)
        +
         (middle_vector.x - bottomCornerLeft.x) * this->loadPixelColor(bottomCornerRight.x, bottomCornerRight.y);

    color = (bottomCornerLeft.y - middle_vector.y) * cu + (middle_vector.y - topCornerLeft.y) * cl;
    if(x == 900 && y == 750){
        std::cout << "cu, cl" << std::endl;
        std::cout << cu.x << ", " << cu.y << ", " << cu.z << std::endl;
        std::cout << cl.x << ", " << cl.y << ", " << cl.z << std::endl;

        std::cout << "Printing this information" << std::endl;
        for(int i = 0; i < colors.size(); i++)
        std::cout << colors[i].x << ", " << colors[i].y << ", " << colors[i].z << std::endl;

        std::cout << color.x << ", " << color.y << ", " << color.z << std::endl;
    }

    return color;
}