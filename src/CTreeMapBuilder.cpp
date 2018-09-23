#include "CTreeMapBuilder.h"

#include "CFpgaItem.h"

CTreeMapBuilder::CTreeMapBuilder(CFpgaItem* items) :
		_items(items),
		_lastItem(NULL)
{

}

CTreeMapBuilder::~CTreeMapBuilder()
{

}

void CTreeMapBuilder::reset()
{
	_items->clear();
	_lastItem = NULL;
}

void CTreeMapBuilder::addElement(const char* elementId, const CResourceUtilisation& ru)
{
	uint32_t thisElementDepth = getHeirachyDepth(elementId);
	if(_lastItem == NULL)
	{
		if(thisElementDepth != 0)
		{
			fprintf(stderr, "Expected root element but got %s (depth = %u)\n", elementId, thisElementDepth);
			exit(1);
		}
		_items->clear();
		CFpgaItem* item = new CFpgaItem(elementId + thisElementDepth, ru, _items);
		_items->addChild(item);
		_lastItem = item;
	}
	else
	{
		uint32_t lastHeirachyDepth = _lastItem->getDepth();
		CFpgaItem* parent = NULL;

		if(thisElementDepth > lastHeirachyDepth)
		{
			if(thisElementDepth - lastHeirachyDepth == 1)
			{
				parent = _lastItem;
			}
			else
			{
				fprintf(stderr, "We appear to have descended two levels of heirachy (%u -> %u), exiting\n", thisElementDepth, lastHeirachyDepth);
				exit(1);
			}
		}
		else
		{
			parent = _lastItem;
			uint32_t parentDepth = lastHeirachyDepth;
			while(parentDepth >= thisElementDepth)
			{
				parent = parent->getParent();
				parentDepth--;
			}
		}
		CFpgaItem* item = new CFpgaItem(elementId + thisElementDepth, ru, parent);
		parent->addChild(item);
		_lastItem = item;
	}
}

uint32_t CTreeMapBuilder::getHeirachyDepth(const char* elementId)
{
	uint32_t depth = 0;
	while(*elementId == '+')
	{
		depth++;
		elementId++;
	}
	return depth;
}

