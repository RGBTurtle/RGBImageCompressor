#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <stdint.h>
#include <array>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "include/stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "include/stb/stb_image_write.h"

const int width = 150;
const int height = 150;
const int greyChannels = 3;

unsigned char *grey_img;

size_t grey_img_size;


std::array<unsigned char, 3> color_by_distance(std::array<unsigned char, 3> color, float distance){
    std::array<unsigned char, 3> newColor;
    newColor[0] = (unsigned char)((int)color[0] * distance);
    newColor[1] = (unsigned char)((int)color[1] * distance);
    newColor[2] = (unsigned char)((int)color[2] * distance);
    return newColor;
}

float distance_to_center(unsigned char* pixel, unsigned char* start, std::array<int16_t, 2> scale){
    int16_t pixelpos = (pixel - grey_img) / greyChannels;
    int16_t startpos = (start - grey_img) / greyChannels;
    int16_t relapos = pixelpos - startpos;
    std::array<float, 2> vector = {(relapos) % scale[0], relapos / scale[1]};
    vector[0] /= scale[0];
    vector[1] /= scale[1];
    printf("|%f|", vector[0]);
    return float(vector[0] + vector[1]);
}


void set_pixel_color(unsigned char* pixel, std::array<unsigned char, 3> color){
    if (pixel > grey_img && pixel < grey_img + grey_img_size){
        if (color[0] + *pixel >= 256){*pixel = 255;} else {*pixel += color[0];}
        if (color[1] + *(pixel + 1) >= 256){*(pixel + 1) = 255;} else {*(pixel + 1) += color[1];}
        if (color[2] + *(pixel + 2) >= 256){*(pixel + 2) = 255;} else {*(pixel + 2) += color[2];}
        //TODO --- Fix this to not be additive!!!!!!!!
    }
}

struct splat {
    std::array<int16_t, 2> position ;

    std::array<int16_t, 2> scale ;

    std::array<unsigned char, 3> color ;

    splat(std::array<int16_t, 2> position, std::array<int16_t, 2> scale, std::array<unsigned char, 3> color){
        this->position = position;
        this->scale = scale;
        this->color = color;
    };

    void paint(){  //runs for every splat at runtime
        unsigned char *placement = grey_img;
        placement += position[0] * greyChannels;
        placement += position[1] * greyChannels * width;
        set_pixel_color(placement, color);

        for (int i = 0; i != scale[1] * width * greyChannels; i += width * greyChannels){
            for (int j = 0; j != scale[0] * greyChannels; j += greyChannels){
                set_pixel_color(placement + i + j, color_by_distance(color, distance_to_center(placement + i + j, placement, scale)));
            }
        }
    }

};

std::vector<splat> splats;




int main(){
    splats.push_back(splat( {5, 5}, {140, 140}, {255, 0, 255} ));

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

    //std::ofstream outFile("GreenDom.RGBIC", std::ios_base::trunc);


    stbi_write_jpg("output.jpg", width, height, greyChannels, grey_img, 100);
    return 1;
}