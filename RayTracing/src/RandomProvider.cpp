#include "RandomProvider.h"
#include "Walnut/Random.h"

RandomProvider::RandomProvider()
{
	for (int i = 0; i < m_PoolSize; i++)
	{
		m_RandomsPool.push_back(Walnut::Random::Vec3(-0.5f, 0.5f));
	}
	int temp = 0;
}

void RandomProvider::IncrementRandomTracker()
{
	if (m_RandomTracker == m_RandomsPool.size() - 1)
	{
		m_RandomTracker = 0;
		return;
	}
	m_RandomTracker++;
}

void RandomProvider::IncrementRandomLock()
{
	if (m_RandomLock == m_RandomsPool.size() - 1)
	{
		m_RandomLock = 0;
		return;
	}
	m_RandomLock++;
}

void RandomProvider::UpdatePsuedoRandomDir()
{
	while (true)
	{
		RandomProvider::IncrementRandomLock();
		m_RandomsPool[m_RandomLock] = Walnut::Random::Vec3(-0.5f, 0.5f);
	}
}

glm::vec3 RandomProvider::GetPsuedoRandomDir()
{
	RandomProvider::IncrementRandomTracker();
	if (m_RandomTracker == m_RandomLock)
	{
		RandomProvider::IncrementRandomTracker();
	}
	return m_RandomsPool[m_RandomTracker];
}
