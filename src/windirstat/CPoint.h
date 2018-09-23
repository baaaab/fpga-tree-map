#ifndef SRC_WINDIRSTAT_CPOINT_H_
#define SRC_WINDIRSTAT_CPOINT_H_


#include <inttypes.h>

class CPoint
{
public:
	CPoint(uint32_t nx, uint32_t ny);
	~CPoint();

	uint32_t x;
	uint32_t y;

};

#endif /* SRC_WINDIRSTAT_CPOINT_H_ */
