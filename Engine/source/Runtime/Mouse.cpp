#include "Mouse.h"

namespace Runtime
{
	std::pair<int, int> Mouse::GetPos() const noexcept(!IS_DEBUG)
	{
		return { x,y };
	}

	std::optional<Mouse::RawDelta> Mouse::ReadRawDelta() noexcept(!IS_DEBUG)
	{
		if (m_rawDeltaBuffer.empty())
		{
			return std::nullopt;
		}
		const RawDelta d = m_rawDeltaBuffer.front();
		m_rawDeltaBuffer.pop();
		return d;
	}

	int Mouse::GetPosX() const noexcept(!IS_DEBUG)
	{
		return x;
	}

	int Mouse::GetPosY() const noexcept(!IS_DEBUG)
	{
		return y;
	}

	bool Mouse::IsInWindow() const noexcept(!IS_DEBUG)
	{
		return m_isInWindow;
	}

	bool Mouse::LeftIsPressed() const noexcept(!IS_DEBUG)
	{
		return m_leftIsPressed;
	}

	bool Mouse::RightIsPressed() const noexcept(!IS_DEBUG)
	{
		return m_rightIsPressed;
	}

	Mouse::Event Mouse::Read() noexcept(!IS_DEBUG)
	{
		if (m_buffer.size() > 0u)
		{
			Mouse::Event e = m_buffer.front();
			m_buffer.pop();
			return e;
		}
		else
		{
			return Mouse::Event();
		}
	}

	void Mouse::Flush() noexcept(!IS_DEBUG)
	{
		m_buffer = std::queue<Event>();
	}

	void Mouse::EnableRaw() noexcept(!IS_DEBUG)
	{
		m_rawEnabled = true;
	}

	void Mouse::DisableRaw() noexcept(!IS_DEBUG)
	{
		m_rawEnabled = false;
	}

	bool Mouse::RawEnabled() const noexcept(!IS_DEBUG)
	{
		return m_rawEnabled;
	}

	void Mouse::OnMouseMove(int newx, int newy) noexcept(!IS_DEBUG)
	{
		x = newx;
		y = newy;

		m_buffer.push(Mouse::Event(Mouse::Event::Type::Move, *this));
		TrimBuffer();
	}

	void Mouse::OnMouseLeave() noexcept(!IS_DEBUG)
	{
		m_isInWindow = false;
		m_buffer.push(Mouse::Event(Mouse::Event::Type::Leave, *this));
		TrimBuffer();
	}

	void Mouse::OnMouseEnter() noexcept(!IS_DEBUG)
	{
		m_isInWindow = true;
		m_buffer.push(Mouse::Event(Mouse::Event::Type::Enter, *this));
		TrimBuffer();
	}

	void Mouse::OnRawDelta(int dx, int dy) noexcept(!IS_DEBUG)
	{
		m_rawDeltaBuffer.push({ dx,dy });
		TrimBuffer();
	}

	void Mouse::OnLeftPressed(int x, int y) noexcept(!IS_DEBUG)
	{
		m_leftIsPressed = true;

		m_buffer.push(Mouse::Event(Mouse::Event::Type::LPress, *this));
		TrimBuffer();
	}

	void Mouse::OnLeftReleased(int x, int y) noexcept(!IS_DEBUG)
	{
		m_leftIsPressed = false;

		m_buffer.push(Mouse::Event(Mouse::Event::Type::LRelease, *this));
		TrimBuffer();
	}

	void Mouse::OnRightPressed(int x, int y) noexcept(!IS_DEBUG)
	{
		m_rightIsPressed = true;

		m_buffer.push(Mouse::Event(Mouse::Event::Type::RPress, *this));
		TrimBuffer();
	}

	void Mouse::OnRightReleased(int x, int y) noexcept(!IS_DEBUG)
	{
		m_rightIsPressed = false;

		m_buffer.push(Mouse::Event(Mouse::Event::Type::RRelease, *this));
		TrimBuffer();
	}

	void Mouse::OnWheelUp(int x, int y) noexcept(!IS_DEBUG)
	{
		m_buffer.push(Mouse::Event(Mouse::Event::Type::WheelUp, *this));
		TrimBuffer();
	}

	void Mouse::OnWheelDown(int x, int y) noexcept(!IS_DEBUG)
	{
		m_buffer.push(Mouse::Event(Mouse::Event::Type::WheelDown, *this));
		TrimBuffer();
	}

	void Mouse::TrimBuffer() noexcept(!IS_DEBUG)
	{
		while (m_buffer.size() > m_bufferSize)
		{
			m_buffer.pop();
		}
	}

	void Mouse::TrimRawInputBuffer() noexcept(!IS_DEBUG)
	{
		while (m_rawDeltaBuffer.size() > m_bufferSize)
		{
			m_rawDeltaBuffer.pop();
		}
	}

	void Mouse::OnWheelDelta(int x, int y, int delta) noexcept(!IS_DEBUG)
	{
		m_wheelDeltaCarry += delta;
		// generate events for every 120 
		while (m_wheelDeltaCarry >= WHEEL_DELTA)
		{
			m_wheelDeltaCarry -= WHEEL_DELTA;
			OnWheelUp(x, y);
		}
		while (m_wheelDeltaCarry <= -WHEEL_DELTA)
		{
			m_wheelDeltaCarry += WHEEL_DELTA;
			OnWheelDown(x, y);
		}
	}
}