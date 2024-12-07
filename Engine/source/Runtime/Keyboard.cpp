#include "Keyboard.h"

namespace Runtime
{
	bool Keyboard::KeyIsPressed(unsigned char keycode) const noexcept(!IS_DEBUG)
	{
		return keystates[keycode];
	}

	Keyboard::Event Keyboard::ReadKey() noexcept(!IS_DEBUG)
	{
		if (keybuffer.size() > 0u)
		{
			Keyboard::Event e = keybuffer.front();
			keybuffer.pop();
			return e;
		}
		else
		{
			return Keyboard::Event();
		}
	}

	bool Keyboard::KeyIsEmpty() const noexcept(!IS_DEBUG)
	{
		return keybuffer.empty();
	}

	char Keyboard::ReadChar() noexcept(!IS_DEBUG)
	{
		if (charbuffer.size() > 0u)
		{
			unsigned char charcode = charbuffer.front();
			charbuffer.pop();
			return charcode;
		}
		else
		{
			return 0;
		}
	}

	bool Keyboard::CharIsEmpty() const noexcept(!IS_DEBUG)
	{
		return charbuffer.empty();
	}

	void Keyboard::FlushKey() noexcept(!IS_DEBUG)
	{
		keybuffer = std::queue<Event>();
	}

	void Keyboard::FlushChar() noexcept(!IS_DEBUG)
	{
		charbuffer = std::queue<char>();
	}

	void Keyboard::Flush() noexcept(!IS_DEBUG)
	{
		FlushKey();
		FlushChar();
	}

	void Keyboard::EnableAutorepeat() noexcept(!IS_DEBUG)
	{
		autorepeatEnabled = true;
	}

	void Keyboard::DisableAutorepeat() noexcept(!IS_DEBUG)
	{
		autorepeatEnabled = false;
	}

	bool Keyboard::AutorepeatIsEnabled() const noexcept(!IS_DEBUG)
	{
		return autorepeatEnabled;
	}

	void Keyboard::OnKeyPressed(unsigned char keycode) noexcept(!IS_DEBUG)
	{
		keystates[keycode] = true;
		keybuffer.push(Keyboard::Event(Keyboard::Event::Type::Press, keycode));
		TrimBuffer(keybuffer);
	}

	void Keyboard::OnKeyReleased(unsigned char keycode) noexcept(!IS_DEBUG)
	{
		keystates[keycode] = false;
		keybuffer.push(Keyboard::Event(Keyboard::Event::Type::Release, keycode));
		TrimBuffer(keybuffer);
	}

	void Keyboard::OnChar(char character) noexcept(!IS_DEBUG)
	{
		charbuffer.push(character);
		TrimBuffer(charbuffer);
	}

	void Keyboard::ClearState() noexcept(!IS_DEBUG)
	{
		keystates.reset();
	}

	template<typename T>
	void Keyboard::TrimBuffer(std::queue<T>& buffer) noexcept(!IS_DEBUG)
	{
		while (buffer.size() > bufferSize)
		{
			buffer.pop();
		}
	}
}