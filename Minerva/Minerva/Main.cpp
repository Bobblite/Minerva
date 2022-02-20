#include "MinervaAPI/Minerva.h"
#include <iostream>
#include <Windows.h>
#include <wincon.h>
#include <string_view>

void LogError(std::string_view _str)
{
	HANDLE  hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, FOREGROUND_RED);

	std::cout << _str << "\n";
}

void LogWarn(std::string_view _str)
{
	HANDLE  hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN);
	std::cout << _str << "\n";
}

int main(int argc, const char* argv[])
{
#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif//DEBUG || 
	{

		Minerva::Instance myInst;
		Minerva::CreateInstance(myInst,
			{.m_sApplicationName = "Assignment 2",
			.m_uApplicationVersion = 0,
			.m_bIsRenderDoc = true,
			.m_bIsDebug = true},
			LogError,
			LogWarn);
	
		Minerva::Device device;
		myInst.Create(device, {
			.m_Type = Minerva::Vulkan::Device::Type::RENDER_AND_SWAP,
			.m_Discrete = Minerva::Vulkan::Device::Discrete::DISCRETE_ONLY });

		Minerva::Window window;
		device.Create(window, {
			.m_Width = 1280,
			.m_Height = 720,
			.m_bFullScreen = false,
			.m_bClearOnRender = false,
			.m_bVSyncOn = false,
			.m_ClearColorR = 0.45f,
			.m_ClearColorG = 0.45f,
			.m_ClearColorB = 0.45f,
			.m_ClearColorA = 1.f});

		while (window.ProcessInput())
		{
			window.DrawFrame();
		}

		vkDeviceWaitIdle(device.m_pVKDeviceHandle->m_VKDevice);
	}
	
}