/* App/util/FileSystem.cpp — October 2013 STUB. TODO: full decompile. */

#include "stdafx.h"

#include "Util/FileSystem.h"
#include "RbxFormat.h"

#include <io.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

namespace RBX {
	namespace FileSystem {

		void getUserDirectory(std::string& result)
		{
			// TODO(decompile): 2013 user-directory resolution.
			result.clear();
		}

		bool filepathExists(const boost::filesystem::path& p)
		{
			return boost::filesystem::exists(p);
		}

		std::string getBaseCacheDirectory(bool create)
		{
			static std::string cached;
			static bool have = false;
			if (have)
				return cached;
			char tmpl[] = "/tmp/roblox.XXXXXX";
			std::string base = ::_mktemp(tmpl);
			base += "/Roblox";
			if (create)
			{
				boost::system::error_code ec;
				if (!boost::filesystem::create_directories(base, ec))
					throw RBX::runtime_error("Unable to create %s", base.c_str());
			}
			if (!filepathExists(base))
				return "";
			base += "/";
			cached = base;
			have = true;
			return cached;
		}

		std::string getCacheDirectory(bool create, const char* sub)
		{
			if (sub == NULL)
				return getBaseCacheDirectory(create);
			std::string base = getBaseCacheDirectory(create);
			if (base.empty())
				return "";
			boost::filesystem::path p(base);
			p /= sub;
			base = p.string();
			if (create)
			{
				boost::system::error_code ec;
				if (!boost::filesystem::create_directories(base, ec))
					throw RBX::runtime_error("Unable to create %s", base.c_str());
			}
			if (!filepathExists(base))
				return "";
			return base + "/";
			}


		// mac: 0x1F969E: see header.  Body from the 2016 carry (the 2013 mode int
		// is unused by the only call site).
		void clearCacheDirectory(const char* subDirectory, int mode)
		{
			(void)mode;
			std::string cachePath = getCacheDirectory(false, subDirectory);
			namespace fs = boost::filesystem;
			boost::system::error_code ec;
			if (!cachePath.empty() && fs::exists(cachePath, ec) && !ec)
			{
				fs::directory_iterator end_iter;
				for (fs::directory_iterator iter(cachePath); end_iter != iter; ++iter)
				{
					if (!fs::is_directory(iter->status()))
					{
						boost::system::error_code ec;
						boost::filesystem::remove(*iter, ec);
					}
				}
			}
		}

	} // namespace FileSystem
} // namespace RBX
