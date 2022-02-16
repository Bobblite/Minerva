#include "Minerva_Instance.h"
#include "Minerva_Exception.h"

#include <iostream>

namespace Minerva
{
	Instance::~Instance()
	{
		std::cout << "Destroying Minerva::Instance object\n";
	}

	void CreateInstance(Instance& _instance, const Minerva::Vulkan::Instance::Setup& _setup)
	{
		_instance.m_pVKInstanceHandle = std::make_unique<Minerva::Vulkan::Instance>();

		if (!_instance.m_pVKInstanceHandle)
		{
			//todo return error here
			std::cout << "Failed to create VK Instance handle\n";
			return;
		}

		_instance.m_sApplicationName = _setup.m_sApplicationName;
		_instance.m_uApplicationVersion = _setup.m_uApplicationVersion;
		

		//todo Do  this after error checking function is done
		/*
		if (_instance.m_pVKInstanceHandle->CreateVKInstance(_setup))
		{

		}*/
		try
		{
			_instance.m_pVKInstanceHandle->CreateVKInstance(_setup);
		}
		catch (const MinervaException& e)
		{
			std::cout << e.what();
		}
	}
}
