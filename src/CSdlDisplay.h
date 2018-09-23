#ifndef CSDLDISPLAY_H_
#define CSDLDISPLAY_H_

#include <SDL/SDL.h>
#include <thread>
#include <mutex>

class CRect;
class CTreeMap;
class CFpgaItem;

class CSdlDisplay
{
public:
	CSdlDisplay();
	virtual ~CSdlDisplay();

	uint32_t    getWidth             () const;
	uint32_t    getHeight            () const;

	void        drawRectangle        (int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t pixel);
	void        setPixel             (int32_t x, int32_t y, uint32_t pixel);
	void        drawStraightLine     (int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t pixel);
	void        drawLine             (int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t pixel);
	uint32_t    setColour            (uint8_t r, uint8_t g, uint8_t b);

	void        fillSolidRect        (const CRect& rect, uint32_t colour);
	void        drawRectEdge         (const CRect& rect, uint32_t colour);

	void        swapBuffers          ();

	uint32_t*   getScreen            () const;
	void        run                  (CTreeMap* treemap, CFpgaItem* rootItem);


private:

	SDL_Surface* _screen;
	CTreeMap* _treeMap;
	CFpgaItem* _rootItem;

	SDL_Event _event;
	uint32_t _windowWidth;
	uint32_t _windowHeight;

};

#endif /* CDISPLAY_H_ */
