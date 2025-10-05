#include <CanvasTriangle.h>
#include <CanvasPoint.h>
#include <Colour.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <vector>

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

void triangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour, int type = STROKED) {
	line(window, triangle.v0(), triangle.v1(), colour);
	line(window, triangle.v1(), triangle.v2(), colour);
	line(window, triangle.v2(), triangle.v0(), colour);
}

void randomTriangle (DrawingWindow &window, int type = STROKED) {
	Colour randColour = Colour(rand() % 256, rand() % 256, rand() % 256);
	CanvasPoint v0 = CanvasPoint(rand() % WIDTH, rand() % HEIGHT);
	CanvasPoint v1 = CanvasPoint(rand() % WIDTH, rand() % HEIGHT);
	CanvasPoint v2 = CanvasPoint(rand() % WIDTH, rand() % HEIGHT);
	CanvasTriangle randTriangle = CanvasTriangle(v0, v1, v2);
	triangle(window, randTriangle, randColour);
}



void draw(DrawingWindow &window) {
	// window.clearPixels();
	//	Task 2: Line Drawing
	CanvasPoint centre = CanvasPoint(WIDTH/2, HEIGHT/2);
	CanvasPoint top = CanvasPoint(WIDTH/2, 0);
	CanvasPoint bottom = CanvasPoint(WIDTH/2, HEIGHT-1);
	CanvasPoint topLeft = CanvasPoint(0, 0);
	CanvasPoint topRight = CanvasPoint(WIDTH-1, 0);
	CanvasPoint middleLeftThird = CanvasPoint(WIDTH/3, HEIGHT/2);
	CanvasPoint middleRightThird = CanvasPoint(2*WIDTH/3, HEIGHT/2);

	Colour white = Colour(255,255,255);
	line(window,  topLeft, centre, white);
	line(window,  topRight, centre, white);
	line(window,  top, bottom, white);
	line(window,  middleLeftThird,  middleRightThird, white);
}

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
		else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
		else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
		else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
		else if (event.key.keysym.sym == SDLK_u) randomTriangle(window);
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}

int main(int argc, char *argv[]) {
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;

	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		// draw(window);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}
