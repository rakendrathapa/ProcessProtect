#pragma once

template<typename Tlock>
struct AutoLock
{
	AutoLock(TLock& lock) : _lock(lock)
	{
		_lock.Lock();
	}

	~AutoLock()
	{
		_lock.Unlock();
	}

private:
	Tlock& lock;
};
