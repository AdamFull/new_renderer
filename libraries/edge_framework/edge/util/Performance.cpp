#include "Performance.h"

#ifdef _WIN32
#include <Windows.h>

namespace edge
{
	struct Stopwatch::StopwatchImpl
	{
		LARGE_INTEGER frequency;
		LARGE_INTEGER start_time;

		StopwatchImpl()
		{
			QueryPerformanceFrequency(&frequency);
			QueryPerformanceCounter(&start_time);
		}

		float Elapsed()
		{
			LARGE_INTEGER current_time;
			QueryPerformanceCounter(&current_time);
			return static_cast<float>((double)current_time.QuadPart - (double)start_time.QuadPart) / (double)frequency.QuadPart;
		}
	};
}

#endif

namespace edge
{
	Stopwatch::Stopwatch()
	{
		impl = new StopwatchImpl();
	}

	Stopwatch::~Stopwatch()
	{
		delete impl;
	}

	float Stopwatch::ElapsedSeconds() const
	{
		return impl->Elapsed();
	}

	float Stopwatch::ElapsedMilliseconds() const
	{
		return ElapsedSeconds() * 1000.f;
	}

	float Stopwatch::ElapsedMicroseconds() const
	{
		return ElapsedSeconds() * 1000000.f;
	}


	FPSCounter::FPSCounter() : total_frame_count(0u), frame_count(0), delta_time(0), accumulated_time(0), fps(0)
	{
		last_time = ElapsedSeconds();
	}

	void FPSCounter::Frame()
	{
		++total_frame_count;
		++frame_count;

		float current_time = ElapsedSeconds();
		delta_time = current_time - last_time;
		last_time = current_time;

		accumulated_time += delta_time;

		if (accumulated_time >= 1.0f)
		{
			fps = frame_count / accumulated_time;
			frame_count = 0;
			accumulated_time = 0.f;
		}
	}

	float FPSCounter::GetFps() const
	{
		return fps;
	}

	float FPSCounter::GetFrameTime() const
	{
		return delta_time;
	}

	uint32_t FPSCounter::GetTotalFrames() const
	{
		return total_frame_count;
	}
}