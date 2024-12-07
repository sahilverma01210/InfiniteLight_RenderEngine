#include "Mouse.h"

namespace Runtime
{
	std::pair<int, int> Mouse::GetPos() const noexcept(!IS_DEBUG)
	{
		return { x,y };
	}

	std::optional<Mouse::RawDelta> Mouse::ReadRawDelta() noexcept(!IS_DEBUG)
	{
		if (rawDeltaBuffer.empty())
		{
			return std::nullopt;
		}
		const RawDelta d = rawDeltaBuffer.front();
		rawDeltaBuffer.pop();
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
		return isInWindow;
	}

	bool Mouse::LeftIsPressed() const noexcept(!IS_DEBUG)
	{
		return leftIsPressed;
	}

	bool Mouse::RightIsPressed() const noexcept(!IS_DEBUG)
	{
		return rightIsPressed;
	}

	Mouse::Event Mouse::Read() noexcept(!IS_DEBUG)
	{
		if (buffer.size() > 0u)
		{
			Mouse::Event e = buffer.front();
			buffer.pop();
			return e;
		}
		else
		{
			return Mouse::Event();
		}
	}

	void Mouse::Flush() noexcept(!IS_DEBUG)
	{
		buffer = std::queue<Event>();
	}

	void Mouse::EnableRaw() noexcept(!IS_DEBUG)
	{
		rawEnabled = true;
	}

	void Mouse::DisableRaw() noexcept(!IS_DEBUG)
	{
		rawEnabled = false;
	}

	bool Mouse::RawEnabled() const noexcept(!IS_DEBUG)
	{
		return rawEnabled;
	}

	void Mouse::OnMouseMove(int newx, int newy) noexcept(!IS_DEBUG)
	{
		x = newx;
		y = newy;

		buffer.push(Mouse::Event(Mouse::Event::Type::Move, *this));
		TrimBuffer();
	}

	void Mouse::OnMouseLeave() noexcept(!IS_DEBUG)
	{
		isInWindow = false;
		buffer.push(Mouse::Event(Mouse::Event::Type::Leave, *this));
		TrimBuffer();
	}

	void Mouse::OnMouseEnter() noexcept(!IS_DEBUG)
	{
		isInWindow = true;
		buffer.push(Mouse::Event(Mouse::Event::Type::Enter, *this));
		TrimBuffer();
	}

	void Mouse::OnRawDelta(int dx, int dy) noexcept(!IS_DEBUG)
	{
		rawDeltaBuffer.push({ dx,dy });
		TrimBuffer();
	}

	void Mouse::OnLeftPressed(int x, int y) noexcept(!IS_DEBUG)
	{
		leftIsPressed = true;

		buffer.push(Mouse::Event(Mouse::Event::Type::LPress, *this));
		TrimBuffer();
	}

	void Mouse::OnLeftReleased(int x, int y) noexcept(!IS_DEBUG)
	{
		leftIsPressed = false;

		buffer.push(Mouse::Event(Mouse::Event::Type::LRelease, *this));
		TrimBuffer();
	}

	void Mouse::OnRightPressed(int x, int y) noexcept(!IS_DEBUG)
	{
		rightIsPressed = true;

		buffer.push(Mouse::Event(Mouse::Event::Type::RPress, *this));
		TrimBuffer();
	}

	void Mouse::OnRightReleased(int x, int y) noexcept(!IS_DEBUG)
	{
		rightIsPressed = false;

		buffer.push(Mouse::Event(Mouse::Event::Type::RRelease, *this));
		TrimBuffer();
	}

	void Mouse::OnWheelUp(int x, int y) noexcept(!IS_DEBUG)
	{
		buffer.push(Mouse::Event(Mouse::Event::Type::WheelUp, *this));
		TrimBuffer();
	}

	void Mouse::OnWheelDown(int x, int y) noexcept(!IS_DEBUG)
	{
		buffer.push(Mouse::Event(Mouse::Event::Type::WheelDown, *this));
		TrimBuffer();
	}

	void Mouse::TrimBuffer() noexcept(!IS_DEBUG)
	{
		while (buffer.size() > bufferSize)
		{
			buffer.pop();
		}
	}

	void Mouse::TrimRawInputBuffer() noexcept(!IS_DEBUG)
	{
		while (rawDeltaBuffer.size() > bufferSize)
		{
			rawDeltaBuffer.pop();
		}
	}

	void Mouse::OnWheelDelta(int x, int y, int delta) noexcept(!IS_DEBUG)
	{
		wheelDeltaCarry += delta;
		// generate events for every 120 
		while (wheelDeltaCarry >= WHEEL_DELTA)
		{
			wheelDeltaCarry -= WHEEL_DELTA;
			OnWheelUp(x, y);
		}
		while (wheelDeltaCarry <= -WHEEL_DELTA)
		{
			wheelDeltaCarry += WHEEL_DELTA;
			OnWheelDown(x, y);
		}
	}
}