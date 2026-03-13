#pragma once

#ifdef _DEBUG

#include "../EterLib/ImGui/imgui.h"

#include "PythonNetworkStream.h"
#include "PythonPlayer.h"
#include "PythonChat.h"
#include "../EterBase/Debug.h"

#include <array>
#include <string_view>

class CMapManager;
class YosunAdminPanel
{
    public:
        explicit YosunAdminPanel(CMapManager& mapMgr);
        ~YosunAdminPanel();

        void Render();
    private:
        void RenderMenuBar();

        void RenderTextMetrics(bool* p_open) const;
        bool show_text_metrics = false;

        void RenderTerrainMetrics(bool* p_open) const;
        bool show_terrain_metrics = false;

        /* - WORLD_EDITOR_IMGUI -------------------------------- */
        void RenderWorldEditor(bool* p_open) const;
        bool show_world_editor = false;
        /* ----------------------------------------------------- */

        /* - SHADER [PANEL] ------------------------------------ */
        void RenderShaderManager(bool* p_open) const;
        bool show_shader_manager = false;
        /* ----------------------------------------------------- */

        void RenderGMCommands(bool* p_open);
        bool show_gm_commands = false;
        void GMCommandsList() const;
        void Warp();

        void RenderDebugOverlay(bool* p_open);
        bool show_debug_overlay = false;
        void DebugOverlayChangePosition(bool* p_open);
        int32_t location = 0;
        void RenderFpsLineGraph() const;

        struct AtlasEntry
        {
            std::string name;
            int         x;  // raw X*100 from AtlasInfo.txt
            int         y;  // raw Y*100
        };
        std::vector<AtlasEntry> m_atlasEntries;
        int                     m_warpIdx = 0;
        void LoadAtlasInfo();

private:
    CMapManager& m_rkMapMgr;
};

#endif /* _DEBUG */
