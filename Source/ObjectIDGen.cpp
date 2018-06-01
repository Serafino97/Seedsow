
#include "StdAfx.h"
#include "ObjectIDGen.h"
#include "DatabaseIO.h"

CObjectIDGenerator::CObjectIDGenerator()
{
	m_dwHintDynamicGUID = 0x80000000;
	LoadState();
}

CObjectIDGenerator::~CObjectIDGenerator()
{
}

void CObjectIDGenerator::LoadState()
{
	m_dwHintDynamicGUID = g_pDBIO->GetHighestWeenieID(0x80000000, 0xFF000000);
}

DWORD CObjectIDGenerator::GenerateGUID(eGUIDClass type)
{
	switch (type)
	{
	case eDynamicGUID:
		{
			if (m_dwHintDynamicGUID >= 0xF0000000)
			{
				SERVER_ERROR << "Dynamic GUID overflow!";
				return 0;
			}
			
			return (++m_dwHintDynamicGUID);
		}
	}

	return 0;
}