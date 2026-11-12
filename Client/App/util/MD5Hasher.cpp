/* App/util/MD5Hasher.cpp — October 2013 (hasher shell + public-domain MD5).
   MD5 core (md5.h/md5.c) vendored verbatim from the 2016 tree (public-domain
   Solar Designer implementation, same as 2016 — third-party, never decompile).
   MD5HasherImpl/convertToLegacyHash/create carried from 2016
   App/util/MD5Hasher.cpp (header already in tree, byte-shape matches:
   create/addData/toString/c_str/toBuffer/convertToLegacyHash).
   DIVERGENCE: CollectMd5Hash opens the file directly (2016 routes through
   utf8_decode via StringConv.h — absent from this tree, TODO).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/util/MD5Hasher.cpp */

#include "stdafx.h"

#include "Util/MD5Hasher.h"

#include "Util/md5.h"
// Windows needs digest length defined
#define MD5_DIGEST_LENGTH 16

#include <fstream>
#include <boost/scoped_ptr.hpp>
#include <boost/static_assert.hpp>

namespace RBX
{
	class MD5HasherImpl : public MD5Hasher
	{
		MD5_CTX context;

		unsigned char resultBuffer[MD5_DIGEST_LENGTH];
		std::string resultString;
		bool resultReady;

	public:
		MD5HasherImpl()
			: resultReady(false)
		{
			MD5_Init(&context);
		}

		~MD5HasherImpl()
		{
		}

		virtual void addData(std::istream& data)
		{
			RBXASSERT(!resultReady);

			data.clear();
			data.seekg(0, std::ios_base::beg);
			do
			{
				char buffer[1024];
				data.read(buffer, 1024);
				addData(buffer, (size_t)data.gcount());
			}
			while (data.gcount() > 0);
		}

		virtual void addData(const std::string& data)
		{
			RBXASSERT(!resultReady);

			MD5_Update(&context, (void*)data.data(), data.length());
		}

		virtual void addData(const char* data, size_t nBytes)
		{
			RBXASSERT(!resultReady);

			MD5_Update(&context, (void*)data, nBytes);
		}

		virtual const std::string& toString()
		{
			finish();

			return resultString;
		}

		virtual const char* c_str()
		{
			finish();

			return resultString.c_str();
		}

		void finish()
		{
			if (!resultReady)
			{
				resultReady = true;

				MD5_Final(resultBuffer, &context);

				resultString.reserve(MD5_DIGEST_LENGTH * 2);

				for (size_t i = 0; i < MD5_DIGEST_LENGTH; ++i)
				{
					const char* hex = "0123456789abcdef";

					resultString += hex[(resultBuffer[i] >> 4) & 15];
					resultString += hex[(resultBuffer[i] >> 0) & 15];
				}
			}
		}
	};

	std::string MD5Hasher::convertToLegacyHash(std::string hash)
	{
		std::string result;
		for (int i = 0; i < 32; i += 2)
		{
			if (hash[i] != '0')
				result += hash[i];
			result += hash[i + 1];
		}
		return result;
	}

	MD5Hasher* MD5Hasher::create()
	{
		return new MD5HasherImpl();
	}

	std::string CollectMd5Hash(const std::string& fileName)
	{
		boost::scoped_ptr<RBX::MD5Hasher> hasher(RBX::MD5Hasher::create());
		// TODO: 2016 decodes via utf8_decode (StringConv.h, absent here).
		std::ifstream input(fileName.c_str(), std::ios_base::binary);
		hasher->addData(input);
		return hasher->toString();
	}

	std::string ComputeMd5Hash(const std::string& data)
	{
		MD5HasherImpl impl;
		impl.addData(data);
		return impl.toString();
	}

} //namespace
