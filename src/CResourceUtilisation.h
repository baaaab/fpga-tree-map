#ifndef SRC_CRESOURCEUTILISATION_H_
#define SRC_CRESOURCEUTILISATION_H_

#include <cstdint>

class CResourceUtilisation
{
public:
	CResourceUtilisation();
	~CResourceUtilisation();

	uint32_t& getDsps();
	uint32_t& getLuts();
	uint32_t& getRams();
	uint32_t& getRegisters();
	uint32_t& getSlices();

	uint32_t getDsps() const;
	uint32_t getLuts() const;
	uint32_t getRams() const;
	uint32_t getRegisters() const;
	uint32_t getSlices() const;

private:
	uint32_t _registers;
	uint32_t _luts;
	uint32_t _slices;
	uint32_t _rams;
	uint32_t _dsps;

};

#endif /* SRC_CRESOURCEUTILISATION_H_ */
