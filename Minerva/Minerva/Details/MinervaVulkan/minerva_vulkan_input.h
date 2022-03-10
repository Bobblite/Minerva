#pragma once

namespace Minerva::Vulkan
{
	class Input
	{
	public:
		Input();

		void UpdateKeystate(Minerva::Keycode _code, Minerva::Keystate _state);

		bool IsTriggered(Minerva::Keycode _code);
		bool IsPressed(Minerva::Keycode _code);

	private:
		std::array<Keystate, static_cast<size_t>(Keycode::ENUM_COUNT)> m_PrevState;
		std::array<Keystate, static_cast<size_t>(Keycode::ENUM_COUNT)> m_CurrState;
	};
}

#include "minerva_vulkan_input.cpp"