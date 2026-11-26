/* App/v8datamodel/FastLogSettings.cpp -- October 2013 (partial).
   2013 registers its log groups and fast flags from one TU: the dynamic initializers run in
   one block at PLAYER 0x00a65c00.. (CloseDataModel 0x00a65e50 -> FLog::CloseDataModel 0x00d30f17,
   default 1; LegacyLock 0x00a65eb0 -> 0x00de3f76, default 0; CoreScripts 0x00a65ff0 ->
   0x00d30f1a, default 1; DataModelJobs 0x00a654e0 -> 0x00d30ee9, default 2). The 2016 tree has
   the same file (App/v8datamodel/FastLogSettings.cpp). Only the groups the DataModel code uses
   are defined here; ContentProviderRequests and others still live in their users' files.
   Without a definition these channels were unresolved and the link bound them all to one
   address, so two different channels compared equal in reccmp (DataModel ctor 0x0064ec50). */

#include "stdafx.h"
#include "FastLog.h"

// LOGVARIABLE(group, default) spelled out so each channel carries its reccmp marker.
namespace FLog {
	// Initializer PLAYER 0x00a65080 / STUDIO 0x00b1c250 (the shell's GameVerbs log through it).
	// GLOBAL: PLAYER 0x00de3f41
	// GLOBAL: STUDIO 0x00f84e19
	Channel Verbs = 0;
	int VerbsIniter = RegisterLogGroup("Verbs", &Verbs, NULL, FASTVARTYPE_STATIC);

	// Initializer PLAYER 0x00a65100 / STUDIO 0x00b1c2d0 (the shell's View logs through it).
	// GLOBAL: PLAYER 0x00d30edd
	// GLOBAL: STUDIO 0x00ec17c9
	Channel RobloxWndInit = 1;
	int RobloxWndInitIniter = RegisterLogGroup("RobloxWndInit", &RobloxWndInit, NULL, FASTVARTYPE_STATIC);

	// Initializer PLAYER 0x00a65120 / STUDIO 0x00b1c2f0 (the shell's LogManager logs through it).
	// GLOBAL: PLAYER 0x00d30ede
	// GLOBAL: STUDIO 0x00ec17ca
	Channel CrashReporterInit = 1;
	int CrashReporterInitIniter = RegisterLogGroup("CrashReporterInit", &CrashReporterInit, NULL, FASTVARTYPE_STATIC);

	// Initializer PLAYER 0x00a651c0 / STUDIO 0x00b1c390, ahead of DataModelJobs's.
	// GLOBAL: PLAYER 0x00de3f47
	// GLOBAL: STUDIO 0x00f84e1f
	Channel GuiTargetLifetime = 0;
	int GuiTargetLifetimeIniter = RegisterLogGroup("GuiTargetLifetime", &GuiTargetLifetime, NULL, FASTVARTYPE_STATIC);

	// GLOBAL: PLAYER 0x00d30ee9
	// GLOBAL: STUDIO 0x00ec17d5
	Channel DataModelJobs = 2;
	int DataModelJobsIniter = RegisterLogGroup("DataModelJobs", &DataModelJobs, NULL, FASTVARTYPE_STATIC);
}

// The fast flags 2013 registers from this block, in initializer order (PLAYER DebugLocalRcc
// 0x00a65800, CrispFiltering 0x00a65c60, ImmediateCrash 0x00a65ca0, ImmediateHang 0x00a65cc0);
// Network's Server, Player and CrashReporter read them.
namespace FFlag {
	// GLOBAL: PLAYER 0x00de3f5f
	// GLOBAL: STUDIO 0x00f84e38
	bool DebugLocalRccServerConnection = false;
	int DebugLocalRccServerConnectionIniter = FLog::RegisterFlag("DebugLocalRccServerConnection", &DebugLocalRccServerConnection, NULL, FASTVARTYPE_STATIC);
	// .data byte starts at 1.
	// GLOBAL: PLAYER 0x00d30f10
	// GLOBAL: STUDIO 0x00ec17fc
	bool CrispFilteringEnabled = true;
	int CrispFilteringEnabledIniter = FLog::RegisterFlag("CrispFilteringEnabled", &CrispFilteringEnabled, NULL, FASTVARTYPE_STATIC);
	// GLOBAL: PLAYER 0x00de3f6c
	// GLOBAL: STUDIO 0x00f84e45
	bool ImmediateCrashUploadEnabled = false;
	int ImmediateCrashUploadEnabledIniter = FLog::RegisterFlag("ImmediateCrashUploadEnabled", &ImmediateCrashUploadEnabled, NULL, FASTVARTYPE_STATIC);
	// GLOBAL: PLAYER 0x00de3f6d
	// GLOBAL: STUDIO 0x00f84e46
	bool ImmediateHangUploadEnabled = false;
	int ImmediateHangUploadEnabledIniter = FLog::RegisterFlag("ImmediateHangUploadEnabled", &ImmediateHangUploadEnabled, NULL, FASTVARTYPE_STATIC);
}

namespace FLog {
	// GLOBAL: PLAYER 0x00d30f17
	// GLOBAL: STUDIO 0x00ec1803
	Channel CloseDataModel = 1;
	int CloseDataModelIniter = RegisterLogGroup("CloseDataModel", &CloseDataModel, NULL, FASTVARTYPE_STATIC);

	// GLOBAL: PLAYER 0x00de3f76
	// GLOBAL: STUDIO 0x00f84e4f
	Channel LegacyLock = 0;
	int LegacyLockIniter = RegisterLogGroup("LegacyLock", &LegacyLock, NULL, FASTVARTYPE_STATIC);

	// GLOBAL: PLAYER 0x00d30f1a
	// GLOBAL: STUDIO 0x00ec1806
	Channel CoreScripts = 1;
	int CoreScriptsIniter = RegisterLogGroup("CoreScripts", &CoreScripts, NULL, FASTVARTYPE_STATIC);
}

// Dynamic (registered with kind 1; iOS 10-29 spells it DFFlag::UseClusterCache). Initializer
// PLAYER 0x00a661d0 / STUDIO 0x00b1d3e0; Network's Server::onServiceProvider reads it.
namespace DFFlag {
	// GLOBAL: PLAYER 0x00de3f88
	// GLOBAL: STUDIO 0x00f84e62
	bool UseClusterCache = false;
	int UseClusterCacheIniter = FLog::RegisterFlag("UseClusterCache", &UseClusterCache, NULL, FASTVARTYPE_DYNAMIC);
}
