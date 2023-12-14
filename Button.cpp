#include "pch.h"
#include "Button.h"

Button::Button(const glm::vec2 midPos, string_view strTexName, const float scaleFactor)
	:UI{midPos,strTexName,scaleFactor}
{
}

Button::Button()
{
}

Button::~Button()
{
}
