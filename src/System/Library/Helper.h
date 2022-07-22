#pragma once

template<class T>
void SafeRelease(T*& p) {
	if (p) {
		p->Release();
		p = nullptr;
	}
}
template<class T>
void SafeDelete(T*& p) {
	if (p) {
		delete p;
		p = nullptr;
	}
}

#define SafeClear(p) (p).clear()

void DebugLog(const char* format, ...)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	vprintf(format, valist);
	va_end(valist);
#endif
}

template<class T>
using weak_ptr = std::weak_ptr<T>;

template<class T>
using shared_ptr = std::shared_ptr<T>;

using 