#pragma once
#include "stringstuff.h"
#include <functional>
#include <cmath>
#include <SDL.h>

class Coord : public SDL_Point {
public: 
	typedef std::function<void(const Coord&)> Action;
	typedef std::function<bool(const Coord&)> Func;

	Coord() : SDL_Point({0,0}) {}
	Coord(int x, int y) : SDL_Point({ x, y }) {}
	Coord(const Coord& o) : SDL_Point({ o.x, o.y }) {}

	int GetX() const { return x; }
	int GetY() const { return y; }

	static Coord Zero;// = new Coord(0, 0);
	static Coord One;// = new Coord(1, 1);
	static Coord Two;// = new Coord(2, 2);
	static Coord Up;// = new Coord(0, -1);
	static Coord Left;// = new Coord(-1, 0);
	static Coord Down;// = new Coord(0, 1);
	static Coord Right;// = new Coord(1, 0);

	std::string ToString() const { return string_format("%f,%f", x, y); }
	int GetHashCode() const { return y * 0x00010000 + x; }
	bool Equals(const Coord& c) const { return y == c.y && x == c.x; }

	bool operator ==(const Coord& o) const { return Equals(o); }
	bool operator !=(const Coord& o) const { return !Equals(o); }
	Coord operator +(const Coord& o) const { return Coord(x + o.x, y + o.y); }
	Coord operator -(const Coord& o) const { return Coord(x - o.x, y - o.y); }
	Coord& operator -=(const Coord& o) { x -= o.x; y -= o.y; return *this; }
	Coord& operator +=(const Coord& o) { x += o.x; y += o.y; return *this; }
	Coord operator -() const { return Coord(-x, -y); }
	Coord& operator=(const Coord& o) {
		if (this == &o) { return *this; }
		x = o.x; y = o.y;
		return *this;
	}

	Coord& Scale(const Coord& scale) { x *= scale.x; y *= scale.y; return *this; }
	Coord& InverseScale(const Coord& scale) { x /= scale.x; y /= scale.y; return *this; }

	/// <param name="min">inclusive starting point</param>
	/// <param name="max">exclusive limit</param>
	/// <returns>if this is within the given range</returns>
	bool IsWithin(const Coord& min, const Coord& max) const {
		return y >= min.y && y < max.y && x >= min.x && x < max.x;
	}

	/// <param name="max">exclusive limit</param>
	/// <returns>IsWithin(<see cref="Coord.Zero"/>, max)</returns>
	bool IsWithin(const Coord& max) const { return IsWithin(Zero, max); }

	void Clamp(const Coord& min, const Coord& max) {
		x = (x < min.x) ? min.x : (x > max.x) ? max.x : x;
		y = (y < min.y) ? min.y : (y > max.y) ? max.y : y;
	}

	inline float MagnitudeSq() const { return (float)(x * x + y * y); }
	float Magnitude() const { return sqrt(MagnitudeSq()); }

	//public static Coord SizeOf(Array map) {
	//	return new Coord { y = (short)map.GetLength(0), x = (short)map.GetLength(1) };
	//}

	static void ForEach(const Coord& min, const Coord& max, Action& action) {
		Coord cursor = min;
		for(cursor.y = min.y; cursor.y < max.y; ++cursor.y) {
			for(cursor.x = min.x; cursor.x < max.x; ++cursor.x) {
				action(cursor);
			}
		}
	}

	void ForEach(Action action) { Coord::ForEach(Zero, *this, action); }

	/// <summary>
	/// stops iterating as soon as action returns true
	/// </summary>
	/// <param name="action">runs till the first return true</param>
	/// <returns>true if action returned true even once</returns>
	static bool ForEach(Coord min, Coord max, Func action) {
		Coord cursor = min;
		for(cursor.y = min.y; cursor.y < max.y; ++cursor.y) {
			for(cursor.x = min.x; cursor.x < max.x; ++cursor.x) {
				if(action(cursor)) { return true; }
			}
		}
		return false;
	}

	bool ForEach(Func action) { return ForEach(Zero, *this, action); }

	static void ForEachInclusive(Coord start, Coord end, Action action) {
		bool xIncrease = start.x < end.x, yIncrease = start.y < end.y;
		Coord cursor = start;
		do {
			cursor.x = start.x;
			do {
				action(cursor);
				if (cursor.x == end.x || (xIncrease ? cursor.x > end.x : cursor.x < end.x)) { break; }
				if (xIncrease) { ++cursor.x; } else { --cursor.x; }
			} while (true);
			if (cursor.y == end.y || (yIncrease ? cursor.y > end.y : cursor.y < end.y)) { break; }
			if (yIncrease) { ++cursor.y; } else { --cursor.y; }
		} while (true);
	}

	static int ManhattanDistance(Coord a, Coord b) {
		Coord delta = b - a;
		return abs(delta.x) + abs(delta.y);
	}

	static float Distance(Coord a, Coord b) {
		Coord delta = b - a;
		return (float)sqrt(delta.x * delta.x + delta.y * delta.y);
	}

	void Set(int x, int y) {
		this->x = x; this->y = y;
	}

	//void SetCursorPosition() {
	//	return Console.SetCursorPosition(x, y);
	//}
	//static Coord GetCursorPosition() {
	//	return Coord(Console.CursorLeft, Console.CursorTop);
	//}
};

//public static class CoordExtension {
//	public static TYPE At<TYPE>(this TYPE[,] matrix, Coord coord) {
//		return matrix[coord.y, coord.x];
//	}
//	public static void SetAt<TYPE>(this TYPE[,] matrix, Coord coord, TYPE value) {
//		matrix[coord.y, coord.x] = value;
//	}
//}
