#include <CanvasTriangle.h>
#include <CanvasPoint.h>
#include <Colour.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <vector>

#include "TextureMap.h"

#define WIDTH 320
#define HEIGHT 240

#define STROKED 0
#define FILLED 1

std::vector<float> interpolateSingleFloats(float from, float to, int numberOfValues) {
	std::vector<float> result;
	float spacing;
	spacing = (to - from) / (numberOfValues - 1);
	for (int i = 0; i < numberOfValues; i++) {
		result.push_back(from+spacing*i);
	}
	return result;
}

std::vector<glm::vec3> interpolateThreeElementValues(glm::vec3 from, glm::vec3 to, int numberOfValues) {
	std::vector<glm::vec3> result;
	glm::vec3 spacing;
	spacing = (to - from) / float(numberOfValues - 1);
	for (int i = 0; i < numberOfValues; i++) {
		result.push_back(from+spacing * float(i));
	}
	return result;
}

std::vector<std::vector<glm::vec3>> colourInterpolation2D() {
	glm::vec3 topLeft(255, 0, 0);        // red
	glm::vec3 topRight(0, 0, 255);       // blue
	glm::vec3 bottomRight(0, 255, 0);    // green
	glm::vec3 bottomLeft(255, 255, 0);   // yellow

	std::vector<glm::vec3> firstCol, lastCol;
	std::vector<std::vector<glm::vec3>> canvas;
	std::vector<glm::vec3> temp;
	firstCol = interpolateThreeElementValues(topLeft, bottomLeft, HEIGHT);
	lastCol = interpolateThreeElementValues(topRight, bottomRight, HEIGHT);
	for (size_t y = 0; y < HEIGHT; y++) {
		temp = interpolateThreeElementValues(firstCol[y],lastCol[y],WIDTH);
		canvas.push_back(temp);
	}
	return canvas;
}

// we have convertToBarycentricCoordinates() : x, y -> u, v, w
// we want a function, barycentricToRGB(): u, v, w -> r, g, b
glm::vec3 barycentricToRGB(glm::vec3 pos) {
	return pos * float(255);
}

void line(DrawingWindow &window, CanvasPoint from, CanvasPoint to, Colour colour) {
	float xDiff = (to.x-from.x);
	float yDiff = (to.y-from.y);
	float numSteps = std::max(abs(xDiff), abs(yDiff));
	if (numSteps == 0) {
		uint32_t uintColour = (255 << 24) + (colour.red << 16) + (colour.green << 8) + colour.blue;
		window.setPixelColour(round(from.x), round(from.y), uintColour);
		return;
	}
	float xStepSize = xDiff/numSteps;
	float yStepSize = yDiff/numSteps;
	// repeated addition instead of multiplication (out of bounds)
	// float x = from.x;
	// float y = from.y;
	// for (int i = 0; i <= numSteps; i++) {
	// 	x += xStepSize;
	// 	y += yStepSize;
	// 	uint32_t uintColour = (255 << 24) + (colour.red << 16) + (colour.green << 8) + colour.blue;
	// 	window.setPixelColour(int(x), int(y), uintColour);
	// }

	for (float i=0.0; i<=numSteps; i++) {
		float x = from.x + xStepSize*i;
		float y = from.y + yStepSize*i;
		uint32_t uintColour = (255 << 24) + (colour.red << 16) + (colour.green << 8) + colour.blue;
		window.setPixelColour(round(x), round(y), uintColour);
	}
}

std::vector<uint32_t> getTexturedLine(CanvasPoint from, CanvasPoint to, std::vector<std::vector<uint32_t>> texture) {
	float xDiff = to.texturePoint.x-from.texturePoint.x;
	float yDiff = to.texturePoint.y-from.texturePoint.y;
	float numSteps = std::max(abs(xDiff), abs(yDiff));
	std::vector<uint32_t> result;

	if (numSteps == 0) {
		return {texture[from.texturePoint.y][from.texturePoint.x]};
	}
	float xStepSize = xDiff/numSteps;
	float yStepSize = yDiff/numSteps;
	for (float i=0.0; i<=numSteps; i++) {
		float x = from.texturePoint.x + xStepSize*i;
		float y = from.texturePoint.y + yStepSize*i;
		result.push_back(texture[y][x]);
	}
	return result;
}

void texturedLine(DrawingWindow &window, CanvasPoint from, CanvasPoint to, std::vector<std::vector<uint32_t>> texture) {
	float xDiff = (to.x-from.x);
	float yDiff = (to.y-from.y);
	float numSteps = std::max(abs(xDiff), abs(yDiff));
	if (numSteps == 0) {
		uint32_t uintColour = texture[from.texturePoint.x][from.texturePoint.y];
		window.setPixelColour(round(from.x), round(from.y), uintColour);
		return;
	}
	float xStepSize = xDiff/numSteps;
	float yStepSize = yDiff/numSteps;

	std::vector<uint32_t> texturedLine = getTexturedLine(from, to, texture);
	int texturedLineLength = texturedLine.size();
	for (float i=0.0; i<=numSteps; i++) {
		float x = from.x + xStepSize*i;
		float y = from.y + yStepSize*i;
		int index = texturedLineLength/numSteps * i;
		uint32_t uintColour = texturedLine[index];
		window.setPixelColour(round(x), round(y), uintColour);
	}

}

void drawStrokedTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour) {
	line(window, triangle.v0(), triangle.v1(), colour);
	line(window, triangle.v1(), triangle.v2(), colour);
	line(window, triangle.v2(), triangle.v0(), colour);
}

Colour randomColour() {
	return Colour(rand() % 256, rand() % 256, rand() % 256);
}

CanvasTriangle randomTriangle () {
	CanvasPoint v0 = CanvasPoint(rand() % WIDTH, rand() % HEIGHT);
	CanvasPoint v1 = CanvasPoint(rand() % WIDTH, rand() % HEIGHT);
	CanvasPoint v2 = CanvasPoint(rand() % WIDTH, rand() % HEIGHT);
	return CanvasTriangle(v0, v1, v2);
}

CanvasPoint pointOnLineY(float y, CanvasPoint p1, CanvasPoint p2) {
	float proportion = (y - p1.y) / (p2.y - p1.y);
	float x = proportion * (p2.x - p1.x) + p1.x;

	CanvasPoint result = CanvasPoint(x, y);
	float textureX = proportion * (p2.texturePoint.x-p1.texturePoint.x) + p1.texturePoint.x;
	float textureY = proportion * (p2.texturePoint.y-p1.texturePoint.y) + p1.texturePoint.y;
	result.texturePoint = TexturePoint(textureX, textureY);

	return result;
}

std::vector<CanvasTriangle> splitTriangle(CanvasTriangle triangle) {
	CanvasTriangle top, bottom;
	std::vector<CanvasPoint> points = {triangle.v0(), triangle.v1(), triangle.v2()};
	std::sort(points.begin(),points.end(),
		[](CanvasPoint p1, CanvasPoint p2) {
		return (p1.y < p2.y);
	});
	// for each y from p1.y to p2.y, we want to print a line
	// the line goes from proportionalPoint(yVal, p1, p2) to proportionalPoint(yVal, p1, p3)
	// proportionalPoint(Y, p1, p2) returns a point(X,Y) on a line

	CanvasPoint newPoint = pointOnLineY(points[1].y, points[0], points[2]);

	top.v0() = points[0];
	top.v1() = points[1];
	top.v2() = newPoint;

	bottom.v0() = points[1];
	bottom.v1() = newPoint;
	bottom.v2() = points[2];

	return {top, bottom};
}

int getHeight(CanvasTriangle triangle) {
	return std::max({triangle.v0().y, triangle.v1().y, triangle.v2().y}) - std::min({triangle.v0().y, triangle.v1().y, triangle.v2().y});
}

void drawFilledTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour) {
	std::vector<CanvasTriangle> triangleSegments = splitTriangle(triangle);
	CanvasTriangle topTriangle = triangleSegments[0];
	int height = getHeight(topTriangle);
	// std::cout << height << std::endl;
	// std::cout << triangleSegment.v0() << std::endl;
	for (int i = 0; i < height; i++) {
		int yVal = topTriangle.v0().y + i;
		CanvasPoint p1 = pointOnLineY(yVal, topTriangle.v0(), topTriangle.v1());
		CanvasPoint p2 = pointOnLineY(yVal, topTriangle.v0(), topTriangle.v2());
		line(window,p1,p2,colour);
	}

	CanvasTriangle bottomTriangle = triangleSegments[1];
	height = getHeight(bottomTriangle);
	for (int i = 0; i < height; i++) {
		int yVal = bottomTriangle.v0().y + i;
		CanvasPoint p1 = pointOnLineY(yVal, bottomTriangle.v2(), bottomTriangle.v1());
		CanvasPoint p2 = pointOnLineY(yVal, bottomTriangle.v2(), bottomTriangle.v0());
		line(window,p1,p2,colour);
	}

	Colour white = Colour(255,255,255);
	drawStrokedTriangle(window, triangle, white);
}

void drawRandomTriangle(DrawingWindow &window, int type = STROKED) {
	Colour randColour = randomColour();
		CanvasTriangle randTriangle = randomTriangle();
	if (type == STROKED) {
		drawStrokedTriangle(window, randTriangle, randColour);
	}else {
		drawFilledTriangle(window, randTriangle, randColour);
	}
}

std::vector<std::vector<uint32_t>> loadTexture(std::string filename) {
	TextureMap textureMap = TextureMap(filename);
	int tMapHeight = textureMap.height;
	int tMapWidth = textureMap.width;
	std::vector<std::vector<uint32_t>> texture;
	texture.resize(tMapHeight);
	for (int i = 0; i < tMapHeight; i++) {
		texture[i].resize(tMapWidth);
	}
	for (int i = 0; i < tMapHeight; i++) {
		for (int j = 0; j <	tMapWidth; j++) {
			texture[i][j] = textureMap.pixels[j + i*tMapWidth];
		}
	}
	return texture;
}

void drawTexturedTriangle(DrawingWindow &window, CanvasTriangle canvasTriangle, std::vector<std::vector<uint32_t>> texture) {
	// input:
	// canvas triangle
	// texture triangle
	// METHODOLOGY:
	// split canvas triangle into two
	// go row by row
	// for each row, calculate each point that corresponds to the texture
	CanvasTriangle topTriangle, bottomTriangle;
	topTriangle = splitTriangle(canvasTriangle)[0];
	bottomTriangle = splitTriangle(canvasTriangle)[1];

	for (int y = topTriangle.v0().y; y < topTriangle.v1().y; y++) {
		CanvasPoint p1 = pointOnLineY(y, topTriangle.v0(), topTriangle.v1());
		CanvasPoint p2 = pointOnLineY(y, topTriangle.v0(), topTriangle.v2());
		texturedLine(window, p1, p2, texture);
	}

	for (int y = bottomTriangle.v0().y; y < bottomTriangle.v2().y; y++) {
		CanvasPoint p1 = pointOnLineY(y, bottomTriangle.v2(), bottomTriangle.v1());
		CanvasPoint p2 = pointOnLineY(y, bottomTriangle.v2(), bottomTriangle.v0());

		texturedLine(window, p1, p2, texture);
	}
	// draw white stroked triangle as outline
	drawStrokedTriangle(window, canvasTriangle,Colour(255,255,255));
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
		else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
		else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
		else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
		else if (event.key.keysym.sym == SDLK_u) drawRandomTriangle(window);
		else if (event.key.keysym.sym == SDLK_f) drawRandomTriangle(window, FILLED);
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}

int main(int argc, char *argv[]) {
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;
	std::vector<std::vector<uint32_t>> texture = loadTexture("texture.ppm");

	CanvasPoint v0 = CanvasPoint(160,10);
	CanvasPoint v1 = CanvasPoint(300,230);
	CanvasPoint v2 = CanvasPoint(10,150);
	v0.texturePoint = TexturePoint(195,5);
	v1.texturePoint = TexturePoint(395,380);
	v2.texturePoint = TexturePoint(65,330);

	CanvasTriangle canvasTriangle = CanvasTriangle(v0,v1,v2);

	drawTexturedTriangle(window, canvasTriangle, texture);
	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}
