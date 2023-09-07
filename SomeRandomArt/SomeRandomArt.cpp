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

const uint32_t threadCount = 8;

float uScale = 1, vScale = 1;
float uOffset = 0, vOffset = 0;

uint32_t mandelbrotIterations = 32;

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

    cv::putText(image, "(1) UV + Help", cv::Point(0, 32), cv::FONT_HERSHEY_COMPLEX, 1.0, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);
    cv::putText(image, "(2) Mandelbrot Black & White", cv::Point(0, 64), cv::FONT_HERSHEY_COMPLEX, 1.0, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);
    cv::putText(image, "(3) Mandelbrot Colored", cv::Point(0, 96), cv::FONT_HERSHEY_COMPLEX, 1.0, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);
    cv::putText(image, "(4) Mandelbrot Hue", cv::Point(0, 128), cv::FONT_HERSHEY_COMPLEX, 1.0, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);

    cv::putText(image, "(zxcvbnm) Zoom Mandelbrot Set", cv::Point(0, 192), cv::FONT_HERSHEY_COMPLEX, 1.0f, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);
    cv::putText(image, "(,.) Mandelbrot Set Iterations", cv::Point(0, 224), cv::FONT_HERSHEY_COMPLEX, 1.0f, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);
    cv::putText(image, "(s) Save Image to CWD", cv::Point(0, 256), cv::FONT_HERSHEY_COMPLEX, 1.0f, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);
}


using FractalFunction = std::function<void(uint32_t x, uint32_t y, uint32_t iterations, uint32_t maxIterations, uchar value, cv::Mat* image)>;

uchar MandelbrotIteration(float u, float v, uint32_t maxIterations, uint32_t* iterations)
{
    std::complex<float> z, c = { 2.0f * u - 1.5f, 2.0f * v - 1 };
    uint32_t i = 0;
    for (i = 0; i < maxIterations; i++)
    {
        if (abs(z) >= 2)
        {
            break;
        }
        z = z * z + c;
    }

    if (iterations != nullptr)
    {
        *iterations = i;
    }

    return (i / (float)maxIterations) * 255;
}

void MandelbrotSet(cv::Mat* image, FractalFunction func, uint32_t maxIterations, uint32_t offset, uint32_t batchSize)
{
    for (uint i = offset; i < offset + batchSize; i++)
    {
        uint32_t x = 0, y = 0;
        float u = 0, v = 0;
        x = i % width;
        y = floor(i / height);

        u = x / (float)width;
        v = y / (float)height;

        u = u * uScale + uOffset;
        v = v * vScale + vOffset;

        if (func != nullptr)
        {
            uint32_t iterations = 0;
            uchar value = MandelbrotIteration(u, v, maxIterations, &iterations);
            func(x, y, iterations, maxIterations, value, image);
        }
    }
}

void GenerateMandlebrotSet(cv::Mat* image, FractalFunction func, const uint32_t maxIterations = 32)
{
    if (func == nullptr)
    {
        throw std::invalid_argument("No func provided");
    }

    printf("Generating Mandlebrot Set with %i max iterations over %i threads\r\n", maxIterations, threadCount);

    uint32_t batchSize = (width * height) / threadCount;

    std::vector<std::thread> threads{};
    for (uint t = 0; t < threadCount; t++)
    {
        uint32_t offset = batchSize * t;
        std::thread thread([=]()
            {
                // printf("Mandelbrot thread %i dispatched\r\n", t);
                MandelbrotSet(image, func, maxIterations, offset, batchSize);
            }
        );
        threads.push_back(std::move(thread));
    }

    printf("All Mandlebrot threads dispatched\r\n");

    for (std::thread& thread : threads)
    {
        if (thread.joinable())
            thread.join();
    }

    printf("Mandlebrot set generated\r\n");
}

FractalFunction mandelbrotBW = [](uint32_t x, uint32_t y, uint32_t i, uint32_t mi, uchar value, cv::Mat* image)
    {
        image->at<cv::Vec3b>(y, x) = cv::Vec3b(value, value, value);
    };

FractalFunction mandelbrotColoured = [](uint32_t x, uint32_t y, uint32_t i, uint32_t mi, uchar value, cv::Mat* image)
    {
        // Red
        image->at<cv::Vec3b>(y, x)[2] = value;
        // Green
        image->at<cv::Vec3b>(y, x)[1] = (x / (float)width) * 255;
        // Blue
        image->at<cv::Vec3b>(y, x)[0] = (y / (float)height) * 255;
    };

FractalFunction mandelbrotHue = [](uint32_t x, uint32_t y, uint32_t i, uint32_t mi, uchar value, cv::Mat* image)
    {
        if (i == mi)
        {
            image->at<cv::Vec3b>(y, x) = cv::Vec3b(0, 0, 0);
        }
        image->at<cv::Vec3b>(y, x) = cv::Vec3b(value * 0.705f, 255, 255);
    };

cv::Point pt1;
//Callback for mousclick event, the x-y coordinate of mouse button-up and button-down are stored in two points pt1, pt2.
void mouse_click(int event, int x, int y, int flags, void* param)
{
    switch (event)
    {
    case cv::EVENT_LBUTTONDOWN:
        pt1.x = x;
        pt1.y = y;
        break;
    case cv::EVENT_LBUTTONUP:
    {
        if (x != pt1.x && y != pt1.y)
        {

            float newWidth = (x - pt1.x) / (float)width;
            float newHeight = (y - pt1.y) / (float)height;

            uOffset += pt1.x / (float)width * uScale;
            vOffset += pt1.y / (float)height * vScale;

            uScale *= newWidth;
            // vScale *= newHeight;
            vScale *= newWidth;
        }
    }
    break;

    case cv::EVENT_RBUTTONUP:
        uScale = 1;
        vScale = 1;
        uOffset = 0;
        vOffset = 0;
        break;
    }
}

int main()
{
    cv::Mat image(cv::Size(size, size), CV_8UC3, cv::Scalar(0, 0, 0));

    cv::namedWindow("Image", cv::WINDOW_NORMAL);
    cv::resizeWindow("Image", cv::Size(768, 768));
    cv::setMouseCallback("Image", mouse_click, 0);

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
            GenerateMandlebrotSet(&image, mandelbrotBW, mandelbrotIterations);
        }
        if (key == (int)'3')
        {
            GenerateMandlebrotSet(&image, mandelbrotColoured, mandelbrotIterations);
        }
        if (key == (int)'4')
        {
            GenerateMandlebrotSet(&image, mandelbrotHue, mandelbrotIterations);
            cvtColor(image, image, cv::COLOR_HSV2BGR);
        }

        if (key == (int)'z')
        {
            uScale = 1;
            vScale = 1;
            uOffset = 0;
            vOffset = 0;
        }
        else if (key == (int)'x')
        {
            uScale = 1 / 4.0f;
            vScale = 1 / 4.0f;
            uOffset = 0.055f;
            vOffset = 0.522f;
        }
        else if (key == (int)'c')
        {
            uScale = 1 / 16.0f;
            vScale = 1 / 16.0f;
            uOffset = 0.055f;
            vOffset = 0.522f;
        }
        else if (key == (int)'v')
        {
            uScale = 1 / 64.0f;
            vScale = 1 / 64.0f;
            uOffset = 0.055f;
            vOffset = 0.522f;
        }
        else if (key == (int)'b')
        {
            uScale = 1 / 256.0f;
            vScale = 1 / 256.0f;
            uOffset = 0.055f;
            vOffset = 0.522f;
        }
        else if (key == (int)'n')
        {
            uScale = 1 / 1024.0f;
            vScale = 1 / 1024.0f;
            uOffset = 0.055f;
            vOffset = 0.522f;
        }
        else if (key == (int)'m')
        {
            uScale = 1 / 4096.0f;
            vScale = 1 / 4096.0f;
            uOffset = 0.055f;
            vOffset = 0.522f;
        }

        if (key == (int)',')
        {
            mandelbrotIterations /= 2;
            if (mandelbrotIterations <= 0)
            {
                mandelbrotIterations = 1;
            }
            printf("Mandelbrot Iterations is now %i\r\n", mandelbrotIterations);
        }
        if (key == (int)'.')
        {
            mandelbrotIterations *= 2;
            if (mandelbrotIterations >= 256)
            {
                mandelbrotIterations = 256;
            }
            printf("Mandelbrot Iterations is now %i\r\n", mandelbrotIterations);
        }
    }
}