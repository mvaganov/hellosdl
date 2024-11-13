#include <SDL.h>
#include "sdleventprocessor.h"
#include "rect.h"
#include <string>
#include "sdlengine.h"

// TODO implement scrolling function that moves the _srcRect
// TODO test me
class SdlText : public SdlDrawable {
public:
	std::string _text;
	SDL_Texture* SdlTexture;
	Rect _srcRect;
	Rect _destRect;

	SdlText(std::string text) : SdlText(text, "", -1) { }

	SdlText(std::string text, std::string font, int size) : _text(), SdlTexture(NULL), _srcRect(), _destRect() {
		SetText(text, font, size);
		SdlEngine::GetInstance()->RegisterDrawable(this);
	}

	~SdlText() {
		SdlEngine* engine = SdlEngine::GetInstance();
		if (SdlTexture != NULL) {
			engine->ReleaseSdlTexture(SdlTexture);
			SdlTexture = NULL;
		}
		SdlEngine::GetInstance()->UnregisterDrawable(this);
	}

	const std::string& GetText() const { return _text; }

	Rect& DestRect() { return _destRect; }
	Rect& SrcRect() { return _srcRect; }

	void SetText(std::string text, std::string font, int fontSize) {
		SdlEngine* engine = SdlEngine::GetInstance();
		bool setFont = font != "";
		bool setSize = fontSize > 0;
		if ((setFont && font != engine->GetFontName())
		|| (setSize && fontSize != engine->GetFontSize())) {
			if (!setFont) { font = engine->GetFontName(); }
			if (!setSize) { fontSize = engine->GetFontSize(); }
		}
		if (SdlTexture != NULL) {
			engine->ReleaseSdlTexture(SdlTexture);
		}
		_text = text;
		if (_text.length() == 0) {
			SdlTexture = NULL;
			_destRect.SetSize(0, 0);
			return;
		}
		SdlEngine::ErrorCode err = engine->CreateText(_text, SdlTexture);
		engine->FailFast();
		Coord size = engine->GetTextureSize(SdlTexture);
		_destRect.SetSize(size);
		_srcRect.SetSize(size);
	}

	virtual void Draw(SDL_Renderer* g) {
		SDL_RenderCopy(g, SdlTexture, &_srcRect, &_destRect);
	}
};
