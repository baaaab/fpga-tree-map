#include "CResourceUtilisation.h"

CResourceUtilisation::CResourceUtilisation() :
	_registers(0),
	_luts(0),
	_slices(0),
	_rams(0),
	_dsps(0)
{

}

CResourceUtilisation::~CResourceUtilisation()
{

}

uint32_t& CResourceUtilisation::getDsps()
{
	return _dsps;
}

uint32_t& CResourceUtilisation::getLuts()
{
	return _luts;
}
uint32_t& CResourceUtilisation::getRams()
{
	return _rams;
}

uint32_t& CResourceUtilisation::getRegisters()
{
	return _registers;
}

uint32_t& CResourceUtilisation::getSlices()
{
	return _slices;
}

uint32_t CResourceUtilisation::getDsps() const
{
	return _dsps;
}

uint32_t CResourceUtilisation::getLuts() const
{
	return _luts;
}
uint32_t CResourceUtilisation::getRams() const
{
	return _rams;
}

uint32_t CResourceUtilisation::getRegisters() const
{
	return _registers;
}

uint32_t CResourceUtilisation::getSlices() const
{
	return _slices;
}

