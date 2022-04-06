#pragma once

namespace Minerva
{
	std::unique_ptr<Minerva::Vulkan::Input> Input::m_VKInputHandle;


	inline void Input::Initialize()
	{
		if (!m_VKInputHandle)
			m_VKInputHandle = std::make_unique<Minerva::Vulkan::Input>();
	}

	inline void Input::UpdateMousePositionCallback(int _x, int _y)
	{
		if (m_VKInputHandle)
			return m_VKInputHandle->UpdateMousePositionCB(_x, _y);
	}

	inline void Input::UpdateKeyUpCallback(Minerva::Keycode _keycode)
	{
		if (m_VKInputHandle)
			return m_VKInputHandle->UpdateKeyUpCB(_keycode);
	}

	inline bool Input::IsTriggered(char _key)
	{
		if (m_VKInputHandle)
			return m_VKInputHandle->IsTriggered(_key);
	}

	inline bool Input::IsReleased(char _key)
	{
		if (m_VKInputHandle)
			return m_VKInputHandle->IsReleased(_key);
	}


	inline bool Input::IsPressed(char _key)
	{
		if (m_VKInputHandle)
			return m_VKInputHandle->IsPressed(_key);
	}


	inline bool Input::IsTriggered(Keycode _key)
	{
		if (m_VKInputHandle)
			return m_VKInputHandle->IsTriggered(_key);
	}

	inline bool Input::IsReleased(Keycode _key)
	{
		if (m_VKInputHandle)
			return m_VKInputHandle->IsReleased(_key);
	}


	inline bool Input::IsPressed(Keycode _key)
	{
		if (m_VKInputHandle)
			return m_VKInputHandle->IsPressed(_key);
	}

	inline const std::array<int, 2>& Input::GetMousePosition()
	{
		if (m_VKInputHandle)
			return m_VKInputHandle->GetMousePosition();
	}

}