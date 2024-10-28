#include "coord.h"

class Rect {
	Coord min, max;

	Rect(const Coord& min, const Coord& max) : min(min), max(max) { }
	Rect(const Rect& o) : Rect(o.min, o.max) {}
	Rect(int x, int y, int width, int height) : min(x, y), max(x + width, y + height) { }

	int GetX() const { return min.col; }
	void SetX(int value) { min.col = (short)value; }
	int GetY() const { return min.row; }
	void SetY(int value) { min.row = (short)value; }
	int GetWidth() const { return max.col - min.col; }
	void SetWidth(int value) { max.col = (short)(min.col + value); }
	int GetHeight() const { return max.row - min.row; }
	void setHeight(int value) { max.row = (short)(min.row + value); }

	int GetMinY() const { return min.row; }
	void SetMinY(int value) { min.row = (short)value; }
	int GetMinX() const { return min.col; }
	void SetMinX(int value) { min.col = (short)value; }
	int GetMaxY() const { return max.row; }
	void SetMaxY(int value) { max.row = (short)value; }
	int GetMaxX() const { return max.col; }
	void SetMaxX(int value) { max.col = (short)value; }

	Coord GetPosition() const { return min; }

	Coord Size() const { return max - min; }

	static bool GetRectIntersect(const Coord& aMin, const Coord& aMax, const Coord& bMin, const Coord& bMax,
		Coord& oMin, Coord& oMax) {
		oMin = Coord(std::max(aMin.col, bMin.col), std::max(aMin.row, bMin.row));
		oMax = Coord(std::min(aMax.col, bMax.col), std::min(aMax.row, bMax.row));
		return oMin.col < oMax.col && oMin.row < oMax.row;
	}

	Rect Intersect(const Rect& r) const {
		Coord iMin, iMax;
		GetRectIntersect(min, max, r.min, r.max, iMin, iMax);
		return Rect(iMin, iMax);
	}

	static bool TryGetIntersect(const Rect& a, const Rect& b, Rect& o) {
		return GetRectIntersect(a.min, a.max, b.min, b.max, o.min, o.max);
	}

	bool TryGetIntersect(const Rect& r, Rect& intersection) const { TryGetIntersect(*this, r, intersection); }

	void ForEach(Coord::Action locationAction) const { Coord::ForEach(min, max, locationAction); }

	bool ForEach(Coord::Func locationCondition) const { return Coord::ForEach(min, max, locationCondition); }

	bool IsIntersect(Rect other) const { return IsRectIntersect(min, max, other.min, other.max); }

	static Rect Sum(Rect a, Rect b) {
		b.Expand(a);
		return b;
	}

	static bool IsRectIntersect(const Coord& aMin, const Coord& aMax, const Coord& bMin, const Coord& bMax) {
		return aMin.col < bMax.col && bMin.col < aMax.col && aMin.row < bMax.row && bMin.row < aMax.row;
	}

	static bool IsSizeRectIntersect(const Coord& aMin, const Coord& aSize, const Coord& bMin, const Coord& bSize) {
		return IsRectIntersect(aMin, aMin + aSize, bMin, bMin + bSize);
	}

	static bool GetSizeRectIntersect(const Coord& aMin, const Coord& aSize, const Coord& bMin, const Coord& bSize,
		Coord& oMin, Coord& oSize) {
		bool result = GetRectIntersect(aMin, aMin + aSize, bMin, bMin + bSize, oMin, oSize);
		oSize -= oMin;
		return result;
	}

	/// <param name="nMin">needle min corner</param>
	/// <param name="nMax">needle max corner</param>
	/// <param name="hMin">haystack min corner</param>
	/// <param name="hMax">haystack max corner</param>
	/// <returns></returns>
	static bool IsRectContained(const Coord& nMin, const Coord& nMax, const Coord& hMin, const Coord& hMax) {
		return nMin.col >= hMin.col && hMax.col >= nMax.col && nMin.row >= hMin.row && hMax.row >= nMax.row;
	}

	static bool IsSizeRectContained(const Coord& nMin, const Coord& nSize, const Coord& hMin, const Coord& hSize) {
		return IsRectContained(nMin, nMin + nSize, hMin, hMin + hSize);
	}

	static bool ExpandRectangle(const Coord& pMin, const Coord& pMax, Coord& min, Coord& max) {
		bool change = false;
		if (pMin.col < min.col) { min.col = pMin.col; change = true; }
		if (pMin.row < min.row) { min.row = pMin.row; change = true; }
		if (pMax.col > max.col) { max.col = pMax.col; change = true; }
		if (pMax.row > max.row) { max.row = pMax.row; change = true; }
		return change;
	}

	bool Expand(const Rect& p) { return ExpandRectangle(p.min, p.max, min, max); }

	bool Expand(const Coord& p) { return ExpandRectangle(p, p, min, max); }
};
