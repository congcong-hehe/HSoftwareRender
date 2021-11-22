// make by hyq
// 2021/11/16

#include "image.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../../third_party/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../third_party/stb_image_write.h"
#include <iostream>
#include <string>
using namespace std;
using namespace Math;

Image::Image() {
    
}

Image::~Image() {
    stbi_image_free(data_);
}

bool Image::read(const string &file_path) {
    data_ = stbi_load(file_path.c_str(), &width_, &height_, &channels_, channels_);
    if(!data_) {
        printf("ERROR: Could not read image file %s\n", file_path.c_str());
        return false;
    } else {
        printf("LOG: Load image file %s\n", file_path.c_str());
        return true;
    }
}

// 暂时全部保存为png形式
bool Image::write(const string &file_path) {
    int tag = stbi_write_png(file_path.c_str(), width_, height_, channels_, data_, width_ * channels_);
    if(0 == tag) {
        printf("ERROR: Could not write image file %s\n", file_path.c_str());
        return false;
    } else {
        printf("LOG: Write image file %s\n", file_path.c_str());
        return true;
    }
}

Vec3f Image::getColor(const int u, const int v) const {
    assert(u < height_ && u >= 0);
    assert(v < width_ && v >= 0);

    float scale = 1/255.0f;
    int index = u * channels_ * width_ + v * channels_;
    return Vec3f(scale * data_[index], scale * data_[index + 1], scale * data_[index + 2]);
}

Vec3f Image::getColor(const float u, const float v) const {
    return getColor(static_cast<int>(u * height_), static_cast<int>(v * width_));
}

void Image::setColor(const int u, const int v, const Vec3f &color) const {
    assert(u < height_ && u >= 0);
    assert(v < width_ && v >= 0);

    float scale = 255.0f;
    int index = u * channels_ * width_ + v * channels_;
    data_[index] = static_cast<int>(color.r * scale);
    data_[index + 1] = static_cast<int>(color.g * scale);
    data_[index + 2] = static_cast<int>(color.b * scale);
}

bool Image::init() {
    data_  = (unsigned char*)stbi__malloc(width_ * height_ * channels_);
    if(!data_) {
        printf("ERROR: Could not malloc memory for image\n");
        return false;
    } else {
        memset(data_, 0, width_ * height_ * channels_);
        return true;
    }
}