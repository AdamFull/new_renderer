#include "VariableBlock.h"

using namespace engine;

void CVariableBlock::Display()
{
    // Draw variables
    for (auto& [name, var] : m_variables)
        var->Display();
 
    // Draw blocks
    for (auto& [name, blk] : m_blocks) 
    {
        //if (ImGui::TreeNode(name.c_str())) 
        {
            blk->Display();
            //ImGui::TreePop();
        }
    }
}