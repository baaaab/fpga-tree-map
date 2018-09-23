#include "CRect.h"

CRect::CRect() :
_left(0),
_top(0),
_right(0),
_bottom(0)
{

}

CRect::CRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height) :
_left(x),
_top(y),
_right(x+width),
_bottom(y+height)
{

}

CRect::CRect(CPoint& topLeft, CPoint& widthHeight) :
_left(topLeft.x),
_top(topLeft.y),
_right(topLeft.x+widthHeight.x),
_bottom(topLeft.y + widthHeight.y)
{

}

CRect::~CRect()
{

}

uint32_t CRect::getHeight() const
{
	return _bottom - _top;
}

uint32_t CRect::getWidth() const
{
	return _right - _left;
}

uint32_t& CRect::getLeft()
{
	return _left;
}

uint32_t& CRect::getRight()
{
	return _right;
}

uint32_t& CRect::getTop()
{
	return _top;
}

uint32_t& CRect::getBottom()
{
	return _bottom;
}

uint32_t CRect::getLeft() const
{
	return _left;
}

uint32_t CRect::getRight() const
{
	return _right;
}

uint32_t CRect::getTop() const
{
	return _top;
}

uint32_t CRect::getBottom() const
{
	return _bottom;
}

CPoint CRect::size() const
{
	return CPoint(getWidth(), getHeight());
}

bool CRect::ptInRect(const CPoint p) const
{
	return _left <= p.x && _right >= p.x && _top <= p.y && _bottom >= p.y;
}
