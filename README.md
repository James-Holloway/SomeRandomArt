# Some Random Art

![Simple Colored Mandelbrot Set, Zoomed](./Example.png)

Just some random, unorganized but programmatic art I have created. At the moment I only have Mandelbrot Sets and a help screen.

The mandelbrot sets can be zoomed in on by clicking and dragging the zoom region and right clicking to unzoom. Use Z to reset zoom.

There are currently five different screens:
1. UV + Help
2. Mandelbrot Black & White
3. Mandelbrot Colored
4. Mandelbrot Hue
5. Cubic Fractal Black & White

I used `vcpkg install opencv` to install OpenCV 4.8.0, which is used to generate all the images along with usage of `std::complex<float>`. This makes use of 8 threads to concurrently generate different parts of the set at once (naively split evenly).