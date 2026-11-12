/* gui/EquationDisplay.h -- October 2013 C++-only widget (NO reflection).
   Evidence: N; C2(title,equation) 0x5CBF66 calls TextDisplay(this,title,title) (label:=title); equation at +160; 2-arg ONLY (2016 3-arg absent); getLabel 0x5CC03C.
   TODO: full decompile (member map + init/render/process bodies).
   Original file: /Applications/buildAgent/work/fa0a2141fe3d2f4d/Client/App/gui/EquationDisplay.h */

#pragma once

#include "gui/TextDisplay.h"
#include <string>

namespace RBX {

	class EquationDisplay
		: public TextDisplay
	{
	public:
		EquationDisplay(const std::string& title, const std::string& equation)
			: TextDisplay(title, title), equationText(equation)
		{}
	private:
		std::string equationText;
	};

} // namespace RBX
