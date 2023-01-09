#pragma once
#include <vector>
#include "glm/glm.hpp"

class RandomProvider
{
private:
	int32_t m_RandNumSlices = 6;
	uint32_t m_RandomTracker = 0;
	uint32_t m_RandomLock = 0;
	uint32_t m_PoolSize = 20;
	std::vector<glm::vec3>m_RandomsPool;

	void IncrementRandomTracker();
	void IncrementRandomLock();

public:
	//ctor
	RandomProvider();

	//returns a new random from stored array while avoiding tearing from multithreading
	glm::vec3 GetPsuedoRandomDir();

	// updates randoms one by one while keeping track
	void UpdatePsuedoRandomDir();

};
