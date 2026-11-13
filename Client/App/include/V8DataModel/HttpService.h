/* V8DataModel/HttpService.h -- October 2013 STUB (minimal shell).
   Evidence: FactoryProduct<HttpService, Instance> 0x367BCE;
     sHttpService = "HttpService".
   TODO: full decompile.
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/v8datamodel/HttpService.h */

#pragma once

#include "V8Tree/Instance.h"

namespace RBX {

	extern const char* const sHttpService;

	class HttpService
		: public DescribedCreatable<HttpService, Instance, sHttpService>
	{
	public:
		enum HttpContentType
		{
			APPLICATION_JSON = 0,
			TEXT_PLAIN = 2,
		};

		HttpService() {}
	};

} // namespace RBX
