# Some Random Art

![Simple Colored Mandelbrot Set, Zoomed](./Example.png)

Just some random, unorganized but programmatic art I have created. At the moment I only have Mandelbrot Sets and a help screen.

The mandelbrot sets can be zoomed in on by clicking and dragging the zoom region and right clicking to reset zoom. Alternatively use Z, X, C, V, B, N & M to zoom in on a predefined part of the mandelbrot set. Once you have zoomed you need to press the corresponding button again:

1. UV + Help
2. Mandelbrot Black & White
3. Mandelbrot Colored
4. Mandelbrot Hue

I used `vcpkg install opencv` to install OpenCV 4.8.0, which is used to generate all the images along with usage of `std::complex<float>`. This makes use of 8 threads to concurrently generate different parts of the set at once (naively split evenly).