// reconstructed from Mach-O Studio 0.126.0.39503 (binary wins; style from 2016 reference)
#pragma once

#include <string>
#include <cstddef>
#include "util/Name.h"

namespace RBX
{

	class ContentId
	{
	public:
		static ContentId fromUrl(const std::string& url);							// 0x1F8830
		static ContentId fromAssets(const char* filePath);	// filePath is a relative pathname within the "Content" directory	// 0x1F8E0A

		// Constructors are explicit to encourage you to use static constructors above if the string isn't a fully qualified URL
		explicit ContentId(const char* id)						// 0x1A92E0
			:id(id), name(&Name::getNullName()) { CorrectBackslash(this->id); }
		explicit ContentId(const std::string& id)				// 0x32E020
			:id(id), name(&Name::getNullName()) { CorrectBackslash(this->id); }
		ContentId(const std::string& id, const Name& name)		// 0x5BEC7E
			:id(id), name(&name) { CorrectBackslash(this->id); }
		ContentId()												// 0x5D1A10
			:id(), name(&Name::getNullName()) {}

		void clear()
		{
			id.clear();
		}

		const char* c_str() const {
			return id.c_str();
		}
		const std::string& toString() const {
			return id;
		}

		void convertToLegacyContent(const std::string& baseUrl);	// 0x1F8B2C
		void convertAssetId(const std::string& baseUrl);			// 0x1F889E
		bool reconstructUrl(const std::string& baseUrl, const char* const paths[], const int pathCount);
		bool reconstructAssetUrl(const std::string& baseUrl);

		std::string getAssetId() const;		// 0x1F8BB4

		bool isNull() const { return id.size()==0; }
		bool isAsset() const { return id.compare(0, 11, "rbxasset://") == 0; }
		bool isAssetId() const { return id.compare(0, 13, "rbxassetid://") == 0; }
		bool isHttp() const { return id.compare(0, 4, "http") == 0; }
		bool isFile() const { return id.compare(0, 7, "file://") == 0; }
		bool isRbxHttp() const { return id.compare(0, 10, "rbxhttp://") == 0; }

		friend bool operator<(const ContentId& a, const ContentId& b);		// 0x1F87C6
		friend bool operator==(const ContentId& a, const ContentId& b);		// 0x1F880B

		// binary: copy ctor + operator= used at 0x83DC12 etc. (member-wise copy of id + name)
		ContentId& operator=(const ContentId& other)
		{
			const_cast<std::string&>(id) = other.id;
			name = other.name;
			return *this;
		}
		friend bool operator!=(const ContentId& a, const ContentId& b);		// 0x1F87E6

	private:
		static void CorrectBackslash(std::string& id);						// 0x1F8850

		std::string id;
		const Name* name;	// +4 in binary layout: every ctor stores getNullName()/arg POINTER (0x32E020, 0x5BEC7E, 0x5D1A10)
	};

	std::size_t hash_value(const ContentId& id);

	// GuiImageButton::setImage 0x3F4F90 takes RBX::TextureId but operates on
	// it with ContentId ops (operator!= + assign + Name* copy at +4) --
	// layout-identical ({string, Name*}), so TextureId aliases ContentId.
	// Offset-0 corroboration (ChatOutput arc, Oct 2013 client): the four
	// bubble factories call __ZN3RBX9ContentIdC2ERKSs on the SAME temp
	// (0x5C1192/0x5C1707/0x5C1C01/0x5C1DDB) they then push to the
	// vtable+0xC4 slot (ImageLabel::setImage(TextureId) 0x3F6A4A), and no
	// out-of-line TextureId ctors exist in the symbol table.
	typedef ContentId TextureId;

}// namespace
