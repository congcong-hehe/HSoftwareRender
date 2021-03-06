// make by hyq
// 2021/11/17

#include "pipline.h"
#include <algorithm>
#include "../base.h"
#include "../shader/base_shader.h"
#include "image.h"

using namespace std;
using namespace Math;

Pipeline::Pipeline() {
    
}

Pipeline::~Pipeline() {

}

void Pipeline::init(const int w, const int h) {
    w_ = w;
    h_ = h;
    setImage(w, h, 3);
    z_buffer_.resize(w_ * h_, FLT_MAX);
}

void Pipeline::setImage(int w, int h, int c) {
    image_.setHeight(h);
    image_.setWidth(w);
    image_.setChannels(c);
    image_.init();
}

void Pipeline::vertexShader() {
    auto vs = dynamic_pointer_cast<BaseVSShader>(vs_);

    auto &vertices = mesh_->getVertices();
    vertices_.resize(vertices.size());
    for(int j = 0; j < vertices.size(); ++j) {
        // 顶点着色器
        vs->pos = vertices[j];
        vs->execute();

        vertices_[j] = vs->gl_position;
        vertices_[j].z = vertices[j].z; // 等于变换前还是后的z值，这是一个选择
    }

}

// 裁剪并使用透视除法
// 考虑简单的情况，裁剪的时候只对整个三角形进行裁剪，不考虑产生新的三角形
void Pipeline::perspectiveDivision() {
    auto &indices = mesh_->getVertexIndices();
    for(int i = 0; i < indices.size(); ++i) {
        bool tag = false;
        for(int j = 0; j < 3; ++j) {
            int index = indices[i][j];
            Vec4f &v = vertices_[index];
            // 裁剪 
            float abs_w = abs(v.w);
            if(abs(v.x) > abs_w || abs(v.y) > abs_w || abs(v.z) > abs_w) {
                tag = true;
                break;
            }

            v.x /= v.w;
            v.y /= v.w;
            // v.z /= v.w;
        }
        if(!tag) indices_.emplace_back(indices[i]);
    }
}

void Pipeline::viewPort() {
    // 视口变换
    for(int i = 0; i < vertices_.size(); ++i) {
        float y = (vertices_[i].x + 1) / 2.0f * w_;
        float x = (vertices_[i].y + 1) / 2.0f * h_;
        // opengl的窗口的图片的坐标是反的，所以需要颠倒一下
        x = h_ - x;
        vertices_[i].x = x;
        vertices_[i].y = y;
    }
}

// 计算一个点在三角形中的重心坐标
// https://zhuanlan.zhihu.com/p/65495373
Vec3f barycentric(Vec2f A, Vec2f B, Vec2f C, Vec2f P) {
    Vec3f v1(B.x - A.x, C.x - A.x, A.x - P.x);
    Vec3f v2(B.y - A.y, C.y - A.y, A.y - P.y);
    Vec3f u = cross(v1, v2);
    if(abs(u.z) > 1e-2)
        return Vec3f(1.0f - (u.x + u.y) / u.z, u.x / u.z, u.y / u.z);
    else return Vec3f(-1, 1, 1);
}

// https://github.com/ssloy/tinyrenderer/wiki/Lesson-1:-Bresenham%E2%80%99s-Line-Drawing-Algorithm
// bresenham 算法
void Pipeline::drawLine(int x0, int y0, int x1, int y1, Math::Vec3f &color) {
    bool steep = false;
    if(abs(x0 - x1) < abs(y0 - y1)) {
        swap(x0, y0);
        swap(x1, y1);
        steep = true;
    }
    if(x0 > x1) {
        swap(x0, x1);
        swap(y0, y1);
    }
    int dx = x1 - x0;
    int dy = y1 - y0;
    int derror2 = abs(dy) * 2;
    int error2 = 0;
    int y = y0;
    for(int x = x0; x <= x1; ++x) {
        if(steep) {
            image_.setColor(y, x, color);
        } else {
            image_.setColor(x, y, color);
        }
        error2 += derror2;
        if(error2 > dx) {
            y += (y1 > y0 ? 1 : -1);
            error2 -= dx * 2;
        }
    }
}

// 设置深度测试在片段着色器之前执行，在光栅化阶段
// 创建二维三角形的包围盒，对于包围盒中的像素，通过重心坐标判断是否在三角形中
// 光栅化的对象是每一个三角形，而片段着色器是对于三角形中的每一个像素
// 如果要将光栅化阶段和片段着色器分开，那么就必须存储光栅化后得到的像素， 那么存储就需要一笔开销
// https://developer.nvidia.com/content/life-triangle-nvidias-logical-pipeline 
// 从上面的链接可以看出光栅化之后，片段着色器和其在一个模块中分为多个运行
void Pipeline::rasterAndFragmentShader() {
    auto fs = dynamic_pointer_cast<BaseFSShader>(fs_);
    auto &uvs = mesh_->getUvs();
    switch(context_.mode) {
        case Mode::FILL:
            for(int j = 0; j < indices_.size(); ++j) {
                auto &vertex_indice = indices_[j];

                Vec2f box_min(static_cast<float>(image_.getWidth() - 1), static_cast<float>(image_.getWidth() - 1));
                Vec2f box_max(0.f, 0.f);

                // 计算包围盒
                for(int k = 0; k < 3; ++k) {
                    box_min.x = min(box_min.x, vertices_[vertex_indice.vec[k]].x);
                    box_min.y = min(box_min.y, vertices_[vertex_indice.vec[k]].y);
                    box_max.x = max(box_max.x, vertices_[vertex_indice.vec[k]].x);
                    box_max.y = max(box_max.y, vertices_[vertex_indice.vec[k]].y);
                }

                Vec2f A = Vec2f(vertices_[vertex_indice.x].x, vertices_[vertex_indice.x].y);
                Vec2f B = Vec2f(vertices_[vertex_indice.y].x, vertices_[vertex_indice.y].y);
                Vec2f C = Vec2f(vertices_[vertex_indice.z].x, vertices_[vertex_indice.z].y);

                // 对包围盒中的每一个像素处理
                for(int x = static_cast<int>(box_min.x); x <= box_max.x; ++x) {
                    for(int y = static_cast<int>(box_min.y); y <= box_max.y; ++y) {
                        Vec2f P = Vec2f(static_cast<float>(x), static_cast<float>(y));

                        Vec3f bc = barycentric(A, B, C, P);
                        if(bc.x < 0 || bc.y < 0 || bc.z < 0) continue;  // 点不在三角形中
                        
                        // early - z测试
                        if(context_.ifZtest) {
                            float z = bc.x * vertices_[vertex_indice.x].z + bc.y * vertices_[vertex_indice.y].z + bc.z * vertices_[vertex_indice.z].z;
                            if(z_buffer_[x * w_ + y] > z) {    // 通过深度测试
                                z_buffer_[x * w_ + y] = z;
                            }
                            else continue;
                        }

                        // 插值纹理
                        fs->uv = Vec2f(0.0f, 0.0f);
                        fs->uv += bc.x * uvs[vertex_indice.x];
                        fs->uv += bc.y * uvs[vertex_indice.y];
                        fs->uv += bc.z * uvs[vertex_indice.z];

                        // 顶点着色器
                        fs->execute();

                        image_.setColor(x, y, fs->frag_color);
                    }
                }
            }
        break;

        case Mode::WIRE :
            for(int j = 0; j < indices_.size(); ++j) {
                auto &vertex_indice = indices_[j];
                for(int i = 0; i < 3; ++i) {
                    Vec2f v0 = Vec2f(vertices_[vertex_indice.vec[i % 3]].x, vertices_[vertex_indice.vec[i % 3]].y);
                    Vec2f v1 = Vec2f(vertices_[vertex_indice.vec[(i + 1) % 3]].x, vertices_[vertex_indice.vec[(i + 1) % 3]].y);

                    fs->execute();
                    drawLine(static_cast<int>(v0.x), static_cast<int>(v0.y), static_cast<int>(v1.x), static_cast<int>(v1.y),
                        fs->frag_color);
                }
            }
        break;
    }
}

void Pipeline::renderMesh(shared_ptr<TriMesh> mesh) {
    mesh_ = mesh;
    vertexShader();
    perspectiveDivision();
    viewPort();
    // DEBUG
    rasterAndFragmentShader();
    // DEBUG
    image_.write(path_);
}