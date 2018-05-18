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
Some default scene files are contained in test_scenes. Scenes are defined by a series of lines with a command and the parameters to the command. Blank lines are skipped. Lines starting with # are comments and ignored by the parser.

Parameters can be defined with an integer or floating point number (1 or 1.0)

A short sample scene file:
    
    #Make a 160x120 image named raytrace.png with a maximum recursive depth of 4
    size 160 120
    maxdepth 4
    output raytrace.png
    
    #Create a camera at the origin looking down the -Z axis with a 45 degree vertical fov and a directional light pointing down
    camera 0 0 0 0 0 -1 0 1 0 45.0
    directional 0 1 0 1 1 1
    
    #Set diffuse color to red
    diffuse 1 0 0 
    
    #Create a sphere at (0,0,0), translate it to (1,0,0) and remove the transform
    pushTransform
    translate 1 0 0
    sphere 0 0 0 1.0
    popTransform
    
    #Set diffuse color to green
    diffuse 0 1 0
    
    #Transform was popped off so sphere stays at (0,0,0)
    sphere 0 0 0 1.0

#### General
    size width height: Defines the image size.
    maxdepth depth: The maximum depth (number of bounces) for a ray (default is 5).
    output filename: The output file to which the image should be written. Defaults to test.png. Images are always png.

#### Camera
Only one camera is used. If more than one camera is defined, the last one in the file is used.

    camera lookfromx lookfromy lookfromz lookatx lookaty lookatz upx upy upz fovy : fovy means vertical fov, the horizontal fov is calculated.
    
#### Lighting
Lights are either point or directional. Any number of lights can be defined.

    directional x y z r g b: The direction to the light source and the color.
    point x y z r g b: The location of a point source and the color.
    attenuation const linear quadratic: Sets the constant, linear and quadratic attenuations (default 1,0,0).
    ambient r g b: The global ambient color to be added for each object (default is .2,.2,.2).

#### Geometry
Only spheres, triangles and their transformed variants (ellipsoids) are defined.
For triangles, you can define vertices with or without normals. 

Once the geometry command is issued (with the sphere or tri command), the currently defined material properties and transform are associated with the object. 

    sphere x y z radius : Defines a sphere with a given position and radius.
    maxverts number: Defines a maximum number of vertices for later triangle specifications. It must be set before vertices are defined.
    maxvertnorms number: Defines a maximum number of vertices with normals for later specifications. It must be set before vertices with normals are defined.
    vertex x y z: Defines a vertex at the given location. The vertex is put into a pile, starting to be numbered at 0.
    vertexnormal x y z nx ny nz: Similar to the above, but define a surface normal with each vertex. The vertex and vertexnormal set of vertices are completely independent (as are maxverts and maxvertnorms).
    tri v1 v2 v3: Create a triangle out of the vertices involved (which have previously been specified with the vertex command). The vertices are assumed to be specified in counter-clockwise order.
    trinormal v1 v2 v3: Same as above but for vertices specified with normals. In this case, each vertex has an associated normal.

#### Transformations
You can apply transformations to any geometry or light source. Transformations are stored in a stack so you can apply different sets of transforms to different geometry by pushing and popping transform operations.

    translate x y z: A translation 3-vector.
    rotate x y z angle: Rotate by angle (in degrees) about the given axis.
    scale x y z: Scale by the corresponding amount in each axis (a non-uniform scaling).
    pushTransform: Push the current modeling transform on the stack.
    popTransform: Pop the current transform from the stack.

#### Materials
Once defined, material properties apply to every object defined afterwards until the property is overwritten. All default to 0.

    diffuse r g b: specifies the diffuse color of the surface.
    specular r g b: specifies the specular color of the surface.
    shininess s: specifies the shininess of the surface.
    emission r g b: gives the emissive color of the surface.


## Options
You can toggle features and debugging options.
### Toggleable Features
Shadows

Reflections

Diffuse Lighting

Specular Lighting

Performance Reporting

### Debug views
Normal Map

Shadow Map

Specular Lighting Intensity Map

Diffuse Lighting Intensity Map

### Making consecutive renders
Info coming soon...
### Performance Reporting
Performance reporting is turned on by default. Every render creates a report in the reports/ directory with the name \[Output file name]_report.txt.

Reports note:
* image size
* input file and output file
* number of pixels processed, average milliseconds spent calculating each pixel
* total time to render scene
* number of directional and point lights
* number of objects, number of spheres and triangles
