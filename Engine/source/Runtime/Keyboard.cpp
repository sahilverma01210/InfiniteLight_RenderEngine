#include "Keyboard.h"

namespace Runtime
{
	bool Keyboard::KeyIsPressed(unsigned char keycode) const noexcept(!IS_DEBUG)
	{
		return m_keystates[keycode];
	}

	Keyboard::Event Keyboard::ReadKey() noexcept(!IS_DEBUG)
	{
		if (m_keybuffer.size() > 0u)
		{
			Keyboard::Event e = m_keybuffer.front();
			m_keybuffer.pop();
			return e;
		}
		else
		{
			return Keyboard::Event();
		}
	}

	bool Keyboard::KeyIsEmpty() const noexcept(!IS_DEBUG)
	{
		return m_keybuffer.empty();
	}

	char Keyboard::ReadChar() noexcept(!IS_DEBUG)
	{
		if (m_charbuffer.size() > 0u)
		{
			unsigned char charcode = m_charbuffer.front();
			m_charbuffer.pop();
			return charcode;
		}
		else
		{
			return 0;
		}
	}

	bool Keyboard::CharIsEmpty() const noexcept(!IS_DEBUG)
	{
		return m_charbuffer.empty();
	}

	void Keyboard::FlushKey() noexcept(!IS_DEBUG)
	{
		m_keybuffer = std::queue<Event>();
	}

	void Keyboard::FlushChar() noexcept(!IS_DEBUG)
	{
		m_charbuffer = std::queue<char>();
	}

	void Keyboard::Flush() noexcept(!IS_DEBUG)
	{
		FlushKey();
		FlushChar();
	}

	void Keyboard::EnableAutorepeat() noexcept(!IS_DEBUG)
	{
		m_autorepeatEnabled = true;
	}

	void Keyboard::DisableAutorepeat() noexcept(!IS_DEBUG)
	{
		m_autorepeatEnabled = false;
	}

	bool Keyboard::AutorepeatIsEnabled() const noexcept(!IS_DEBUG)
	{
		return m_autorepeatEnabled;
	}

	void Keyboard::OnKeyPressed(unsigned char keycode) noexcept(!IS_DEBUG)
	{
		m_keystates[keycode] = true;
		m_keybuffer.push(Keyboard::Event(Keyboard::Event::Type::Press, keycode));
		TrimBuffer(m_keybuffer);
	}

	void Keyboard::OnKeyReleased(unsigned char keycode) noexcept(!IS_DEBUG)
	{
		m_keystates[keycode] = false;
		m_keybuffer.push(Keyboard::Event(Keyboard::Event::Type::Release, keycode));
		TrimBuffer(m_keybuffer);
	}

	void Keyboard::OnChar(char character) noexcept(!IS_DEBUG)
	{
		m_charbuffer.push(character);
		TrimBuffer(m_charbuffer);
	}

	void Keyboard::ClearState() noexcept(!IS_DEBUG)
	{
		m_keystates.reset();
	}

	template<typename T>
	void Keyboard::TrimBuffer(std::queue<T>& buffer) noexcept(!IS_DEBUG)
	{
		while (buffer.size() > m_bufferSize)
		{
			buffer.pop();
		}
	}
}