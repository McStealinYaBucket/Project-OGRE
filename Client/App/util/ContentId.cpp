// ContentId.cpp - reconstructed October 2013 (Studio 0.126.0.39503 Mach-O)
// Evidence EAs from idalib decompilation; style follows 2016 reference source.
#include "stdafx.h"

#include "Util/ContentId.h"
#include "Util/LegacyContentTable.h"
#include "Reflection/Type.h"
#include <boost/functional/hash.hpp>
#include <algorithm>
#include <cctype>

namespace RBX {

// Type singleton for property-reflection + SerializerV2 ContentId-element check
// (parse() compares element tags against Type::getSingleton<ContentId>().tag).
// Follows the Region3.cpp TType pattern.
namespace Reflection
{
	template<>
	const Type& Type::getSingleton<ContentId>()
	{
		static TType<ContentId> type("ContentId");
		return type;
	}
}

} // namespace RBX

namespace
{
	const char* kValidAssetPaths[] =
	{
		// assets
		"asset",
		"asset/",
		"asset/bodycolors.ashx",
		// thumbnails
		"thumbs/asset.ashx",
		"thumbs/avatar.ashx",
		"thumbs/script.png",
		"thumbs/staticimage",
		"game/tools/thumbnailasset.ashx",
		// scripts
		"game/edit.ashx",
		"game/gameserver.ashx",
		"game/join.ashx",
		"game/visit.ashx"
	};

	// helper inlined at 0x1F8A8F in the binary; createIdUrl(result, baseUrl, id):
	// reserve(baseUrl.size() + id.size() + 16); assign baseUrl; append '/' if needed;
	// append "asset/?id="; append id.
	// FUNCTION: PLAYER 0x0074eb50
	// FUNCTION: STUDIO 0x00779b30
	void createIdUrl(std::string& result, const std::string& baseUrl, const std::string& id)
	{
		result.reserve(baseUrl.size() + id.size() + 16);

		result = baseUrl;

		// append slash only if baseUrl does not end with one
		if (result.empty() || result[result.size() - 1] != '/')
			result += '/';

		result += "asset/?id=";
		result += id;
	}
}

namespace RBX
{
	// static LegacyContentTable instance referenced directly at 0x1F8B2C:
	// RBX::LegacyContentTable::FindEntry(&RBX::ContentId::legacyContentTable, this)
	static LegacyContentTable legacyContentTable;

	bool operator<(const ContentId& a, const ContentId& b) {	// 0x1F87C6
		return a.id < b.id;
	}
	bool operator!=(const ContentId& a, const ContentId& b) {	// 0x1F87E6
		return a.id != b.id;
	}
	bool operator==(const ContentId& a, const ContentId& b) {	// 0x1F880B
		return a.id == b.id;
	}

	ContentId ContentId::fromUrl(const std::string& url) {	// 0x1F8830
		// TODO: Throw if string isn't a real Url?
		return ContentId(url);
	}

	void ContentId::CorrectBackslash(std::string& id)	// 0x1F8850
	{
		for (size_t i = 0; i < id.size(); ++i)
			if (id[i] == '\\')
				id[i] = '/';
	}

	// FUNCTION: PLAYER 0x0074ec00
	// FUNCTION: STUDIO 0x00779be0
	void ContentId::convertAssetId(const std::string& baseUrl)	// 0x1F889E
	{
		if (isAssetId())
			createIdUrl(id, baseUrl, id.substr(13));
		else if (isRbxHttp()){
			id = baseUrl + id.substr(10);
		}
	}

	// STUB: PLAYER 0x0074ecf0
	// STUB: STUDIO 0x00779cd0
	void ContentId::convertToLegacyContent(const std::string& baseUrl)	// 0x1F8B2C
	{
		if (isAsset())
		{
			const std::string& mappedAssetId = legacyContentTable.FindEntry(id);

			if (!mappedAssetId.empty())
			{
				if (isdigit(mappedAssetId[0]))
					createIdUrl(id, baseUrl, mappedAssetId);
				else
					id = mappedAssetId;
			}
		}
	}

	std::string ContentId::getAssetId() const	// 0x1F8BB4
	{
		if (isAssetId()){
			return id.substr(13);
		}
		else if (isHttp() || isRbxHttp()){
			std::string lower = id;
			std::transform(lower.begin(), lower.end(), lower.begin(), tolower);
			std::string::size_type pos = lower.find("id=");
			if (pos != std::string::npos){
				pos += 3;
				std::string::size_type second_pos = lower.find('&', pos);
				if (second_pos == std::string::npos){
					second_pos = lower.size();
				}
				return lower.substr(pos, (second_pos - pos));
			}
		}
		return "";
	}

	// FUNCTION: PLAYER 0x0074ed80
	// FUNCTION: STUDIO 0x00779d60
	ContentId ContentId::fromAssets(const char* filePath) {	// 0x1F8E0A
		std::string header("rbxasset://");
		return ContentId(header + filePath);
	}

	std::size_t hash_value(const ContentId& id)
	{
		boost::hash<std::string> hasher;
		return hasher(id.toString());
	}
}

// TODO (not reconstructed here; belong to other modules / not present in 2013 binary):
//  - fromGameAssetName / getAssetName / getUnConvertedAssetName / isNamedAsset /
//    isConvertedNamedAsset: no symbols in the Oct 2013 Studio or Mobile binaries
//    (rbxgameasset:// named-universe asset support postdates 2013).
//  - reconstructUrl / reconstructAssetUrl: not present as symbols in 2013 binary
//    (possibly fully inlined at call sites); kept on header per 2016 template with
//    empty definition omitted. Wire up if a call site is identified.
//  - RBX::operator<<(RakNet::BitStream&, const ContentId&) (0x83DBF1) and
//    RBX::operator>><ContentId> (0x83DC12) live in the Network module.
