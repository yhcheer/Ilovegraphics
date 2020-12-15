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

        float sample_coord_left, sample_coord_right, sample_coord_down, sample_coord_up;
        if(u_img - (int)u_img < 0.5f)
        {
            sample_coord_left = std::max(0.5f, (int)u_img - 0.5f);
            sample_coord_right = (int)u_img + 0.5f;
        }
        else
        {
            sample_coord_left = (int)u_img + 0.5f;
            sample_coord_right = std::min((int)u_img + 1.5f, width - 0.5f);
        }
        //
        if(v_img - (int)v_img < 0.5f)
        {
            sample_coord_down = std::max(0.5f, (int)v_img - 0.5f);
            sample_coord_up = (int)v_img + 0.5f;
        }
        else
        {
            sample_coord_down = (int)v_img + 0.5f;
            sample_coord_up = std::min((int)v_img + 1.5f, height - 0.5f);
        }


        auto color_00 = image_data.at<cv::Vec3b>(sample_coord_down, sample_coord_left);
        auto color_01 = image_data.at<cv::Vec3b>(sample_coord_up, sample_coord_left);
        auto color_10 = image_data.at<cv::Vec3b>(sample_coord_down, sample_coord_right);
        auto color_11 = image_data.at<cv::Vec3b>(sample_coord_up, sample_coord_right);

        auto color_interpolation_u_down = (u_img - sample_coord_left) * color_00 + (sample_coord_right - u_img) * color_01;
        auto color_interpolation_u_up = (u_img - sample_coord_left) * color_10 + (sample_coord_right - u_img) * color_11;
        auto color = (v_img - sample_coord_down) * color_interpolation_u_down + (sample_coord_up - v_img) * color_interpolation_u_up;

        return Eigen::Vector3f(color[0], color[1], color[2]);
    }

};
#endif //RASTERIZER_TEXTURE_H
