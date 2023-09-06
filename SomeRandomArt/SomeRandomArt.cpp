// SomeRandomArt.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>
#include <vector>

const uint32_t size = 1024;
const uint32_t width = size;
const uint32_t height = size;

void GenerateUV(cv::Mat& image)
{
    uint i = 0, x = 0, y = 0;
    float u = 0, v = 0;

    for (i = 0; i < size * size; i++)
    {
        x = i % size;
        y = floor(i / size);

        u = x / (float)size;
        v = y / (float)size;

        // Red
        image.at<cv::Vec3b>(y, x)[2] = u * 255;
        // Green
        image.at<cv::Vec3b>(y, x)[1] = v * 255;
        // Blue
        image.at<cv::Vec3b>(y, x)[0] = 0;
    }
}


void GenerateMandlebrotSet(cv::Mat& image)
{
    const uint32_t maxIteration = 32;

    uint i = 0, x = 0, y = 0;
    float u = 0, v = 0;

    uint32_t iteration = 0;
    float fraction;
    uint32_t value;

    printf("Generating Mandlebrot Set with %i max iterations\r\n", maxIteration);

    for (i = 0; i < size * size; i++)
    {
        x = i % size;
        y = floor(i / size);

        u = x / (float)size;
        v = y / (float)size;

        std::complex<float> z, c = { 2.0f * x / width - 1.5f, 2.0f * y / height - 1 };

        for (iteration = 0; iteration < maxIteration; iteration++)
        {
            if (abs(z) >= 2)
            {
                break;
            }
            z = z * z + c;
        }

        fraction = iteration / (float)maxIteration;
        value = fraction * 255;

        // Red
        image.at<cv::Vec3b>(y, x)[2] = value;
        // Green
        image.at<cv::Vec3b>(y, x)[1] = value;
        // Blue
        image.at<cv::Vec3b>(y, x)[0] = value;
    }

    printf("Mandlebrot set generated\r\n");
}

void GenerateMandlebrotSetColoured(cv::Mat& image)
{
    const uint32_t maxIteration = 25;

    uint i = 0, x = 0, y = 0;
    float u = 0, v = 0;

    uint32_t iteration = 0;
    float fraction;
    uint32_t value;

    printf("Generating Mandlebrot Set Coloured with %i max iterations\r\n", maxIteration);

    for (i = 0; i < size * size; i++)
    {
        x = i % size;
        y = floor(i / size);

        u = x / (float)size;
        v = y / (float)size;

        std::complex<float> z, c = { 2.0f * x / width - 1.5f, 2.0f * y / height - 1 };

        for (iteration = 0; iteration < maxIteration; iteration++)
        {
            if (abs(z) >= 2)
            {
                break;
            }
            z = z * z + c;
        }

        fraction = iteration / (float)maxIteration;
        value = fraction * 255;

        // Inner of the set
        if (iteration > maxIteration - 1)
        {
            image.at<cv::Vec3b>(y, x)[2] = 0;
            image.at<cv::Vec3b>(y, x)[1] = 0;
            image.at<cv::Vec3b>(y, x)[0] = 0;
        }
        else // Outer of the set
        {
            // Red
            image.at<cv::Vec3b>(y, x)[2] = value * u;
            // Green
            image.at<cv::Vec3b>(y, x)[1] = value * v;
            // Blue
            image.at<cv::Vec3b>(y, x)[0] = value;
        }
    }

    printf("Mandlebrot set coloured generated\r\n");
}

void GenerateMandlebrotSetHue(cv::Mat& image)
{
    const uint32_t maxIteration = 32;

    uint i = 0, x = 0, y = 0;
    float u = 0, v = 0;

    uint32_t iteration = 0;
    float fraction;
    uint32_t value;

    printf("Generating Mandlebrot Set Hue with %i max iterations\r\n", maxIteration);

    for (i = 0; i < size * size; i++)
    {
        x = i % size;
        y = floor(i / size);

        u = x / (float)size;
        v = y / (float)size;

        std::complex<float> z, c = { 2.0f * x / width - 1.5f, 2.0f * y / height - 1 };

        for (iteration = 0; iteration < maxIteration; iteration++)
        {
            if (abs(z) >= 2)
            {
                break;
            }
            z = z * z + c;
        }

        fraction = iteration / (float)maxIteration;
        value = fraction * 255;

        // Inner of the set
        if (iteration > maxIteration - 1)
        {
            image.at<cv::Vec3b>(y, x)[2] = 0;
            image.at<cv::Vec3b>(y, x)[1] = 0;
            image.at<cv::Vec3b>(y, x)[0] = 0;
        }
        else // Outer of the set
        {
            image.at<cv::Vec3b>(y, x)[0] = value * 0.705f; // hue (255 -> 180)
            image.at<cv::Vec3b>(y, x)[1] = 255;            // saturation
            image.at<cv::Vec3b>(y, x)[2] = 255;            // value
        }
    }
    cv::cvtColor(image, image, cv::COLOR_HSV2BGR);

    printf("Mandlebrot set hue generated\r\n");
}

int main()
{
    cv::Mat image(cv::Size(size, size), CV_8UC3, cv::Scalar(0, 0, 0));

    cv::namedWindow("Image", cv::WINDOW_AUTOSIZE);

    GenerateUV(image);

    for (;;)
    {
        cv::imshow("Image", image);

        // int key = cv::waitKey(1000 / 60);
        int key = cv::waitKey(1);

        if (key == 27) // ESC
        {
            break;
        }

        if (key == (int)'s')
        {
            printf("Writing image to image.png\r\n");
            cv::imwrite("image.png", image);
        }

        if (key == (int)'1')
        {
            GenerateUV(image);
        }
        if (key == (int)'2')
        {
            GenerateMandlebrotSet(image);
        }
        if (key == (int)'3')
        {
            GenerateMandlebrotSetColoured(image);
        }
        if (key == (int)'4')
        {
            GenerateMandlebrotSetHue(image);
        }
    }
}