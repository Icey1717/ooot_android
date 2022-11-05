#pragma once
#include <functional>

#ifdef WITH_IMGUI
void EnqueueImguiCommand(std::function<void(void)> newCommand);
#endif