#ifndef SRC_CTREEMAPBUILDER_H_
#define SRC_CTREEMAPBUILDER_H_

#include "CResourceUtilisation.h"

class CFpgaItem;

class CTreeMapBuilder
{
public:
	CTreeMapBuilder(CFpgaItem* items);
	~CTreeMapBuilder();

	void reset();

	void addElement(const char* elementId, const CResourceUtilisation& ru);

private:

	CFpgaItem* _items;
	CFpgaItem* _lastItem;

	uint32_t getHeirachyDepth(const char* elementId);

};

#endif /* SRC_CTREEMAPBUILDER_H_ */
