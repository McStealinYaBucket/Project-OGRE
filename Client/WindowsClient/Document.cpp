/* WindowsClient/Document.cpp -- October 2013 client shell (PLAYER only; no 2013 __FILE__ string names
   it, path from the 2016 tree). Written so far: the destructor, Shutdown and the verb release it calls.
   Against 2016: Shutdown gives the scripts their deadline itself (2016 does it in PrepareShutdown),
   removes the View's jobs and stops it under a write lock (OverlayDataModel::DualLegacyLock when there
   is an overlay DataModel and more than one TaskScheduler thread), then deletes the View and releases
   the marshaller; it does not shut the Game down. */

// App's headers first: they include winsock2.h ahead of windows.h.
#include "V8DataModel/DataModel.h"
#include "V8DataModel/Game.h"
#include "V8DataModel/OverlayDataModel.h"
#include "V8DataModel/ScriptContext.h"
#include "rbx/TaskScheduler.h"
#include "FastLog.h"

#include "Document.h"

#include "FunctionMarshaller.h"
#include "GameVerbs.h"
#include "View.h"

// View.cpp keeps its assert lines by layout, so View's size check sits here.
DECOMP_SIZE_ASSERT(RBX::Document, 0x28);
DECOMP_SIZE_ASSERT(RBX::View, 0xcc);

LOGGROUP(PlayerShutdownLuaTimeoutSeconds)

namespace RBX {

	// Every member destroys itself, the View last (~View 0x00465b90, then operator delete).
	// FUNCTION: PLAYER 0x0041d020
	Document::~Document()
	{
	}

	// FLog::PlayerShutdownLuaTimeoutSeconds is App's (FastLogSettings.obj registers it: initializer
	// 0x00a663b0, .bss 0x00de3f92); until App defines it the two loads do not pair.
	// STUB: PLAYER 0x0041d270
	void Document::Shutdown()
	{
		if (m_unk0x00)
			m_unk0x00->RemoveJobs();

		if (game && game->getDataModel())
		{
			// give scripts a deadline to finish
			if (FLog::PlayerShutdownLuaTimeoutSeconds > 0)
				if (ScriptContext* scriptContext = ServiceProvider::find<ScriptContext>(game->getDataModel().get()))
					scriptContext->setTimeout(FLog::PlayerShutdownLuaTimeoutSeconds);

			if (game->getOverlayDataModel() && TaskScheduler::singleton().getThreadCount() > 1)
			{
				OverlayDataModel::DualLegacyLock lock(game->getOverlayDataModel(), game->getDataModel(), DataModelJob::Write);
				if (m_unk0x00)
					m_unk0x00->FUN_00465e70();
				FUN_0041df60();
			}
			else
			{
				DataModel::LegacyLock lock(game->getDataModel().get(), DataModelJob::Write);
				if (m_unk0x00)
					m_unk0x00->FUN_00465e70();
				FUN_0041df60();
			}
		}

		m_unk0x00.reset();

		if (marshaller)
			FunctionMarshaller::ReleaseWindow(marshaller);
	}

	// FUNCTION: PLAYER 0x0041df60
	void Document::FUN_0041df60()
	{
		if (m_unk0x0c)
			m_unk0x0c.reset();
		if (m_unk0x10)
			m_unk0x10.reset();
		if (m_unk0x18)
			m_unk0x18.reset();
		if (m_unk0x14)
			m_unk0x14.reset();
	}

} // namespace RBX
