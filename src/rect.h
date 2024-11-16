#pragma once
#include "coord.h"
#include "SDL.h"

class Rect : public SDL_Rect {
public:
	enum class Dir { Up = 0, Left = 1, Down = 2, Right = 3, Count = 4 };
	static int DirColor[];

	Rect(const Coord& min, const Coord& size) : Rect(min.x, min.y, size.x, size.y) { }
	Rect() : Rect(0, 0, 0, 0) {}
	Rect(const Rect& o) : Rect(o.x, o.y, o.w, o.h) {}
	Rect(int x, int y, int width, int height) {
		this->x = x; this->y = y; this->w = width; this->h = height;
	}
	Rect(const SDL_Rect& rect) { *this = *((Rect*)&rect); }

	static Rect FromMinMax(const Coord& min, const Coord& max) {
		return Rect(min, max - min);
	}

	int GetX() const { return x; }
	void SetX(int value) { x = value; }
	int GetY() const { return y; }
	void SetY(int value) { y = value; }
	int GetWidth() const { return w; }
	void SetWidth(int value) { w = value; }
	int GetHeight() const { return h; }
	void SetHeight(int value) { h = value; }

	int GetMinY() const { return y; }
	void SetMinY(int value) { y = value; }
	int GetMinX() const { return x; }
	void SetMinX(int value) { x = value; }
	int GetMaxX() const { return x + w; }
	void SetMaxX(int value) { w = value - x; }
	int GetMaxY() const { return y + h; }
	void SetMaxY(int value) { h = value - y; }

	Coord GetMin() const { return Coord(x,y); }
	Coord GetMax() const { return Coord(x+w, y+h); }
	Coord GetCenter() const { return Coord(x + w / 2, y + h / 2); }
	void SetMin(const Coord& value) { Coord max = GetMax(); Min() = value; SetMax(max); }
	void SetMax(const Coord& value) { x = value.x - x; y = value.y - y; }
	void SetMinMax(Coord min, Coord max) {
		x = min.x;
		y = min.y;
		w = max.x - min.x;
		h = max.y - min.y;
	}
	Coord GetPosition() const { return Coord(x,y); }
	Coord GetSize() const { return Coord(w,h); }

	Coord& Min() { return *((Coord*)&x); }
	Coord& Size() { return *((Coord*)&w); }

	void SetPosition(int x, int y) { this->x = x; this->y = y; }
	void SetPosition(Coord p) { SetPosition(p.x, p.y); }
	void SetSize(int w, int h) { this->w = w; this->h = h; }
	void SetSize(Coord size) { SetSize(size.x, size.y); }

	bool IsContains(const Coord& coord) {
		return coord.x >= x && coord.y >= y && coord.x < x + w && coord.y < y + h;
	}

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
		Coord max, min;
		bool result = GetRectIntersect(a.GetMin(), a.GetMax(), b.GetMin(), b.GetMax(), min, max);
		o.SetMinMax(min, max);
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
		Coord max, min;
		bool result = ExpandRectangle(p.GetMin(), p.GetMax(), min, max);
		SetMinMax(min, max);
		return result;
	}

	bool Expand(const Coord& p) {
		Coord min, max;
		bool result = ExpandRectangle(p, p, min, max);
		SetMinMax(min, max);
		return result;
	}

	void RenderFillRect(SDL_Renderer* g) const {
		SDL_RenderFillRect(g, (SDL_Rect*)this);
	}

	void RenderDrawRect(SDL_Renderer* g) const {
		SDL_RenderDrawRect(g, (SDL_Rect*)this);
	}
};

class HasRect {
public:
	virtual Rect* GetRect() = 0;
};
