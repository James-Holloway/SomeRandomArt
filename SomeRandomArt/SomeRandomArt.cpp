// SomeRandomArt.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>
#include <vector>
#include <stack>
#include <thread>

const uint32_t size = 1024;
const uint32_t width = size;
const uint32_t height = size;

uint32_t threadCount = 8;

const cv::Size windowSize = cv::Size(768, 768);

struct UVScaleOffset
{
    double uScale = 1, vScale = 1;
    double uOffset = 0, vOffset = 0;
};

std::stack<UVScaleOffset> UVSOStack;

enum class CurrentScreen
{
    GenerateUV,
    MandelbrotBW,
    MandelbrotColored,
    MandelbrotHue,
    CubicFractalBW,
    Functions,
};

CurrentScreen screen = CurrentScreen::GenerateUV;
uint32_t mandelbrotIterations = 32;
uint32_t functionFunction = 0;
bool regenerateImage = false;

void GenerateUV(cv::Mat& image)
{
    uint i = 0, x = 0, y = 0;
    double u = 0, v = 0;

    for (i = 0; i < size * size; i++)
    {
        x = i % size;
        y = floor(i / size);

        u = x / (double)size;
        v = y / (double)size;

        // Red
        image.at<cv::Vec3b>(y, x)[2] = u * 255;
        // Green
        image.at<cv::Vec3b>(y, x)[1] = v * 255;
        // Blue
        image.at<cv::Vec3b>(y, x)[0] = 0;
    }

    uint32_t textY = 0;
    cv::putText(image, "(1) UV + Help", cv::Point(0, textY += 32), cv::FONT_HERSHEY_COMPLEX, 1.0, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);
    cv::putText(image, "(2) Mandelbrot Black & White", cv::Point(0, textY += 32), cv::FONT_HERSHEY_COMPLEX, 1.0, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);
    cv::putText(image, "(3) Mandelbrot Colored", cv::Point(0, textY += 32), cv::FONT_HERSHEY_COMPLEX, 1.0, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);
    cv::putText(image, "(4) Mandelbrot Hue", cv::Point(0, textY += 32), cv::FONT_HERSHEY_COMPLEX, 1.0, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);
    cv::putText(image, "(5) Cubic Fractal Black & White", cv::Point(0, textY += 32), cv::FONT_HERSHEY_COMPLEX, 1.0, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);
    cv::putText(image, "(6) Functions (Use arrow keys to navigate)", cv::Point(0, textY += 32), cv::FONT_HERSHEY_COMPLEX, 1.0, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);

    textY += 32;
    cv::putText(image, "(z) Reset Zoom", cv::Point(0, textY += 32), cv::FONT_HERSHEY_COMPLEX, 1.0f, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);
    cv::putText(image, "(,.) Increase/Decrease Mandelbrot Set Iterations", cv::Point(0, textY += 32), cv::FONT_HERSHEY_COMPLEX, 1.0f, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);
    cv::putText(image, "(s) Save Image to CWD", cv::Point(0, textY += 32), cv::FONT_HERSHEY_COMPLEX, 1.0f, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);
}


using FractalFunction = std::function<void(uint32_t x, uint32_t y, double u, double v, uint32_t iterations, uint32_t maxIterations, uchar value, cv::Mat* image)>;

typedef void FractalSet(cv::Mat* image, FractalFunction func, uint32_t maxIterations, uint32_t offset, uint32_t batchSize);

#pragma region Mandelbrot
uchar MandelbrotIteration(double u, double v, uint32_t maxIterations, uint32_t* iterations)
{
    std::complex<double> z, c = { 2.0 * u - 1.5, 2.0 * v - 1.0 };
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

    return fmod(i / (double)maxIterations, 1.0) * 255;
}

void MandelbrotSet(cv::Mat* image, FractalFunction func, uint32_t maxIterations, uint32_t offset, uint32_t batchSize)
{
    for (uint i = offset; i < offset + batchSize; i++)
    {
        uint32_t x = 0, y = 0;
        double u = 0, v = 0;
        x = i % width;
        y = floor(i / height);

        u = x / (double)width;
        v = y / (double)height;

        u = u * UVSOStack.top().uScale + UVSOStack.top().uOffset;
        v = v * UVSOStack.top().vScale + UVSOStack.top().vOffset;

        if (func != nullptr)
        {
            uint32_t iterations = 0;
            uchar value = MandelbrotIteration(u, v, maxIterations, &iterations);
            func(x, y, u, v, iterations, maxIterations, value, image);
        }
    }
}

FractalFunction mandelbrotBW = [](uint32_t x, uint32_t y, double u, double v, uint32_t i, uint32_t mi, uchar value, cv::Mat* image)
    {
        image->at<cv::Vec3b>(y, x) = cv::Vec3b(value, value, value);
    };

FractalFunction mandelbrotColored = [](uint32_t x, uint32_t y, double u, double v, uint32_t i, uint32_t mi, uchar value, cv::Mat* image)
    {
        if (i == mi)
        {
            image->at<cv::Vec3b>(y, x) = cv::Vec3b(0, 0, 0);
        }
        else
        {
            // Red
            image->at<cv::Vec3b>(y, x)[2] = value;
            // Green
            image->at<cv::Vec3b>(y, x)[1] = u * 255;
            // Blue
            image->at<cv::Vec3b>(y, x)[0] = v * 255;

        }
    };

FractalFunction mandelbrotHue = [](uint32_t x, uint32_t y, double u, double v, uint32_t i, uint32_t mi, uchar value, cv::Mat* image)
    {
        if (i == mi)
        {
            image->at<cv::Vec3b>(y, x) = cv::Vec3b(0, 0, 0);
        }
        else
            image->at<cv::Vec3b>(y, x) = cv::Vec3b(value * 0.705f, 255, 255);
    };

#pragma endregion

#pragma region Cubic Fractal
uchar CubicFractalIteration(double u, double v, uint32_t maxIterations, uint32_t* iterations)
{
    std::complex<double> z, c = { 4.0 * u - 2.0, 4.0 * v - 2.0 };
    uint32_t i = 0;
    for (i = 0; i < maxIterations; i++)
    {
        if (abs(z) >= 3)
        {
            break;
        }
        z = (z * z * z) + c;
    }

    if (iterations != nullptr)
    {
        *iterations = i;
    }

    return fmod(i / (double)maxIterations, 1.0) * 255;
}

void CubicFractalSet(cv::Mat* image, FractalFunction func, uint32_t maxIterations, uint32_t offset, uint32_t batchSize)
{
    for (uint i = offset; i < offset + batchSize; i++)
    {
        uint32_t x = 0, y = 0;
        double u = 0, v = 0;
        x = i % width;
        y = floor(i / height);

        u = x / (double)width;
        v = y / (double)height;

        u = u * UVSOStack.top().uScale + UVSOStack.top().uOffset;
        v = v * UVSOStack.top().vScale + UVSOStack.top().vOffset;

        if (func != nullptr)
        {
            uint32_t iterations = 0;
            uchar value = CubicFractalIteration(u, v, maxIterations, &iterations);
            func(x, y, u, v, iterations, maxIterations, value, image);
        }
    }
}

#pragma endregion

#pragma region Functions

// Functions Based on https://flam3.com/flame_draves.pdf
void FunctionsSet(cv::Mat* image, FractalFunction func, uint32_t functionType, uint32_t offset, uint32_t batchSize)
{
    for (uint i = offset; i < offset + batchSize; i++)
    {
        uint32_t x = 0, y = 0;
        double u = 0, v = 0;
        x = i % width;
        y = floor(i / height);

        u = x / (double)width;
        v = y / (double)height;

        // u = u * UVSOStack.top().uScale + UVSOStack.top().uOffset;
        // v = v * UVSOStack.top().vScale + UVSOStack.top().vOffset;

        uchar blue = 0, green = 0, red = 0;

        // Map UV from [0,1] to [-1,1] space
        u = (u - 0.5) * 2;
        v = (v - 0.5) * 2;

        switch (functionType)
        {
        case 0:
        default:
        {
            double r = sqrt(u * u + v * v);

            // Sinusoidal
            double u0 = sin(u);
            double v0 = sin(v);

            // Swirl
            double u1 = (u * sin(r * r)) - (v * cos(r * r));
            double v1 = (u * cos(r * r)) + (v * sin(r * r));

            // Spherical
            double u2 = (1 / (r * r)) * u;
            double v2 = (1 / (r * r)) * v;


            double uTotal = (u0 + u1 + u2);
            double vTotal = (v0 + v1 + v2);

            red = uTotal * 255;
            green = vTotal * 255;
            blue = (red + green) / 2;
        }
        break;
        case 1:
        {
            double r = sqrt(u * u + v * v);

            // Linear
            double u0 = u;
            double v0 = v;

            // Swirl
            double u1 = (u * sin(r * r)) - (v * cos(r * r));
            double v1 = (u * cos(r * r)) + (v * sin(r * r));

            // Spherical
            double u2 = (1 / (r * r)) * u;
            double v2 = (1 / (r * r)) * v;


            double uTotal = (u0 + u1 + u2);
            double vTotal = (v0 + v1 + v2);

            red = ((uTotal + vTotal) / 2.0) * 255;
            green = red;
            blue = red;
        }
        break;
        case 2:
        {
            double c = 4.9 / 5.23;
            double f = 7.1 / 16.2;

            double u0 = u + c * sin(tan(3 * v));
            double v0 = v + f * sin(tan(3 * u));

            c = 8 / 25.0;
            f = 3.0 / 50.0;

            double u1 = u + c * sin(tan(3 * v));
            double v1 = v + f * sin(tan(3 * u));

            c = 91 / 100.0;
            f = 4 / 6.6;

            double u2 = u + c * sin(tan(3 * v));
            double v2 = v + f * sin(tan(3 * u));

            double uTotal = (u0 + u1 + u2);
            double vTotal = (v0 + v1 + v2);

            red = sin(uTotal) * 255;
            green = cos(vTotal) * 255;
            blue = (sin(u0 / v0) + cos(u1 / v1) + tan(v2 / u2)) * 255;
        }
        break;
        }

        image->at<cv::Vec3b>(y, x) = cv::Vec3b(blue, green, red);
    }
}


#pragma endregion

void GenerateBatchedSet(cv::Mat* image, FractalFunction func, FractalSet setFunc, const uint32_t maxIterations = 32)
{
    uint32_t batchSize = (width * height) / threadCount;

    std::vector<std::thread> threads{};
    for (uint t = 0; t < threadCount; t++)
    {
        uint32_t offset = batchSize * t;
        std::thread thread([=]()
            {
                setFunc(image, func, maxIterations, offset, batchSize);
            }
        );
        threads.push_back(std::move(thread));
    }

    for (std::thread& thread : threads)
    {
        if (thread.joinable())
            thread.join();
    }
}

cv::Point pt1;
//Callback for mousclick event, the x-y coordinate of mouse button-up and button-down are stored in two points pt1, pt2.
void mouse_click(int event, int x, int y, int flags, void* param)
{
    // Don't handle mouse if we are not in a mandelbrot screen
    if (screen != CurrentScreen::MandelbrotBW && screen != CurrentScreen::MandelbrotColored && screen != CurrentScreen::MandelbrotHue && screen != CurrentScreen::CubicFractalBW)
        return;

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
            double newWidth = (x - pt1.x) / (double)windowSize.width;
            double newHeight = (y - pt1.y) / (double)windowSize.height;

            UVScaleOffset newUV;

            newUV.uOffset = UVSOStack.top().uOffset + ((pt1.x / (double)windowSize.width) * UVSOStack.top().uScale);
            newUV.vOffset = UVSOStack.top().vOffset + ((pt1.y / (double)windowSize.height) * UVSOStack.top().vScale);

            newUV.uScale = UVSOStack.top().uScale * newWidth;
            newUV.vScale = UVSOStack.top().vScale * newWidth;

            UVSOStack.push(newUV);

            printf("Zoom is now %llix\r\n", (long long)(1.0 / newUV.uScale));
            regenerateImage = true;
        }
    }
    break;

    case cv::EVENT_RBUTTONUP:
        if (UVSOStack.size() > 1)
            UVSOStack.pop();

        printf("Zoom is now %llix\r\n", (long long)(1.0 / UVSOStack.top().uScale));

        regenerateImage = true;
        break;

    case cv::EVENT_MOUSEWHEEL:
        int wheelDelta = cv::getMouseWheelDelta(flags);

        if (wheelDelta > 0) // up
        {
            UVScaleOffset newUV;

            newUV.uOffset = UVSOStack.top().uOffset + ((x / (double)windowSize.width / 2) * UVSOStack.top().uScale);
            newUV.vOffset = UVSOStack.top().vOffset + ((y / (double)windowSize.height / 2) * UVSOStack.top().vScale);

            newUV.uScale = UVSOStack.top().uScale / 2;
            newUV.vScale = UVSOStack.top().vScale / 2;

            UVSOStack.push(newUV);

            printf("Zoom is now %llix\r\n", (long long)(1.0 / newUV.uScale));
            regenerateImage = true;
        }
        else if (wheelDelta < 0) // down
        {
            if (UVSOStack.size() > 1)
            {
                UVSOStack.pop();

                regenerateImage = true;
            }
        }
    }
}

int main()
{
    cv::Mat image(cv::Size(size, size), CV_8UC3, cv::Scalar(0, 0, 0));

    cv::namedWindow("Image", cv::WINDOW_NORMAL);
    cv::resizeWindow("Image", windowSize);
    cv::setMouseCallback("Image", mouse_click, 0);

    GenerateUV(image);

    UVSOStack.push(UVScaleOffset());

    threadCount = std::max<uint32_t>(1u, std::thread::hardware_concurrency());

    for (;;)
    {
        cv::imshow("Image", image);

        int key = cv::waitKeyEx(1);

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
            screen = CurrentScreen::GenerateUV;
            regenerateImage = true;
        }
        if (key == (int)'2')
        {
            screen = CurrentScreen::MandelbrotBW;
            regenerateImage = true;
        }
        if (key == (int)'3')
        {
            screen = CurrentScreen::MandelbrotColored;
            regenerateImage = true;
        }
        if (key == (int)'4')
        {
            screen = CurrentScreen::MandelbrotHue;
            regenerateImage = true;
        }
        if (key == (int)'5')
        {
            screen = CurrentScreen::CubicFractalBW;
            regenerateImage = true;
        }
        if (key == (int)'6')
        {
            screen = CurrentScreen::Functions;
            regenerateImage = true;
        }

        if (screen == CurrentScreen::MandelbrotBW || screen == CurrentScreen::MandelbrotColored || screen == CurrentScreen::MandelbrotHue || screen == CurrentScreen::CubicFractalBW)
        {
            if (key == (int)'z')
            {
                while (UVSOStack.size() > 1)
                    UVSOStack.pop();

                printf("Zoom is now 1x\r\n");
                regenerateImage = true;
            }

            if (UVSOStack.size() > 1)
            {
                double moveAmount = 0.125;

                if (key == 0x10000 * 0x25) // VK_LEFT
                {
                    UVSOStack.top().uOffset -= moveAmount * UVSOStack.top().uScale;
                    regenerateImage = true;
                }
                else if (key == 0x10000 * 0x26) // VK_UP
                {
                    UVSOStack.top().vOffset -= moveAmount * UVSOStack.top().vScale;
                    regenerateImage = true;
                }
                else if (key == 0x10000 * 0x27) // VK_RIGHT
                {
                    UVSOStack.top().uOffset += moveAmount * UVSOStack.top().uScale;
                    regenerateImage = true;
                }
                else if (key == 0x10000 * 0x28) // VK_DOWN
                {
                    UVSOStack.top().vOffset += moveAmount * UVSOStack.top().vScale;
                    regenerateImage = true;
                }
            }

            if (key == (int)',')
            {
                mandelbrotIterations /= 2;
                if (mandelbrotIterations <= 0)
                {
                    mandelbrotIterations = 1;
                }
                printf("Mandelbrot Iterations is now %i\r\n", mandelbrotIterations);
                regenerateImage = true;
            }
            if (key == (int)'.')
            {
                mandelbrotIterations *= 2;
                if (mandelbrotIterations >= 1024)
                {
                    mandelbrotIterations = 1024;
                }
                printf("Mandelbrot Iterations is now %i\r\n", mandelbrotIterations);
                regenerateImage = true;
            }
        }
        else if (screen == CurrentScreen::Functions)
        {
            if (key == 0x10000 * 0x25) // VK_LEFT
            {
                if (functionFunction > 0)
                    functionFunction--;
                printf("Function Function is now %i\r\n", functionFunction);
                regenerateImage = true;
            }
            else if (key == 0x10000 * 0x27) // VK_RIGHT
            {
                if (functionFunction < 2) // max functions
                    functionFunction++;
                printf("Function Function is now %i\r\n", functionFunction);
                regenerateImage = true;
            }
        }

        if (regenerateImage)
        {
            switch (screen)
            {
            case CurrentScreen::GenerateUV:
                GenerateUV(image);
                break;
            case CurrentScreen::MandelbrotBW:
                printf("Generating Mandelbrot Set BW with %i max iterations over %i threads\r\n", mandelbrotIterations, threadCount);
                GenerateBatchedSet(&image, mandelbrotBW, MandelbrotSet, mandelbrotIterations);
                break;
            case CurrentScreen::MandelbrotColored:
                printf("Generating Mandelbrot Set Colored with %i max iterations over %i threads\r\n", mandelbrotIterations, threadCount);
                GenerateBatchedSet(&image, mandelbrotColored, MandelbrotSet, mandelbrotIterations);
                break;
            case CurrentScreen::MandelbrotHue:
                printf("Generating Mandelbrot Set Hue with %i max iterations over %i threads\r\n", mandelbrotIterations, threadCount);
                GenerateBatchedSet(&image, mandelbrotHue, MandelbrotSet, mandelbrotIterations);
                cvtColor(image, image, cv::COLOR_HSV2BGR);
                break;
            case CurrentScreen::CubicFractalBW:
                printf("Generating Mandelbrot Set BW with %i max iterations over %i threads\r\n", mandelbrotIterations, threadCount);
                GenerateBatchedSet(&image, mandelbrotBW, CubicFractalSet, mandelbrotIterations);
                break;
            case CurrentScreen::Functions:
                printf("Generating Functional art over %i threads\r\n", threadCount);
                GenerateBatchedSet(&image, nullptr, FunctionsSet, functionFunction);
                break;
            }
            regenerateImage = false;
        }
    }
}