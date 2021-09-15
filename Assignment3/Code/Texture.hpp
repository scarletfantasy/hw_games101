//
// Created by LEI XU on 4/27/19.
//

#ifndef RASTERIZER_TEXTURE_H
#define RASTERIZER_TEXTURE_H
#include "global.hpp"
#include <eigen3/Eigen/Eigen>
#include <opencv2/opencv.hpp>
class Texture{
private:
    cv::Mat image_data;

public:
    Texture(const std::string& name)
    {
        image_data = cv::imread(name);
        cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
        width = image_data.cols;
        height = image_data.rows;
    }

    int width, height;

    Eigen::Vector3f getColor(float u, float v)
    {
        auto u_img = u * width;
        auto v_img = (1 - v) * height;
        auto color = image_data.at<cv::Vec3b>(v_img, u_img);
        return Eigen::Vector3f(color[0], color[1], color[2]);
    }
    Eigen::Vector3f getColorBilinear(float u, float v) 
    {
        auto u_img = u * width;
        auto v_img = (1 - v) * height;
        auto us = floor(u * width);
        auto vs = floor((1 - v) * height);
        auto hor=u_img-us;
        auto ver=v_img-vs;
        auto ub = ceil(u * width);
        auto vb = ceil((1 - v) * height);
        auto colorld = image_data.at<cv::Vec3b>(vs, us);
        auto colorlu = image_data.at<cv::Vec3b>(vs, ub);
        auto colorrd = image_data.at<cv::Vec3b>(vb, us);
        auto colorru = image_data.at<cv::Vec3b>(vb, ub);
        auto color=(colorld*(1-hor)+colorrd*hor)*(1-ver)+(colorlu*(1-hor)+colorru*hor)*ver;
        //auto color = image_data.at<cv::Vec3b>(v_img, u_img);
        return Eigen::Vector3f(color[0], color[1], color[2]);
    }

};
#endif //RASTERIZER_TEXTURE_H
