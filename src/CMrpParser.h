#ifndef SRC_CMRPPARSER_H_
#define SRC_CMRPPARSER_H_

#include <cstdint>

#include "CResourceUtilisation.h"

class CTreeMapBuilder;
class CFpgaItem;

class CMrpParser
{
public:
	CMrpParser(const char* mapReport);
	virtual ~CMrpParser();

	CFpgaItem* getItems() const;

	bool parse();

private:
	const char* _mapReport;

	CResourceUtilisation _used;
	CResourceUtilisation _total;

	CFpgaItem* _items;
	CTreeMapBuilder* _treeMapBuilder;

	void extractDesignSummaryValues(const char* haystack, const char* needle, uint32_t& used, uint32_t& total);
	void copyStringIgnoringChars(const char* src, char* dst, uint32_t dstSize, const char* charsToIgnore);
	void copyStringKeepingChars(const char* src, char* dst, uint32_t dstSize, const char* charsToKeep);
};

#endif /* SRC_CMRPPARSER_H_ */
