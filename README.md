# Raytracer
![Table with reflections](http://www.josephcmontgomery.com/uploads/4/5/8/3/45834621/all-features-scene3-light-test_orig.png)
## Overview
This is a 3D raytracer that produces static images from a file.

Features:
* kd-trees for faster ray intersection tests
* Toggleable Shadows
* Toggleable Reflections
* Blinn-Phong shading
* Toggleable diffuse and specular components
* Point and directional lighting
* Triangle and sphere and transformed sphere intersections 
* 5 debugging renders
* Performance reporting

## Building
### Prerequisites
Visual Studio 2017

Windows. Uses some Windows specific functions for parsing and building file paths. (Tested on Windows 7)

### Instructions
Must be built as x86 (32 bit). 

You shouldn't need to do anything beyond hitting 'Build' in Visual Studio, but if it whines about not finding glm headers or FreeImage bindings you'll have to take these additional steps.

Make these two changes in Project Properties (make sure it's set to apply to all configurations in all modes):
1. Go to C/C++ -> General -> Additional Include Directories. Link to the glm folder in the root of the RayTracer project.

2. Go to Linker->Input->Additional Dependencies and link to FreeImage.lib in the root of the RayTracer project.

## Built With
[FreeImage](http://freeimage.sourceforge.net/) To turn pixel buffers into images.

[GLM (OpenGL Mathematics)](https://glm.g-truc.net/0.9.8/index.html) For the math libraries.

## File Format Description
Info coming soon...

## Options
You can toggle features and debugging options.
### Features
Info coming soon...
### Debug views
Info coming soon...
### Multiple renders
Info coming soon...
### Performance Reporting
Info coming soon...
