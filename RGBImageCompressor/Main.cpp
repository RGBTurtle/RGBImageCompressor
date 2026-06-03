#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <stdint.h>
#include <array>
#include <vector>
#define _USE_MATH_DEFINES
#include <cmath>
#include <random>

#define STB_IMAGE_IMPLEMENTATION
#include "include/stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "include/stb/stb_image_write.h"

const int width = 500;
const int height = 500;
const int greyChannels = 3;

const double degToRad = M_PI / 180;

unsigned char *grey_img;
size_t grey_img_size;



unsigned char* rotate_pixel_by_degree(unsigned char* pixel, unsigned char* center, int rotation){ // returns pixel coord rotated by an int 0 - 180
    std::array<int,2> position  = {((pixel - grey_img) / greyChannels) % width, ((pixel - grey_img) / greyChannels) / width};
    std::array<int,2> centerVec = {((center - grey_img) / greyChannels) % width, ((center - grey_img) / greyChannels) / width};
    position[0] -= centerVec[0];
    position[1] -= centerVec[1];

    double cosine = cos(rotation * degToRad);
    double sine = sin(rotation * degToRad);

    std::array<double,2> newVec = {(cosine * position[0]) + (-sine * position[1]), (sine * position[0]) + (cosine * position[1])};
    std::array<int,2> brandNewVec = {(int)newVec[0], (int)newVec[1]};

    brandNewVec[0] += centerVec[0];
    brandNewVec[1] += centerVec[1];

    unsigned char* newPixel = grey_img;
    newPixel += (brandNewVec[0] * greyChannels) + (brandNewVec[1] * width * greyChannels);

    return(newPixel);
}



std::array<unsigned char, 3> color_by_distance(std::array<unsigned char, 3> color, float distance){ //returns color but mulitplied by distance
    if (distance > 1){
        distance = 1;
    }
    distance = 1 - distance;
    std::array<unsigned char, 3> newColor;
    newColor[0] = (unsigned char)((int)color[0] * distance);
    newColor[1] = (unsigned char)((int)color[1] * distance);
    newColor[2] = (unsigned char)((int)color[2] * distance);
    return newColor;
}

float distance_to_center(unsigned char* pixel, unsigned char* start, std::array<int16_t, 2> scale){ // returns the distance to the center, where 1unit is the distance from the center to the edge
    int32_t pixelpos = (pixel - grey_img) / greyChannels;
    int32_t startpos = (start - grey_img) / greyChannels;
    std::array<float, 2> vector = {pixelpos % width, pixelpos / width};
    vector[0] -= startpos % width;
    vector[1] -= startpos / width;
    vector[0] -= scale[0] / 2;
    vector[1] -= scale[1] / 2;

    vector[0] /= scale[0] / 2;
    vector[1] /= scale[1] / 2;
    return float(sqrt((vector[0] * vector[0]) + (vector[1] * vector[1])));
}


void set_pixel_color(unsigned char* pixel, std::array<unsigned char, 3> color, float distance){ // set the pixel to the color, where distance is how much the original color is included
    if (pixel > grey_img && pixel < grey_img + grey_img_size){
        if (distance > 1){
        distance = 1;
        }
        float COEF = (*(pixel + 3)) / 255.0f;
        *pixel = (*pixel * distance) + color[0];
        *(pixel + 1) = (*(pixel + 1) * distance) + color[1];
        *(pixel + 2) = (*(pixel + 2) * distance) + color[2];
    }
}

struct splat {
    //TODO --- ADD ROTATION!!!! --- ALMOST DONE :)
    std::array<int16_t, 2> position ;

    std::array<int16_t, 2> scale ;

    unsigned char rotation;

    std::array<unsigned char, 3> color ;

    splat(std::array<int16_t, 2> position, std::array<int16_t, 2> scale, unsigned char rotation, std::array<unsigned char, 3> color){
        this->position = position;
        this->scale = scale;
        this->rotation = rotation;
        this->color = color;
    };

    void paint(){  //runs for every splat at runtime
        unsigned char *placement = grey_img;
        placement += position[0] * greyChannels;
        placement += position[1] * greyChannels * width;

        for (int i = 0; i != scale[1] * width * greyChannels; i += width * greyChannels){
            for (int j = 0; j != scale[0] * greyChannels; j += greyChannels){

                float distance = distance_to_center(placement + i + j, placement, scale);

                set_pixel_color(rotate_pixel_by_degree(placement + i + j, placement + ((scale[0] / 2) * greyChannels) + ((scale[1] / 2) * width * greyChannels), rotation), color_by_distance(color, distance), distance);

            }
        }
    }

};

std::vector<splat> splats;

int main(){

    //std::ifstream inFile("Test.RGBG", std::ios_base::binary);
    //inFile.read(reinterpret_cast<char*>(splats.data()), sizeof(splats));


    //splats.push_back(splat( {0, 40}, {380, 40}, {255, 0, 0} ));
    splats.push_back(splat( {150, 250}, {300, 40}, 45, {0, 0, 255} ));
    splats.push_back(splat( {150, 250}, {380, 40}, 80, {0, 255, 0} ));

    
    grey_img_size = width * height * greyChannels;

    grey_img = (unsigned char*)malloc(grey_img_size);
    if (grey_img == NULL){
        printf("ERROR failed to allocate memory");
        return 0;
    }

    for(unsigned char *i = grey_img; i != grey_img + grey_img_size; i += greyChannels){
        *i = 0;
        *(i+1) = 0;
        *(i+2) = 0;
    }

    for(int i = 0 ;i != splats.size(); i++){
        splats[i].paint();
    }

    // std::ofstream outFile("test.RGBG", std::ios_base::binary);
    // if (!outFile) {
    //     printf("ERROR file not found");
    //     return 1;
    // }
    // outFile.write(reinterpret_cast<char*>(splats.data()), splats.size() * sizeof(splat));
    // outFile.close();

    stbi_write_jpg("output.jpg", width, height, greyChannels, grey_img, 100);
    return 1;
}