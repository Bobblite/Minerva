#include "Minerva_Window.h"

namespace Minerva
{
	bool Window::ProcessInput()
	{
		return m_pVKWindowHandle->ProcessInput();
	}

	void Window::DrawFrame()
	{
		m_pVKWindowHandle->DrawFrame();
	}
}
