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
	// GLOBAL: PLAYER 0x00d30ee9
	// GLOBAL: STUDIO 0x00ec17d5
	Channel DataModelJobs = 2;
	int DataModelJobsIniter = RegisterLogGroup("DataModelJobs", &DataModelJobs, NULL, FASTVARTYPE_STATIC);

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
