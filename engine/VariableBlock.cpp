#include "VariableBlock.h"

using namespace engine;

void CVariableBlock::display()
{
    // Draw variables
    for (auto& [name, var] : m_variables)
        var->display();
 
    // Draw blocks
    for (auto& [name, blk] : m_blocks) 
    {
        //if (ImGui::TreeNode(name.c_str())) 
        {
            blk->display();
            //ImGui::TreePop();
        }
    }
}