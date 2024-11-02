#pragma once
#include <SDL.h>
#include <stdio.h>

inline void SDL_SetRenderDrawColor(SDL_Renderer* renderer, long rgba) {
	Uint8* c = (Uint8*)&rgba;
	SDL_SetRenderDrawColor(renderer, c[0], c[1], c[2], c[3]);
}

inline void SDL_GetRenderDrawColor(SDL_Renderer* renderer, long* rgba) {
	Uint8 c[4];
	SDL_GetRenderDrawColor(renderer, &c[0], &c[1], &c[2], &c[3]);
	*rgba = *((long*)c);
}

/// <param name="renderer"></param>
/// <param name="cx">center X</param>
/// <param name="cy">center Y</param>
/// <param name="radius"></param>
inline void SDL_DrawCircle(SDL_Renderer* renderer, int cx, int cy, float radius) {
	const int diameter = (int)(radius * 2);
	int x = (int)(radius - 1), y = 0, tx = 1, ty = 1, error = (tx - diameter), previousY = -1, currentY, left, right;
	while (x >= y) {
		currentY = cy - y; left = cx - x; right = cx + x;
		SDL_RenderDrawPoint(renderer, left, currentY);
		SDL_RenderDrawPoint(renderer, right, currentY);
		currentY = cy + y;
		SDL_RenderDrawPoint(renderer, left, currentY);
		SDL_RenderDrawPoint(renderer, right, currentY);

		currentY = cy - x;
		previousY = currentY; left = cx - y; right = cx + y;
		SDL_RenderDrawPoint(renderer, left, currentY);
		SDL_RenderDrawPoint(renderer, right, currentY);
		currentY = cy + x;
		SDL_RenderDrawPoint(renderer, left, currentY);
		SDL_RenderDrawPoint(renderer, right, currentY);
		if (error <= 0) { ++y; error += ty; ty += 2; }
		if (error > 0) { --x; tx += 2; error += (tx - diameter); }
	}
}

extern short* CIRCLE_VALUES;
extern short CIRCLE_VALUECOUNT;
extern short CIRCLE_VALUEALLOC;

inline void CalculateCircleData(float cx, float cy, float radius) {
	bool allocated = false;
 	if (CIRCLE_VALUES == NULL || CIRCLE_VALUEALLOC < radius * 1.5f) {
		if (CIRCLE_VALUES != NULL) {
			delete[] CIRCLE_VALUES;
		}
		CIRCLE_VALUEALLOC = (int)(radius * 1.5f + 1);
		CIRCLE_VALUES = new short[CIRCLE_VALUEALLOC];
		allocated = true;
	}
	const int diameter = (int)(radius * 2);
	int x = (int)(radius - 1), y = 0, tx = 1, ty = 1, error = (tx - diameter);
	int index = 0;
	CIRCLE_VALUECOUNT = 0;
	while (x >= y) {
		CIRCLE_VALUES[index + 0] = (short)x;
		CIRCLE_VALUES[index + 1] = (short)y;
		index += 2;
		if (error <= 0) { ++y; error += ty; ty += 2; }
		if (error > 0)  { --x; tx += 2; error += (tx - diameter); }
		CIRCLE_VALUECOUNT += 2;
	}
	if (allocated) {
		printf("---- %d\n", CIRCLE_VALUECOUNT);
		for (int i = 0; i < CIRCLE_VALUECOUNT; i += 2) {
			printf("%d: %d %d\n", i, CIRCLE_VALUES[i + 0], CIRCLE_VALUES[i + 1]);
		}
	}
}

/// <param name="renderer"></param>
/// <param name="cx">center X</param>
/// <param name="cy">center Y</param>
/// <param name="radius"></param>
inline void SDL_FillCircle(SDL_Renderer* renderer, float cx, float cy, float radius) {

	CalculateCircleData(cx, cy, radius);
	int x, y;
	int cursor, nextCursor, minPoint, maxPoint;
	// fill majority horizontal band in the center
	for (int index = 0; index < CIRCLE_VALUECOUNT; index += 2) {
		x = CIRCLE_VALUES[index + 0];
		y = CIRCLE_VALUES[index + 1];
		minPoint = (int)(cx - x); maxPoint = (int)(cx + x);
		cursor = (int)(cy - y);
		SDL_RenderDrawLine(renderer, minPoint, cursor, maxPoint, cursor);
		nextCursor = (int)(cy + y);
		if (cursor != nextCursor) {
			SDL_RenderDrawLine(renderer, minPoint, nextCursor, maxPoint, nextCursor);
		}
	}

	// fill top section
	maxPoint = (int)(cy - CIRCLE_VALUES[CIRCLE_VALUECOUNT - 1] - 1);
	for (int index = 0; index < CIRCLE_VALUECOUNT; index += 2) {
		x = CIRCLE_VALUES[index + 0];
		y = CIRCLE_VALUES[index + 1];
		minPoint = (int)(cy - x);
		cursor = (int)(cx - y);
		SDL_RenderDrawLine(renderer, cursor, minPoint, cursor, maxPoint);
		nextCursor = (int)(cx + y);
		if (cursor != nextCursor) {
			SDL_RenderDrawLine(renderer, nextCursor, minPoint, nextCursor, maxPoint);
		}
	}

	// fill top section
	minPoint = (int)(cy + CIRCLE_VALUES[CIRCLE_VALUECOUNT - 1] + 1);
	for (int index = 0; index < CIRCLE_VALUECOUNT; index += 2) {
		x = CIRCLE_VALUES[index + 0];
		y = CIRCLE_VALUES[index + 1];
		maxPoint = (int)(cy + x);
		cursor = (int)(cx - y);
		SDL_RenderDrawLine(renderer, cursor, minPoint, cursor, maxPoint);
		nextCursor = (int)(cx + y);
		if (cursor != nextCursor) {
			SDL_RenderDrawLine(renderer, nextCursor, minPoint, nextCursor, maxPoint);
		}
	}
}
