#pragma once

namespace Minerva::Vulkan
{
	class Input
	{
	public:
		Input();
		void UpdateMousePositionCB(int _x, int _y);
		void UpdateKeyUpCB(Minerva::Keycode _keycode);

		bool IsTriggered(char _key);
		bool IsReleased(char _key);
		bool IsPressed(char _key);
		bool IsTriggered(Minerva::Keycode _key);
		bool IsReleased(Minerva::Keycode _key);
		bool IsPressed(Minerva::Keycode _key);
		const std::array<int, 2>& GetMousePosition() const;


		int CharToVK(char _key);

	private:
		std::array<bool, 256> m_PrevKeystates;
		std::vector<int> m_checkedKeys;
		std::array<int, 2> m_MousePosition;
	};
}

#include "minerva_vulkan_input.cpp"