#include "CameraSystem.h"

#include <world/SceneGraph.h>

using namespace engine::render;
using namespace engine::world::ecs;

CCameraSystem::CCameraSystem(CSceneGraph* scenegraph, IRenderInterface* render_interface)
{
	m_pSceneGraph = scenegraph;
	m_pRenderInterface = render_interface;
}

void CCameraSystem::__create()
{

}

void CCameraSystem::__update(float fDt)
{

}
