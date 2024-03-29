#pragma once

#include "common.h"

enum TextureType {
    UNSIGNED_INTEGER_ALPHA = 0, // RGBA uint32
    FLOAT_ALPHA, // RGBA float
    NUM_TEXTURE_TYPES
};

struct Texture {
    unsigned long long data = 0;
    TextureType type;

    Vector2i resolution;

    Texture() {};
    Texture(std::string pathToImage);

    void allocate(TextureType type, Vector2i resolution);
    void writePixelColor(Vector3f color, int x, int y);
    Vector3f loadPixelColor(int x, int y);
    
    void loadJpg(std::string pathToJpg);
    void loadPng(std::string pathToPng);
    void loadExr(std::string pathToExr);
        
    void save(std::string path);
    void saveExr(std::string path);
    void savePng(std::string path);

    Vector3f nearestNeighbourFetch(float u, float v, int x, int y);     // x, y added for debugging
    Vector2f getUVCoordinates(Vector3f intersection_point, Vector3f v1, Vector3f v2, Vector3f v3, Vector2f u1, Vector2f u2, Vector2f u3);
    Vector3f bilinearFetch(float u, float v, int x, int y);             // x, y added for debugging
    // Vector3f getColor(int option);
};