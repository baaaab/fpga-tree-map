#ifndef SRC_WINDIRSTAT_CRECT_H_
#define SRC_WINDIRSTAT_CRECT_H_

#include <inttypes.h>

#include "../windirstat/CPoint.h"

class CRect
{
public:
	CRect();
	CRect(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
	CRect(CPoint& topLeft, CPoint& widthHeight);
	~CRect();

	uint32_t getHeight() const;
	uint32_t getWidth() const;

	uint32_t& getLeft();
	uint32_t& getRight();
	uint32_t& getTop();
	uint32_t& getBottom();

	uint32_t getLeft() const;
	uint32_t getRight() const;
	uint32_t getTop() const;
	uint32_t getBottom() const;

	CPoint size() const;

	bool ptInRect(const CPoint p) const;

private:
	uint32_t _left;
	uint32_t _top;
	uint32_t _right;
	uint32_t _bottom;
};

#endif /* SRC_WINDIRSTAT_CRECT_H_ */
