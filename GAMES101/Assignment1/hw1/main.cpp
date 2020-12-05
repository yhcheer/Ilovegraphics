#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <eigen3/Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>

constexpr double MY_PI = 3.1415926;

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0], 0, 1, 0, -eye_pos[1], 0, 0, 1,
        -eye_pos[2], 0, 0, 0, 1;

    view = translate * view;

    return view;
}

Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    // TODO: Implement this function
    // Create the model matrix for rotating the triangle around the Z axis.
    // Then return it.
    float angle = rotation_angle / 180.0f * MY_PI;

    model << 
        std::cos(angle), -std::sin(angle), 0, 0,
        std::sin(angle), std::cos(angle), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1;

    return model;
}

// Rodrigues Rotation Formula
Eigen::Matrix4f get_rotation(Vector3f axis, float angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    Eigen::Matrix3f matrix_rotation = Eigen::Matrix3f::Identity();

    float angle_rad = angle / 180.0f * MY_PI;
    float nx = axis[0];
    float ny = axis[1];
    float nz = axis[2];

    Eigen::Matrix3f N;
    N << 
        0, -nz, ny,
        nz, 0, -nx,
        -ny, nx, 0;

    // std::cout << N << std::endl;
    
    matrix_rotation = std::cos(angle_rad) * Eigen::Matrix3f::Identity() + 
        (1 - std::cos(angle_rad)) * axis * axis.transpose() + 
        std::sin(angle_rad) * N;

    model.block<3,3>(0,0) = matrix_rotation;

    return model;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    // Students will implement this function

    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();
    
    // TODO: Implement this function
    // Create the projection matrix for the given parameters.
    // Then return it.
    Eigen::Matrix4f matrix_persp_to_ortho(4,4);
    
    float t, b, l, r, n, f;
    n = zNear;
    f = zFar;
    t = std::tan(eye_fov/2) * std::abs(n);
    b - -t;
    r = aspect_ratio * t;
    l = -r;

    // 1.zFar scale into zNear
    matrix_persp_to_ortho << 
        n, 0, 0, 0,
        0, n, 0, 0,
        0, 0, n+f, -n*f,
        0, 0, 1, 0;

    // 2.do ortho projection

    // 2.1 translation to origin point
    Eigen::Matrix4f ortho_translation(4,4), otrho_scaling(4,4);
    ortho_translation << 
        1, 0, 0, -(r+l)/2,
        0, 1, 0, -(t+b)/2,
        0, 0, 1, -(n+f)/2,
        0, 0, 0, 1;

    // 2.2 scale to canonical cube
    otrho_scaling << 
        2/(r-l), 0, 0, 0,
        0, 2/(t-b), 0, 0,
        0, 0, 2/(n-f), 0,
        0, 0, 0, 1;

    Eigen::Matrix4f matrix_ortho = otrho_scaling * ortho_translation;

    projection = matrix_ortho * matrix_persp_to_ortho;

    return projection;
}

int main(int argc, const char** argv)
{
    float angle = 0;
    bool command_line = false;
    std::string filename = "output.png";

    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // -r by default
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
        else
            return 0;
    }

    rst::rasterizer r(700, 700);

    Eigen::Vector3f eye_pos = {0, 0, 5};

    std::vector<Eigen::Vector3f> pos{{2, 0, -2}, {0, 2, -2}, {-2, 0, -2}};

    std::vector<Eigen::Vector3i> ind{{0, 1, 2}};

    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        //r.set_model(get_model_matrix(angle));
        r.set_model(get_rotation(Eigen::Vector3f(0,0,1), angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);

        cv::imwrite(filename, image);

        return 0;
    }

    Eigen::Vector3f rotation_axis = Eigen::Vector3f(0,0,1);

    while (key != 27) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth);

        //r.set_model(get_model_matrix(angle));
        r.set_model(get_rotation(rotation_axis, angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));

        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("image", image);
        key = cv::waitKey(10);

        std::cout << "frame count: " << frame_count++ << '\n';

        if (key == 'a') 
            angle += 10;
        else if (key == 'd') 
            angle -= 10;

        if(key == 'w')
            eye_pos[1] += 0.1;
        else if(key == 's')
            eye_pos[1] -= 0.1;

        if(key == 'x')
            rotation_axis = Eigen::Vector3f(1,0,0);
        else if(key == 'y')
            rotation_axis = Eigen::Vector3f(0,1,0);
        else if(key == 'z')
            rotation_axis = Eigen::Vector3f(0,0,1);
    }

    return 0;
}
