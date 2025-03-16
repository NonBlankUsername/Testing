#pragma once
#include "../keybinds.hpp"
#include "ImGui/imgui.h"

namespace Globals {
	namespace Interface {
		inline bool Open = true;
	}

	namespace ESP {
		inline bool Enabled = false;

		inline bool Box = false;
		inline bool FilledBox = false;
		inline bool Skeleton = false;

		inline bool Name = false;
		inline bool Distance = false;
		inline bool Health = false;
		inline bool HealthText = false;
		inline bool Lines = false;

		

		extern ImVec4 BoxColor;
		extern ImVec4 LinesColor;
		extern ImVec4 FilledBoxColor;
		extern ImVec4 NameColor;
		extern ImVec4 HealthTextColor;

	}

	namespace Aimbot {
		inline bool Enabled = false;
	}

	namespace Misc {
		inline bool TeamCheck = false;
		inline bool Bhop = false;
	}
}