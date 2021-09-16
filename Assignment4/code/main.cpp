#include <chrono>
#include <iostream>
#include <opencv2/opencv.hpp>

std::vector<cv::Point2f> control_points;

void mouse_handler(int event, int x, int y, int flags, void *userdata) 
{
    if (event == cv::EVENT_LBUTTONDOWN && control_points.size() < 4) 
    {
        std::cout << "Left button of the mouse is clicked - position (" << x << ", "
        << y << ")" << '\n';
        control_points.emplace_back(x, y);
    }     
}

void naive_bezier(const std::vector<cv::Point2f> &points, cv::Mat &window) 
{
    auto &p_0 = points[0];
    auto &p_1 = points[1];
    auto &p_2 = points[2];
    auto &p_3 = points[3];

    for (double t = 0.0; t <= 1.0; t += 0.001) 
    {
        auto point = std::pow(1 - t, 3) * p_0 + 3 * t * std::pow(1 - t, 2) * p_1 +
                 3 * std::pow(t, 2) * (1 - t) * p_2 + std::pow(t, 3) * p_3;

        window.at<cv::Vec3b>(point.y, point.x)[2] = 255;
    }
}

cv::Point2f recursive_bezier(const std::vector<cv::Point2f> &control_points, float t) 
{
    // TODO: Implement de Casteljau's algorithm
    cv::Point2f p=cv::Point2f(0.0f,0.0f);
    int n=control_points.size()-1;
    for(int i=0;i<control_points.size();++i)
    {
        //std::cout<<i<<"\n";
        float c=1.0f;
        for(int j=1;j<=i;++j)
        {
            c*=(n-j+1);
            c/=j;
        }
        p+=std::pow(t,i)*std::pow(1.0f-t,n-i)*control_points[i]*c;
    }
    return p;

}

void bezier(const std::vector<cv::Point2f> &control_points, cv::Mat &window) 
{
    // TODO: Iterate through all t = 0 to t = 1 with small steps, and call de Casteljau's 
    // recursive Bezier algorithm.
    for (double t = 0.0; t <= 1.0; t += 0.001) 
    {
        //std::cout<<"point\n";
        auto point = recursive_bezier(control_points,t);
        //std::cout<<point.y<<" "<<point.x<<"\n";
        auto r=std::ceil(point.x);
        auto l=std::floor(point.x);
        auto u=std::ceil(point.y);
        auto d=std::floor(point.y);
        auto hor=point.x-l;
        auto ver=point.y-d;
        //std::cout<<255*(1.0f-hor)*(1.0-ver)<<" "<<255*(1.0f-hor)*ver<<" "<<255*hor*(1.0-ver)<<" "<<255*hor*ver<<"\n";
        window.at<cv::Vec3b>(d, l)[1] =std::min(255,int(window.at<cv::Vec3b>(d, l)[1]+ 255*(1.0f-hor)*(1.0-ver)));
        window.at<cv::Vec3b>(u, l)[1] = std::min(255,int(window.at<cv::Vec3b>(u, l)[1]+255*(1.0f-hor)*ver));
        window.at<cv::Vec3b>(d, r)[1] = std::min(255,int(window.at<cv::Vec3b>(d, r)[1]+255*hor*(1.0-ver)));
        window.at<cv::Vec3b>(u, r)[1] = std::min(255,int(window.at<cv::Vec3b>(u, r)[1]+255*hor*ver));
        /* window.at<cv::Vec3b>(d, l)[1] = 255;
        window.at<cv::Vec3b>(u, l)[1] = 255;
        window.at<cv::Vec3b>(d, r)[1] = 255;
        window.at<cv::Vec3b>(u, r)[1] = 255;  */
        //window.at<cv::Vec3b>(point.y, point.x)[1] = 255;
    }

}

int main() 
{
    cv::Mat window = cv::Mat(700, 700, CV_8UC3, cv::Scalar(0));
    cv::cvtColor(window, window, cv::COLOR_BGR2RGB);
    cv::namedWindow("Bezier Curve", cv::WINDOW_AUTOSIZE);

    cv::setMouseCallback("Bezier Curve", mouse_handler, nullptr);

    int key = -1;
    while (key != 27) 
    {
        for (auto &point : control_points) 
        {
            cv::circle(window, point, 3, {255, 255, 255}, 3);
        }

        if (control_points.size() == 4) 
        {
            //naive_bezier(control_points, window);
               bezier(control_points, window);

            cv::imshow("Bezier Curve", window);
            cv::imwrite("my_bezier_curve.png", window);
            key = cv::waitKey(0);

            return 0;
        }

        cv::imshow("Bezier Curve", window);
        key = cv::waitKey(20);
    }

return 0;
}
