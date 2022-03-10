#pragma once

namespace Minerva
{
	std::unique_ptr<Minerva::Vulkan::Input> Input::m_VKInputHandle;

	inline void Input::Initialize()
	{
		if (!m_VKInputHandle)
			m_VKInputHandle = std::make_unique<Minerva::Vulkan::Input>();
	}

	inline bool Input::IsTriggered(Keycode _code)
	{
		if (m_VKInputHandle)
			return m_VKInputHandle->IsTriggered(_code);
	}

	inline bool Input::IsPressed(Keycode _code)
	{
		if (m_VKInputHandle)
			return m_VKInputHandle->IsPressed(_code);
	}

	inline void Input::UpdateKeystate(Keycode _code, Keystate _state)
	{
		if (m_VKInputHandle)
			m_VKInputHandle->UpdateKeystate(_code, _state);
	}
}