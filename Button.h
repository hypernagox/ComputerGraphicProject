#pragma once
#include "UI.h"

class Button
	:public UI
{
	friend class PannelUI;
protected:
	Button(const glm::vec2& _LT, const glm::vec2& _RB);
public:
	Button();
	~Button();

	void Render()override{ MyPolygon::Render(); }
};

