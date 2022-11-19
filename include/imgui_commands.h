#pragma once
#include <functional>
#include "imgui.h"

#define ADD_IMGUI_DRAW_TOGGLE(TOGGLE_NAME) static bool bDrawTOGGLE_NAME = false;	\
OOT_Imgui_Commands::EnqueueCheckbox("Toggle " #TOGGLE_NAME, &bDrawTOGGLE_NAME);				\
if (bDrawTOGGLE_NAME)																\
{																					\
	return;																			\
}

#ifdef WITH_IMGUI
namespace OOT_Imgui_Commands
{
	constexpr const char* const szRenderingToggleWindowName = "Rendering";

	void EnqueueImguiCommand(std::function<void(void)> newCommand);

	inline void EnqueueCheckbox(const char* szTextLabel, bool* pBoolPointer)
	{
		EnqueueImguiCommand([=]
			{
				ImGui::Begin(szRenderingToggleWindowName, nullptr);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
				ImGui::Checkbox(szTextLabel, pBoolPointer);
				ImGui::End();
			});
	}
}

#endif