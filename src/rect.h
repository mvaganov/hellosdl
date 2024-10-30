#pragma once
#include "coord.h"
#include "SDL.h"

class Rect {
	Coord min, size;

	Rect(const Coord& min, const Coord& size) : min(min), size(size) { }
	Rect(const Rect& o) : Rect(o.min, o.size) {}
	Rect(int x, int y, int width, int height) : min(x, y), size(width, height) { }
	Rect(const SDL_Rect& rect) { *this = *((Rect*)&rect); }

	static Rect FromMinMax(const Coord& min, const Coord& max) {
		return Rect(min, max - min);
	}

	int GetX() const { return min.x; }
	void SetX(int value) { min.x = value; }
	int GetY() const { return min.y; }
	void SetY(int value) { min.y = value; }
	int GetWidth() const { return size.x; }
	void SetWidth(int value) { size.x = value; }
	int GetHeight() const { return size.y; }
	void SetHeight(int value) { size.y = value; }

	int GetMinY() const { return min.y; }
	void SetMinY(int value) { min.y = value; }
	int GetMinX() const { return min.x; }
	void SetMinX(int value) { min.x = value; }
	int GetMaxX() const { return min.x + size.x; }
	void SetMaxX(int value) { size.x = value - min.x; }
	int GetMaxY() const { return min.y + size.y; }
	void SetMaxY(int value) { size.y = value - min.y; }

	Coord GetMin() const { return min; }
	Coord GetMax() const { return min+size; }
	void SetMin(const Coord& value) { Coord max = GetMax(); min = value; SetMax(max); }
	void SetMax(const Coord& value) { size = value - min; }
	Coord GetPosition() const { return min; }
	Coord GetSize() const { return size; }

	static bool GetRectIntersect(const Coord& aMin, const Coord& aMax, const Coord& bMin, const Coord& bMax,
		Coord& oMin, Coord& oMax) {
		oMin = Coord(std::max(aMin.x, bMin.x), std::max(aMin.y, bMin.y));
		oMax = Coord(std::min(aMax.x, bMax.x), std::min(aMax.y, bMax.y));
		return oMin.x < oMax.x && oMin.y < oMax.y;
	}

	Rect Intersect(const Rect& r) const {
		Coord iMin, iMax;
		GetRectIntersect(GetMin(), GetMax(), r.GetMin(), r.GetMax(), iMin, iMax);
		return Rect::FromMinMax(iMin, iMax);
	}

	static bool TryGetIntersect(const Rect& a, const Rect& b, Rect& o) {
		Coord max;
		bool result = GetRectIntersect(a.GetMin(), a.GetMax(), b.GetMin(), b.GetMax(), o.min, max);
		o.SetMax(max);
		return result;
	}

	bool TryGetIntersect(const Rect& r, Rect& intersection) const {
		TryGetIntersect(*this, r, intersection);
	}

	void ForEach(Coord::Action locationAction) const {
		Coord::ForEach(GetMin(), GetMax(), locationAction);
	}

	bool ForEach(Coord::Func locationCondition) const {
		return Coord::ForEach(GetMin(), GetMax(), locationCondition);
	}

	bool IsIntersect(Rect other) const {
		return IsRectIntersect(GetMin(), GetMax(), other.GetMin(), other.GetMax());
	}

	static Rect Sum(Rect a, Rect b) {
		b.Expand(a);
		return b;
	}

	static bool IsRectIntersect(const Coord& aMin, const Coord& aMax, const Coord& bMin, const Coord& bMax) {
		return aMin.x < bMax.x && bMin.x < aMax.x && aMin.y < bMax.y && bMin.y < aMax.y;
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
		return nMin.x >= hMin.x && hMax.x >= nMax.x && nMin.y >= hMin.y && hMax.y >= nMax.y;
	}

	static bool IsSizeRectContained(const Coord& nMin, const Coord& nSize, const Coord& hMin, const Coord& hSize) {
		return IsRectContained(nMin, nMin + nSize, hMin, hMin + hSize);
	}

	/// <summary>
	/// 
	/// </summary>
	/// <param name="pMin">a rectangle to add to an existing rectangle</param>
	/// <param name="pMax"></param>
	/// <param name="out_min">the existing rectangle</param>
	/// <param name="out_max"></param>
	/// <returns></returns>
	static bool ExpandRectangle(const Coord& pMin, const Coord& pMax, Coord& out_min, Coord& out_max) {
		bool change = false;
		if (pMin.x < out_min.x) { out_min.x = pMin.x; change = true; }
		if (pMin.y < out_min.y) { out_min.y = pMin.y; change = true; }
		if (pMax.x > out_max.x) { out_max.x = pMax.x; change = true; }
		if (pMax.y > out_max.y) { out_max.y = pMax.y; change = true; }
		return change;
	}

	bool Expand(const Rect& p) {
		Coord max;
		bool result = ExpandRectangle(p.GetMin(), p.GetMax(), min, max);
		SetMax(max);
		return result;
	}

	bool Expand(const Coord& p) {
		Coord max;
		bool result = ExpandRectangle(p, p, min, max);
		SetMax(max);
		return result;
	}

	void RenderFillRect(SDL_Renderer* g) const {
		SDL_RenderFillRect(g, (SDL_Rect*)this);
	}

	void RenderFillRect(SDL_Renderer* g) const {
		SDL_RenderDrawRect(g, (SDL_Rect*)this);
	}
};
