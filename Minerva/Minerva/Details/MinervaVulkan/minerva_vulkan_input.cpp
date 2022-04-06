namespace Minerva::Vulkan
{
	Input::Input()
	{
		// Initialize keystates with the initial states when application starts
		for (int i{ 0 }; i < m_PrevKeystates.size(); ++i)
			m_PrevKeystates[i] = IsPressed(i);
	}

	void Input::UpdateMousePositionCB(int _x, int _y)
	{
		m_MousePosition[0] = _x;
		m_MousePosition[1] = _y;
	}

	void Input::UpdateKeyUpCB(Minerva::Keycode _keycode)
	{
		m_PrevKeystates[static_cast<int>(_keycode)] = false;
	}

	bool Input::IsTriggered(char _key)
	{
		int keycode = CharToVK(_key);

		bool prevState{ m_PrevKeystates[keycode] };
		bool currState{ IsPressed(_key) };
		if (currState && !prevState)
		{
			m_PrevKeystates[keycode] = currState;
		}

		return currState && !prevState;
	}

	bool Input::IsTriggered(Minerva::Keycode _key)
	{
		bool prevState{ m_PrevKeystates[static_cast<int>(_key)]};
		bool currState{ IsPressed(_key) };

		//if (currState && !prevState)
		//{
			m_PrevKeystates[static_cast<int>(_key)] = currState;
		//}

		return (currState && !prevState);
	}

	bool Input::IsReleased(char _key)
	{
		int keycode = CharToVK(_key);
		bool prevState{ m_PrevKeystates[keycode] };
		bool currState{ IsPressed(_key) };

		if (!currState && prevState)
			m_PrevKeystates[keycode] = currState;

		return (!currState && prevState);
	}

	bool Input::IsReleased(Minerva::Keycode _key)
	{
		bool prevState{ m_PrevKeystates[static_cast<int>(_key)] };
		bool currState{ IsPressed(_key) };

		//if (!currState && prevState)
			m_PrevKeystates[static_cast<int>(_key)] = currState;

		return (!currState && prevState);
	}

	bool Input::IsPressed(char _key)
	{
		int keycode = CharToVK(_key);
		return (1 << 15) & GetAsyncKeyState(keycode);
	}

	bool Input::IsPressed(Minerva::Keycode _key)
	{
		return (1 << 15) & GetAsyncKeyState(static_cast<int>(_key));
	}

	const std::array<int, 2>& Input::GetMousePosition() const
	{
		return m_MousePosition;
	}


	int Input::CharToVK(char _key)
	{
		return (_key >= 'a' && _key <= 'z') ? _key - 'a' + 'A' : _key; // Convert to uppercase
	}

}