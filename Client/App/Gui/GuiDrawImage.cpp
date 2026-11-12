/* App/gui/GuiDrawImage.cpp -- October 2013 (image-draw member bodies). */

#include "stdafx.h"

#include "Gui/GuiDrawImage.h"

#include <cstring>

namespace RBX {

	// mac: 0x3A671A (batch338: stores + setImageFromName tail).
	GuiDrawImage::GuiDrawImage(Adorn* adorn, const std::string& name, unsigned int unknown)
		: name1(Name::getNullName())
		, name3(Name::getNullName())
	{
		memset(reserved, 0, sizeof(reserved));
		setImageFromName(adorn, name, unknown);
	}

	// mac: 0x5D0CFE (batch339: "Textures/"+name+".png" -> fromAssets -> setImage).
	// STUB: PLAYER 0x008b74d0
	// STUB: STUDIO 0x009c8870
	bool GuiDrawImage::setImageFromName(Adorn* adorn, const std::string& name, unsigned int unknown)
	{
		std::string path = "Textures/" + name + ".png";
		ContentId contentId = ContentId::fromAssets(path.c_str());
		return setImage(adorn, contentId, unknown, NULL);
	}

	// mac: 0x5CFADE STUB (render era). 4th param = G3D::Vector2* per the
	// binary mangle PN3G3D7Vector2E (batch791); was void* unknown2.
	// STUB: PLAYER 0x008b6dc0
	// STUB: STUDIO 0x009c8160
	bool GuiDrawImage::setImage(Adorn* adorn, const TextureId& textureId, unsigned int unknown1, G3D::Vector2* unknown2)
	{
		(void)adorn; (void)textureId; (void)unknown1; (void)unknown2;
		RBXASSERT(false && "GuiDrawImage::setImage: texture-proxy render era");
		return false;
	}

} // namespace RBX
