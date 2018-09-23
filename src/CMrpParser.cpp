#include "CMrpParser.h"

#include <cstdio>
#include <cstring>

#include "CFpgaItem.h"
#include "CTreeMapBuilder.h"

CMrpParser::CMrpParser(const char* mapReport) :
		_mapReport(mapReport),
		_items(NULL),
		_treeMapBuilder(NULL)
{
	CResourceUtilisation ru;
	_items = new CFpgaItem("root", ru, NULL);
	_treeMapBuilder = new CTreeMapBuilder(_items);
}

CMrpParser::~CMrpParser()
{

}

CFpgaItem* CMrpParser::getItems() const
{
	return _items;
}

bool CMrpParser::parse()
{
	FILE* fh = fopen(_mapReport, "r");
	if (!fh)
	{
		fprintf(stderr, "%s::%s error opening map report: %s\n", __FILE__, __FUNCTION__, _mapReport);
		return false;
	}

	enum class ESection
	{
		NONE, DESIGN_SUMMARY, TABLE_OF_CONTENTS, SECTION1, UTILISATION_BY_HEIRACHY
	};

	ESection section = ESection::NONE;

	char* line = NULL;
	size_t bufferSize = 0;
	int32_t lineLength = 0;

	uint32_t usedDspA1s = 0, usedDspE1s = 0, totalDspA1s = 0, totalDspE1s = 0;
	uint32_t usedRam8s = 0, usedRam16s = 0, usedRam18s = 0, usedRam36s = 0;
	uint32_t totalRam8s = 0, totalRam16s = 0, totalRam18s = 0, totalRam36s = 0;
	bool headerRowSeen = false;

	while ((lineLength = getline(&line, &bufferSize, fh)) != -1)
	{
		switch (section)
		{
			case ESection::NONE:
			{
				if(strcmp(line, "Design Summary\n") == 0)
				{
					section = ESection::DESIGN_SUMMARY;
				}
				break;
			}
			case ESection::DESIGN_SUMMARY:
			{
				extractDesignSummaryValues(line, "  Number of Slice LUTs:  ",      _used.getLuts(),      _total.getLuts());
				extractDesignSummaryValues(line, "  Number of Slice Registers:  ", _used.getRegisters(), _total.getRegisters());
				extractDesignSummaryValues(line, "  Number of occupied Slices:  ", _used.getSlices(),    _total.getSlices());
				extractDesignSummaryValues(line, "  Number of DSP48E1s:  ", usedDspE1s, totalDspE1s);
				extractDesignSummaryValues(line, "  Number of DSP48A1s:  ", usedDspA1s, totalDspA1s);
				extractDesignSummaryValues(line, "  Number of RAMB8BWERs:  ", usedRam8s, totalRam8s);
				extractDesignSummaryValues(line, "  Number of RAMB16BWERs:  ", usedRam16s, totalRam16s);
				extractDesignSummaryValues(line, "  Number of RAMB18E1/FIFO18E1s:  ", usedRam18s, totalRam18s);
				extractDesignSummaryValues(line, "  Number of RAMB36E1/FIFO36E1s:  ", usedRam36s, totalRam36s);

				if(strcmp(line, "Table of Contents\n") == 0)
				{
					section = ESection::TABLE_OF_CONTENTS;
				}
				break;
			}
			case ESection::TABLE_OF_CONTENTS:
			{
				// first time we see this will be in the table of contents
				if(strcmp(line, "Section 13 - Utilization by Hierarchy\n") == 0)
				{
					section = ESection::SECTION1;
				}
				break;
			}
			case ESection::SECTION1:
			{
				// this is the actual section header
				if(strcmp(line, "Section 13 - Utilization by Hierarchy\n") == 0)
				{
					section = ESection::UTILISATION_BY_HEIRACHY;
				}
				break;
			}
			case ESection::UTILISATION_BY_HEIRACHY:
			{
				char* module = strtok(line, "| ");
				strtok(NULL, "|"); // partition
				char* slices = strtok(NULL, "|");
				char* registers = strtok(NULL, "|");
				char* luts = strtok(NULL, "|");
				strtok(NULL, "|"); // lutram
				char* rams = strtok(NULL, "|");
				char* dsps = strtok(NULL, "|");

				//printf("module = '%s' [%p], strstr(module, \"Module\") = %p\n", module, module, strstr(module, "Module"));
				if(!headerRowSeen && strstr(module, "Module") != NULL)
				{
					headerRowSeen = true;
				}
				else if(headerRowSeen && module && slices && registers && luts && rams && dsps)
				{
					printf("%s, %s, %s, %s, %s, %s\n", module, slices, registers, luts, rams, dsps);
					CResourceUtilisation ru;
					ru.getSlices() = strtoul(slices, NULL, 10);
					ru.getRegisters() = strtoul(registers, NULL, 10);
					ru.getLuts() = strtoul(luts, NULL, 10);
					ru.getRams() = strtoul(rams, NULL, 10);
					ru.getDsps() = strtoul(dsps, NULL, 10);

					_treeMapBuilder->addElement(module, ru);
				}
				break;
			}
		}
	}

	_items->print(stdout);

	if(section != ESection::UTILISATION_BY_HEIRACHY)
	{
		fprintf(stderr, "Unable to find \"Section 13 - Utilization by Hierarchy\" in MAP Report. Is this a detailed MAP report?\n");
		exit(1);
	}

	_used.getDsps() = usedDspE1s + usedDspA1s;
	_total.getDsps() = totalDspE1s + totalDspA1s;
	if(totalRam8s != 0)
	{
		_used.getRams() = usedRam8s + 2 * usedRam16s;
		_total.getRams() = totalRam8s;
	}
	else if(totalRam18s != 0)
	{
		_used.getRams() = usedRam18s + 2 * usedRam36s;
		_total.getRams() = totalRam18s;
	}

	printf("Slices      : %6u / %6u (%2.1f%%)\n", _used.getSlices(), _total.getSlices(), 100.0f * _used.getSlices() / (float) (std::max(1U, _total.getSlices())));
	printf("  Luts      : %6u / %6u (%2.1f%%)\n", _used.getLuts(), _total.getLuts(), 100.0f * _used.getLuts() / (float) (std::max(1U, _total.getLuts())));
	printf("  Registers : %6u / %6u (%2.1f%%)\n", _used.getRegisters(), _total.getRegisters(), 100.0f * _used.getRegisters() / (float) (std::max(1U, _total.getRegisters())));
	printf("RAMs        : %6u / %6u (%2.1f%%)\n", _used.getRams(), _total.getRams(), 100.0f * _used.getRams() / (float) (std::max(1U, _total.getRams())));
	printf("DSPs        : %6u / %6u (%2.1f%%)\n", _used.getDsps(), _total.getDsps(), 100.0f * _used.getDsps() / (float) (std::max(1U, _total.getDsps())));

	fclose(fh);
	return true;
}

void CMrpParser::extractDesignSummaryValues(const char* haystack, const char* needle, uint32_t& used, uint32_t& total)
{
	const char* startPtr = NULL;
	if((startPtr = strstr(haystack, needle)) != NULL)
	{
		startPtr += strlen(needle);
		uint32_t pLength = strlen(startPtr) + 1;
		char* pLine = new char[pLength];
		copyStringKeepingChars(startPtr, pLine, pLength, "0123456789 ");
		char* endPtr = NULL;
		used = strtoul(pLine, &endPtr, 10);
		if(endPtr && *endPtr)
		{
			total = strtoul(endPtr, NULL, 10);
		}
		delete[] pLine;
	}
}

void CMrpParser::copyStringIgnoringChars(const char* src, char* dst, uint32_t dstSize, const char* charsToIgnore)
{
	uint32_t numChars = 0;
	while(*src && numChars < dstSize)
	{
		if(strchr(charsToIgnore, *src) == NULL)
		{
			*dst = *src;
			dst++;
			numChars++;
		}
		src++;
	}
	*dst = 0;
}

void CMrpParser::copyStringKeepingChars(const char* src, char* dst, uint32_t dstSize, const char* charsToKeep)
{
	uint32_t numChars = 0;
	while(*src && numChars < dstSize)
	{
		if(strchr(charsToKeep, *src) != NULL)
		{
			*dst = *src;
			dst++;
			numChars++;
		}
		src++;
	}
	*dst = 0;
}

