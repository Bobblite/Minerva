#include "MinervaAPI/Minerva.h"
#include <iostream>

int main(int argc, const char* argv[])
{
	Minerva::Instance myInst;
	Minerva::CreateInstance(myInst,
		{.m_sApplicationName = "Assignment 2",
		.m_uApplicationVersion = 0,
		.m_bIsRenderDoc = true,
		.m_bIsDebug = true});
}