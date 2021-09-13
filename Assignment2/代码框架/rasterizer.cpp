// clang-format off
//
// Created by goksu on 4/6/19.
//

#include <algorithm>
#include <vector>
#include "rasterizer.hpp"
#include <opencv2/opencv.hpp>
#include <math.h>


rst::pos_buf_id rst::rasterizer::load_positions(const std::vector<Eigen::Vector3f> &positions)
{
    auto id = get_next_id();
    pos_buf.emplace(id, positions);

    return {id};
}

rst::ind_buf_id rst::rasterizer::load_indices(const std::vector<Eigen::Vector3i> &indices)
{
    auto id = get_next_id();
    ind_buf.emplace(id, indices);

    return {id};
}

rst::col_buf_id rst::rasterizer::load_colors(const std::vector<Eigen::Vector3f> &cols)
{
    auto id = get_next_id();
    col_buf.emplace(id, cols);

    return {id};
}

auto to_vec4(const Eigen::Vector3f& v3, float w = 1.0f)
{
    return Vector4f(v3.x(), v3.y(), v3.z(), w);
}


static bool insideTriangle(float x, float y, const Vector3f* _v)
{   
    // TODO : Implement this function to check if the point (x, y) is inside the triangle represented by _v[0], _v[1], _v[2]
    Vector2f curpos=Vector2f(float(x),float(y));
    Vector3f a1=Vector3f( _v[0].x()-_v[1].x(),_v[0].y()-_v[1].y(),0);
    Vector3f a2=Vector3f(_v[0].x()-curpos.x(),_v[0].y()-curpos.y(),0);

    Vector3f b1=Vector3f( _v[1].x()-_v[2].x(),_v[1].y()-_v[2].y(),0);
    Vector3f b2=Vector3f(_v[1].x()-curpos.x(),_v[1].y()-curpos.y(),0);

    Vector3f c1=Vector3f( _v[2].x()-_v[0].x(),_v[2].y()-_v[0].y(),0);
    Vector3f c2=Vector3f(_v[2].x()-curpos.x(),_v[2].y()-curpos.y(),0);
    float a=a1.cross(a2).z();
    float b=b1.cross(b2).z();
    float c=c1.cross(c2).z();
    if((a>=0)&&(b>=0)&&(c>=0)||(a<=0)&&(b<=0)&&(c<=0))
    {
        return true;
    }
    return false;
}

static std::tuple<float, float, float> computeBarycentric2D(float x, float y, const Vector3f* v)
{
    float c1 = (x*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*y + v[1].x()*v[2].y() - v[2].x()*v[1].y()) / (v[0].x()*(v[1].y() - v[2].y()) + (v[2].x() - v[1].x())*v[0].y() + v[1].x()*v[2].y() - v[2].x()*v[1].y());
    float c2 = (x*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*y + v[2].x()*v[0].y() - v[0].x()*v[2].y()) / (v[1].x()*(v[2].y() - v[0].y()) + (v[0].x() - v[2].x())*v[1].y() + v[2].x()*v[0].y() - v[0].x()*v[2].y());
    float c3 = (x*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*y + v[0].x()*v[1].y() - v[1].x()*v[0].y()) / (v[2].x()*(v[0].y() - v[1].y()) + (v[1].x() - v[0].x())*v[2].y() + v[0].x()*v[1].y() - v[1].x()*v[0].y());
    return {c1,c2,c3};
}

void rst::rasterizer::draw(pos_buf_id pos_buffer, ind_buf_id ind_buffer, col_buf_id col_buffer, Primitive type)
{
    auto& buf = pos_buf[pos_buffer.pos_id];
    auto& ind = ind_buf[ind_buffer.ind_id];
    auto& col = col_buf[col_buffer.col_id];

    float f1 = (50 - 0.1) / 2.0;
    float f2 = (50 + 0.1) / 2.0;

    Eigen::Matrix4f mvp = projection * view * model;
    for (auto& i : ind)
    {
        Triangle t;
        Eigen::Vector4f v[] = {
                mvp * to_vec4(buf[i[0]], 1.0f),
                mvp * to_vec4(buf[i[1]], 1.0f),
                mvp * to_vec4(buf[i[2]], 1.0f)
        };
        //Homogeneous division
        for (auto& vec : v) {
            vec /= vec.w();
        }
        //Viewport transformation
        for (auto & vert : v)
        {
            vert.x() = 0.5*width*(vert.x()+1.0);
            vert.y() = 0.5*height*(vert.y()+1.0);
            vert.z() = vert.z() * f1 + f2;
        }

        for (int i = 0; i < 3; ++i)
        {
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
            t.setVertex(i, v[i].head<3>());
        }

        auto col_x = col[i[0]];
        auto col_y = col[i[1]];
        auto col_z = col[i[2]];

        t.setColor(0, col_x[0], col_x[1], col_x[2]);
        t.setColor(1, col_y[0], col_y[1], col_y[2]);
        t.setColor(2, col_z[0], col_z[1], col_z[2]);

        rasterize_triangle(t);
    }
}

//Screen space rasterization
void rst::rasterizer::rasterize_triangle(const Triangle& t) {
    auto v = t.toVector4();
    int x1=std::floor(std::min(t.v[0].x(),std::min(t.v[1].x(),t.v[2].x())));
    x1=x1<0?0:x1;
    int x2=std::ceil(std::max(t.v[0].x(),std::max(t.v[1].x(),t.v[2].x())));
    x2=x2>width-1?width-1:x2;
    int y1=std::floor(std::min(t.v[0].y(),std::min(t.v[1].y(),t.v[2].y())));
    y1=y1<0?0:y1;
    int y2=std::ceil(std::max(t.v[0].y(),std::max(t.v[1].y(),t.v[2].y())));
    y2=y2>width-1?width-1:y2;
    printf("%f,%f,%f\n%f,%f,%f,\n%f,%f,%f,\n",v[0].x(),v[0].y(),0.0,v[1].x(),v[1].y(),0.0,v[2].x(),v[2].y(),0.0);
    printf("%i %i %i %i %i %i\n",x1,x2,y1,y2,width,height);
    
    int count =0;
    for(int i=x1;i<=x2;++i)
    {
        for(int j=y1;j<=y2;++j)
        {
            Eigen::Vector3f fcolor=Eigen::Vector3f(0.0,0.0,0.0);
            for(int dx=0;dx<=1;++dx)
            {
                for(int dy=0;dy<=1;++dy)
                {
                    auto ind = (height*2-1-2*j-dy)*width*2 + 2*i+dx;
                    if(insideTriangle(i+dx*0.5,j+dy*0.5,t.v))
                    {
                        
                        auto[alpha, beta, gamma] = computeBarycentric2D(i+dx*0.5, j+dy*0.5, t.v);
                        float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
                        float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
                        z_interpolated *= w_reciprocal;
                        
                        if(z_interpolated<depth_buf_util[ind])
                        {
                            depth_buf_util[ind]=z_interpolated;
                            frame_buf_util[ind]=t.getColor();
                            //set_pixel(Vector3f(i,j,0),t.getColor());
                        }
                        
                        ++count;
                        
                    }
                    fcolor+=frame_buf_util[ind];
                }
            }
            fcolor/=4.0;
            //set_pixel(Vector3f(i,j,0),Eigen::Vector3f(255.0,255.0,255.0));
            set_pixel(Vector3f(i,j,0),fcolor);
            
            
        }
    }
    printf("in %f\n",float(count)/float(width*height));

    // TODO : Find out the bounding box of current triangle.
    // iterate through the pixel and find if the current pixel is inside the triangle

    //If so, use the following code to get the interpolated z value.
    /* auto[alpha, beta, gamma] = computeBarycentric2D(x, y, t.v);
    float w_reciprocal = 1.0/(alpha / v[0].w() + beta / v[1].w() + gamma / v[2].w());
    float z_interpolated = alpha * v[0].z() / v[0].w() + beta * v[1].z() / v[1].w() + gamma * v[2].z() / v[2].w();
    z_interpolated *= w_reciprocal; */

    // TODO : set the current pixel (use the set_pixel function) to the color of the triangle (use getColor function) if it should be painted.
}

void rst::rasterizer::set_model(const Eigen::Matrix4f& m)
{
    model = m;
}

void rst::rasterizer::set_view(const Eigen::Matrix4f& v)
{
    view = v;
}

void rst::rasterizer::set_projection(const Eigen::Matrix4f& p)
{
    projection = p;
}

void rst::rasterizer::clear(rst::Buffers buff)
{
    if ((buff & rst::Buffers::Color) == rst::Buffers::Color)
    {
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{0, 0, 0});
        
    }
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth)
    {
        std::fill(depth_buf.begin(), depth_buf.end(), std::numeric_limits<float>::infinity());
    }
    std::fill(depth_buf_util.begin(), depth_buf_util.end(), std::numeric_limits<float>::infinity());
    std::fill(frame_buf_util.begin(), frame_buf_util.end(), Eigen::Vector3f{0, 0, 0});
}

rst::rasterizer::rasterizer(int w, int h) : width(w), height(h)
{
    frame_buf.resize(w * h);
    depth_buf.resize(w * h);
    frame_buf_util.resize(w * h*4);
    depth_buf_util.resize(w * h*4);
}

int rst::rasterizer::get_index(int x, int y)
{
    return (height-1-y)*width + x;
}

void rst::rasterizer::set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color)
{
    //old index: auto ind = point.y() + point.x() * width;
    auto ind = (height-1-point.y())*width + point.x();
    frame_buf[ind] = color;

}

// clang-format on