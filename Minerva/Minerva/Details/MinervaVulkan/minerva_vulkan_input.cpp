namespace Minerva::Vulkan
{
	Input::Input() :
		m_PrevState{ Keystate::KEY_UP }, m_CurrState{ Keystate::KEY_UP } {}

	void Input::UpdateKeystate(Minerva::Keycode _code, Minerva::Keystate _state)
	{
		m_PrevState[static_cast<uint32_t>(_code)] = m_CurrState[static_cast<uint32_t>(_code)];
		m_CurrState[static_cast<uint32_t>(_code)] = _state;
	}

	bool Input::IsTriggered(Minerva::Keycode _code)
	{
		if (m_CurrState[static_cast<uint32_t>(_code)] == Minerva::Keystate::KEY_DOWN)
			return true;
		else
			return false;
	}

	bool Input::IsPressed(Minerva::Keycode _code)
	{
		if (m_CurrState[static_cast<uint32_t>(_code)] == Minerva::Keystate::KEY_DOWN)
		{
			//UpdateKeystate(_code, Keystate::KEY_UP);
			return true;
		}
		return false;
	}

}