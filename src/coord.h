#include "stringstuff.h"
#include <functional>
#include <cmath>

class Coord {
public: 
	typedef std::function<void(const Coord&)> Action;
	typedef std::function<bool(const Coord&)> Func;

	short row, col;

	Coord() : col(0), row(0) {}
	Coord(int col, int row) : col((short)col), row((short)row) {}
	Coord(const Coord& o) : col(o.col), row(o.row) {}

	int GetX() const { return col; }
	int GetY() const { return row; }

	static Coord Zero;// = new Coord(0, 0);
	static Coord One;// = new Coord(1, 1);
	static Coord Two;// = new Coord(2, 2);
	static Coord Up;// = new Coord(0, -1);
	static Coord Left;// = new Coord(-1, 0);
	static Coord Down;// = new Coord(0, 1);
	static Coord Right;// = new Coord(1, 0);

	std::string ToString() const { return string_format("%f,%f", col, row); }
	int GetHashCode() const { return row * 0x00010000 + col; }
	bool Equals(const Coord& c) const { return row == c.row && col == c.col; }

	bool operator ==(const Coord& o) const { return Equals(o); }
	bool operator !=(const Coord& o) const { return !Equals(o); }
	Coord operator +(const Coord& o) const { return Coord(col + o.col, row + o.row); }
	Coord operator -(const Coord& o) const { return Coord(col - o.col, row - o.row); }
	Coord& operator -=(const Coord& o) { col -= o.col; row -= o.row; return *this; }
	Coord& operator +=(const Coord& o) { col += o.col; row += o.row; return *this; }
	Coord operator -() const { return Coord(-col, -row); }
	Coord& operator=(const Coord& o) {
		if (this == &o) { return *this; }
		col = o.col; row = o.row;
		return *this;
	}

	Coord& Scale(const Coord& scale) { col *= scale.col; row *= scale.row; return *this; }
	Coord& InverseScale(const Coord& scale) { col /= scale.col; row /= scale.row; return *this; }

	/// <param name="min">inclusive starting point</param>
	/// <param name="max">exclusive limit</param>
	/// <returns>if this is within the given range</returns>
	bool IsWithin(const Coord& min, const Coord& max) const {
		return row >= min.row && row < max.row && col >= min.col && col < max.col;
	}

	/// <param name="max">exclusive limit</param>
	/// <returns>IsWithin(<see cref="Coord.Zero"/>, max)</returns>
	bool IsWithin(const Coord& max) const { return IsWithin(Zero, max); }

	void Clamp(const Coord& min, const Coord& max) {
		col = (col < min.col) ? min.col : (col > max.col) ? max.col : col;
		row = (row < min.row) ? min.row : (row > max.row) ? max.row : row;
	}

	//public static Coord SizeOf(Array map) {
	//	return new Coord { row = (short)map.GetLength(0), col = (short)map.GetLength(1) };
	//}

	static void ForEach(const Coord& min, const Coord& max, Action& action) {
		Coord cursor = min;
		for(cursor.row = min.row; cursor.row < max.row; ++cursor.row) {
			for(cursor.col = min.col; cursor.col < max.col; ++cursor.col) {
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
		for(cursor.row = min.row; cursor.row < max.row; ++cursor.row) {
			for(cursor.col = min.col; cursor.col < max.col; ++cursor.col) {
				if(action(cursor)) { return true; }
			}
		}
		return false;
	}

	bool ForEach(Func action) { return ForEach(Zero, *this, action); }

	static void ForEachInclusive(Coord start, Coord end, Action action) {
		bool colIncrease = start.col < end.col, rowIncrease = start.row < end.row;
		Coord cursor = start;
		do {
			cursor.col = start.col;
			do {
				action(cursor);
				if (cursor.col == end.col || (colIncrease ? cursor.col > end.col : cursor.col < end.col)) { break; }
				if (colIncrease) { ++cursor.col; } else { --cursor.col; }
			} while (true);
			if (cursor.row == end.row || (rowIncrease ? cursor.row > end.row : cursor.row < end.row)) { break; }
			if (rowIncrease) { ++cursor.row; } else { --cursor.row; }
		} while (true);
	}

	static int ManhattanDistance(Coord a, Coord b) {
		Coord delta = b - a;
		return abs(delta.col) + abs(delta.row);
	}

	//void SetCursorPosition() {
	//	return Console.SetCursorPosition(col, row);
	//}
	//static Coord GetCursorPosition() {
	//	return Coord(Console.CursorLeft, Console.CursorTop);
	//}
};

//public static class CoordExtension {
//	public static TYPE At<TYPE>(this TYPE[,] matrix, Coord coord) {
//		return matrix[coord.row, coord.col];
//	}
//	public static void SetAt<TYPE>(this TYPE[,] matrix, Coord coord, TYPE value) {
//		matrix[coord.row, coord.col] = value;
//	}
//}
