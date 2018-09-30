#include "CSdlDisplay.h"

#include <stdio.h>
#include <X11/Xlib.h>
#include <mutex>
#include <math.h>
#include <unistd.h>
#include <algorithm>

#include "windirstat/CRect.h"
#include "windirstat/CTreeMap.h"
#include "CFpgaItem.h"

CSdlDisplay::CSdlDisplay() :
		_treeMapImage(NULL),
		_screen(NULL),
		_treeMap(NULL),
		_unusedItem(NULL),
		_selectedItem(NULL),
		_itemToDraw(NULL),
		_treeMapRedrawRequired(false),
		_selectedRedrawRequired(false),
		_windowWidth(900),
		_windowHeight(900)
{
	XInitThreads();

	if (SDL_Init( SDL_INIT_VIDEO) != 0)
	{
		fprintf(stderr, "Could not initialise SDL: %s\n", SDL_GetError());
	}

	if ((_screen = SDL_SetVideoMode(_windowWidth, _windowHeight, 32, SDL_HWSURFACE | SDL_DOUBLEBUF)) == NULL)
	{
		fprintf(stderr, "Could not set SDL video mode: %s\n", SDL_GetError());
		SDL_Quit();
		exit(1);
	}
	SDL_WM_SetCaption("Unusual Object Detector", "Unusual Object Detector");

	_treeMapImage = new uint8_t[_windowHeight * _windowWidth * 4];
}

CSdlDisplay::~CSdlDisplay()
{
	delete[] _treeMapImage;

	SDL_QUIT();
}

uint32_t CSdlDisplay::getWidth() const
{
	return _windowWidth;
}

uint32_t CSdlDisplay::getHeight() const
{
	return _windowHeight;
}

void CSdlDisplay::setPixel(int x, int y, uint32_t pixel)
{
	uint8_t *target_pixel = (uint8_t *) _screen->pixels + y * _screen->pitch + x * 4;
	*(uint32_t *) target_pixel = pixel;
}

uint32_t CSdlDisplay::setColour(uint8_t r, uint8_t g, uint8_t b)
{
	uint32_t pixel = b + (g << 8) + (r << 16);
	return pixel;
}

void CSdlDisplay::fillSolidRect(const CRect& rect, uint32_t colour)
{
	for (uint32_t y = rect.getTop(); y < rect.getBottom(); y++)
	{
		for (uint32_t x = rect.getLeft(); x < rect.getRight(); x++)
		{
			uint8_t *target_pixel = (uint8_t *) _screen->pixels + y * _screen->pitch + x * 4;
			*(uint32_t *) target_pixel = colour;
		}
	}
}

void CSdlDisplay::drawRectEdge(const CRect& rect, uint32_t colour)
{
	drawStraightLine(rect.getLeft(), rect.getTop(), rect.getRight(), rect.getTop(), colour);
	drawStraightLine(rect.getLeft(), rect.getBottom(), rect.getRight(), rect.getBottom(), colour);
	drawStraightLine(rect.getLeft(), rect.getTop(), rect.getLeft(), rect.getBottom(), colour);
	drawStraightLine(rect.getRight(), rect.getTop(), rect.getRight(), rect.getBottom(), colour);
}

void CSdlDisplay::drawStraightLine(int x1, int y1, int x2, int y2, uint32_t pixel)
{
	if (y1 > y2)
	{
		int temp = y2;
		y2 = y1;
		y1 = temp;
	}
	if (x1 > x2)
	{
		int temp = x2;
		x2 = x1;
		x1 = temp;
	}
	if (x1 == x2)
	{
		for (int y = y1; y < y2 + 1; y++)
		{
			setPixel(x1, y, pixel);
		}
	}
	else
	{
		for (int x = x1; x < x2; x++)
		{
			setPixel(x, y1, pixel);
		}
	}
}

void CSdlDisplay::drawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t pixel)
{
	int32_t dx = std::abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
	int32_t dy = std::abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
	int32_t err = (dx > dy ? dx : -dy) / 2, e2;

	for (;;)
	{
		setPixel(x1, y1, pixel);
		if (x1 == x2 && y1 == y2)
			break;
		e2 = err;
		if (e2 > -dx)
		{
			err -= dy;
			x1 += sx;
		}
		if (e2 < dy)
		{
			err += dx;
			y1 += sy;
		}
	}
}

void CSdlDisplay::swapBuffers()
{
	SDL_Flip(_screen);
	SDL_FillRect(_screen, NULL, 0);
}

uint32_t* CSdlDisplay::getScreen() const
{
	return static_cast<uint32_t*>(_screen->pixels);
}

void CSdlDisplay::run(CTreeMap* treemap, CFpgaItem* unusedItem)
{

	_treeMap = treemap;
	_unusedItem = unusedItem;
	_itemToDraw = _unusedItem;
	_selectedItem = _unusedItem;
	CTreeMap::Options options = CTreeMap::GetDefaultOptions();

	_unusedItem->recursivelyCalculateSize();
	_unusedItem->sort();

	_treeMapRedrawRequired = true;

	while (1)
	{
		handleEvents();
		if (_treeMapRedrawRequired)
		{
			_treeMap->DrawTreemap(this, CRect(0, 0, getWidth(), getHeight()), _itemToDraw, &options);
			memcpy(_treeMapImage, _screen->pixels, _windowHeight * _windowWidth * 4);
			_treeMapRedrawRequired = false;
			_selectedRedrawRequired = true;
		}
		else
		{
			if (_selectedRedrawRequired)
			{
				memcpy(_screen->pixels, _treeMapImage, _windowHeight * _windowWidth * 4);
			}
		}
		if (_selectedRedrawRequired && _selectedItem)
		{
			drawRectEdge(_selectedItem->TmiGetRectangle(), 0xffffffff);
			swapBuffers();
			_selectedRedrawRequired = false;
		}
		struct timespec ts;
		ts.tv_sec = 0;
		ts.tv_nsec = 1000000000 / 60;
		nanosleep(&ts, NULL);
	}
}

void CSdlDisplay::handleEvents()
{
	while (SDL_PollEvent(&_event))
	{
		switch (_event.type)
		{
			case SDL_MOUSEMOTION:
			{
				//printf("Mouse moved by %d,%d to (%d,%d)\n", _event.motion.xrel, _event.motion.yrel,	_event.motion.x, _event.motion.y);
				break;
			}
			case SDL_MOUSEBUTTONDOWN:
			{
				//printf("Mouse button %d pressed at (%d,%d)\n", _event.button.button, _event.button.x, _event.button.y);
				if (_treeMap && _itemToDraw)
				{
					CFpgaItem* item = dynamic_cast<CFpgaItem*>(_treeMap->FindItemByPoint(_itemToDraw, CPoint(_event.button.x, _event.button.y)));
					if (item)
					{
						_selectedItem = item;
						_selectedItem->printHeirachy();
						_selectedRedrawRequired = true;
					}
				}
				break;
			}
			case SDL_KEYDOWN:
			{
				switch (_event.key.keysym.sym)
				{
					case SDLK_d:
					{
						CFpgaItem::SetUtilisationMetric(EUtilisationMetric::DSP);
						_unusedItem->recursivelyCalculateSize();
						_unusedItem->sort();
						_treeMapRedrawRequired = true;
						break;
					}
					case SDLK_r:
					{
						CFpgaItem::SetUtilisationMetric(EUtilisationMetric::RAM);
						_unusedItem->recursivelyCalculateSize();
						_unusedItem->sort();
						_treeMapRedrawRequired = true;
						break;
					}
					case SDLK_l:
					{
						CFpgaItem::SetUtilisationMetric(EUtilisationMetric::LUT);
						_unusedItem->recursivelyCalculateSize();
						_unusedItem->sort();
						_treeMapRedrawRequired = true;
						break;
					}
					case SDLK_s:
					{
						CFpgaItem::SetUtilisationMetric(EUtilisationMetric::SLICE);
						_unusedItem->recursivelyCalculateSize();
						_unusedItem->sort();
						_treeMapRedrawRequired = true;
						break;
					}
					case SDLK_f:
					{
						CFpgaItem::SetUtilisationMetric(EUtilisationMetric::REG);
						_unusedItem->recursivelyCalculateSize();
						_unusedItem->sort();
						_treeMapRedrawRequired = true;
						break;
					}
					case SDLK_u:
					{
						if(_itemToDraw == _unusedItem)
						{
							_itemToDraw = _unusedItem->getChild(0);
						}
						else
						{
							_itemToDraw = _unusedItem;
						}
						_treeMapRedrawRequired = true;
						break;
					}
					case SDLK_LEFT:
					{
						// select parent
						if (_selectedItem && _selectedItem->getParent())
						{
							CFpgaItem* item = _selectedItem->getParent();
							if (item)
							{
								_selectedItem = item;
								_selectedItem->printHeirachy();
							}
							_selectedRedrawRequired = true;
						}
						break;
					}
					case SDLK_DOWN:
					{
						// select next sibling, or parent's next child
						if (_selectedItem)
						{
							CFpgaItem* item = _selectedItem->getNextSibling();
							if (item)
							{
								_selectedItem = item;
								_selectedItem->printHeirachy();
							}
							_selectedRedrawRequired = true;
						}
						break;
					}
					case SDLK_UP:
					{
						// select previous sibling, of parent when first sibling
						if (_selectedItem)
						{
							CFpgaItem* item = _selectedItem->getPreviousSibling();
							if (item)
							{
								_selectedItem = item;
								_selectedItem->printHeirachy();
							}
							_selectedRedrawRequired = true;
						}
						break;
					}
					case SDLK_RIGHT:
					{
						// select first child
						if (_selectedItem)
						{
							if (_selectedItem->TmiGetChildrenCount() > 0)
							{
								_selectedItem = _selectedItem->getChild(0);
								_selectedItem->printHeirachy();
								_selectedRedrawRequired = true;
							}
						}
						break;
					}
				}
				break;
			}
			case SDL_QUIT:
			{
				exit(0);
			}
		}
	}
}

