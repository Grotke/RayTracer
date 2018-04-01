#pragma once
#include <string>

#include "FreeImage.h"

using namespace std;
class Renderer
{
public:
	Renderer(int w, int h);
	~Renderer();
	void createImage(BYTE* pixels, string outputFileName);

private:
	int width;
	int height;
};

