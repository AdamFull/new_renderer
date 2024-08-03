#include "ViewportWindow.h"

#include <imgui.h>

using namespace engine;

void CViewportWindow::Create()
{

}

void CViewportWindow::__draw(float fDt)
{
	auto content_size = ImGui::GetContentRegionAvail();

	ImVec2 viewportPanelPos = ImGui::GetWindowPos();
	auto textDrawPos = ImGui::GetCursorStartPos();

	DrawViewport();

	auto controlls_enable = ImGui::IsWindowHovered(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsWindowFocused();

	//if (auto camera = EGEngine->getActiveCamera())
	//	camera->controllable = controlls_enable;

	//if (EGEngine->isEditorEditing())
	//{
	//	ViewportPicking();
	//	DrawManipulator(viewportPanelPos.x, viewportPanelPos.y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());
	//}

	DrawOverlay(textDrawPos.x, textDrawPos.y, fDt);
}

void CViewportWindow::DrawViewport()
{
	//ImGui::Image(pDescriptorSet->get(), ImVec2{ viewportPanelSizeX, viewportPanelSizeY });

	if (!ImGui::IsAnyItemActive());
	//	device->setViewportExtent(vk::Extent2D{ (uint32_t)viewportPanelSizeX, (uint32_t)viewportPanelSizeY });
}

void CViewportWindow::ViewportPicking()
{

}

void CViewportWindow::DrawManipulator(float offsetx, float offsety, float sizex, float sizey)
{
}

void CViewportWindow::DrawOverlay(float offsetx, float offsety, float fDt)
{
}