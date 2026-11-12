/* Base/rbx/CEvent.cpp — October 2013 (Win32 event wrapper).
   Carried verbatim from the March 2016 Base/rbx/CEvent.cpp Windows branch
   (thin Win32 wrappers; no 2013 divergence expected — CEvent.h matches).
   TODO: verify against 2013 (CEvent EA pending).
   Original file: (2013 TU layout TODO; 2016 equivalent: Client/Base/rbx/CEvent.cpp) */

#include "rbx/Debug.h"
#include "rbx/CEvent.h"
#include "Util/exception.h"

#include <windows.h>

static void WINAPI RbxThrowLastWin32()
{
	DWORD dwError = ::GetLastError();
	HRESULT hr = HRESULT_FROM_WIN32(dwError);
	throw RBX::runtime_error("HRESULT = 0x%.8X", hr);
}

RBX::CEvent::~CEvent() throw()
{
	if (m_h != NULL)
	{
		BOOL result = ::CloseHandle(m_h);
		RBXASSERT(result != 0);
		m_h = NULL;
	}
}

RBX::CEvent::CEvent(bool bManualReset)
	:m_h(NULL)
{
	m_h = ::CreateEvent(NULL, bManualReset ? TRUE : FALSE, FALSE, NULL);
	if (!m_h)
		RbxThrowLastWin32();
}

void RBX::CEvent::Set() throw()
{
	if (m_h && !::SetEvent(m_h))
		RbxThrowLastWin32();
}

void RBX::CEvent::Wait()
{
	WaitForSingleObject(*this, cINFINITE);
}

bool RBX::CEvent::Wait(int milliseconds)
{
	return WaitForSingleObject(*this, milliseconds) == cWAIT_OBJECT_0;
}

int RBX::CEvent::WaitForSingleObject(CEvent& event, int milliseconds)
{
	return ::WaitForSingleObject(event.m_h, milliseconds);
}
