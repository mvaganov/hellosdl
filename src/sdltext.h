#include <SDL.h>
#include "sdleventprocessor.h"
#include "rect.h"
#include <string>
#include "vyengine.h"
#include "vyobjectcommonbase.h"

// TODO implement scrolling function that moves the _srcRect
// TODO test me
class SdlText : public VyObjectCommonBase, public VyDrawable {
public:
	SDL_Texture* SdlTexture;
	Rect _srcRect;
	Rect _destRect;

	SdlText(std::string text) : SdlText(text, "", -1) { }

	SdlText(std::string text, std::string font, int size) : VyObjectCommonBase(text), SdlTexture(NULL), _srcRect(), _destRect() {
		SetText(text, font, size);
		// TODO make a smarter way to register, so that objects that are contained are removed from the engine list, and haandled as child objects
		VyEngine::GetInstance()->RegisterDrawable(this); 
	}

	~SdlText() {
		VyEngine* engine = VyEngine::GetInstance();
		if (SdlTexture != NULL) {
			engine->ReleaseSdlTexture(SdlTexture);
			SdlTexture = NULL;
		}
		VyEngine::GetInstance()->UnregisterDrawable(this);
	}

	virtual VyEventProcessor* AsEventProcessor() { return nullptr; }
	virtual VyDrawable* AsDrawable() { return this; }
	virtual SdlUpdatable* AsUpdatable() { return nullptr; }

	const std::string& GetName() const { return VyObjectCommonBase::GetName(); }
	const std::string& GetText() const { return GetName(); }

	Rect& DestRect() { return _destRect; }
	Rect& SrcRect() { return _srcRect; }

	void SetText(std::string text, std::string font, int fontSize) {
		VyEngine* engine = VyEngine::GetInstance();
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
		SetName(text);
		if (GetText().length() == 0) {
			SdlTexture = NULL;
			_destRect.SetSize(0, 0);
			return;
		}
		VyEngine::ErrorCode err = engine->CreateText(GetText(), SdlTexture);
		engine->FailFast();
		Coord size = engine->GetTextureSize(SdlTexture);
		_destRect.SetSize(size);
		_srcRect.SetSize(size);
	}

	virtual void Draw(SDL_Renderer* g) {
		SDL_RenderCopy(g, SdlTexture, &_srcRect, &_destRect);
	}
};
