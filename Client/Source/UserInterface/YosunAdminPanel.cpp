#include "StdAfx.h"
#include "YosunAdminPanel.h"

#include "../SphereLib/YosunControlCenter.h"
#include "PythonBackground.h"
#include "../EterPack/EterPackManager.h"

/* - SHADER [PANEL] ------------------------------------ */
#include "../EterLib/ShaderParameters.h"
/* ----------------------------------------------------- */

#include <Psapi.h>

#include <utility>
#include "../GameLib/MapManager.h"

#ifdef _DEBUG

static void ApplyYosunAdminTheme()
{
    ImGuiStyle& style = ImGui::GetStyle();

    // -------------------------------------------------
    // Layout & Spacing
    // -------------------------------------------------
    style.WindowPadding = ImVec2(14, 14);
    style.FramePadding = ImVec2(10, 6);
    style.ItemSpacing = ImVec2(10, 8);
    style.ItemInnerSpacing = ImVec2(8, 6);
    style.ScrollbarSize = 14.0f;
    style.GrabMinSize = 12.0f;

    // -------------------------------------------------
    // Rounding
    // -------------------------------------------------
    style.WindowRounding = 10.0f;
    style.FrameRounding = 6.0f;
    style.PopupRounding = 8.0f;
    style.ScrollbarRounding = 12.0f;
    style.GrabRounding = 6.0f;
    style.TabRounding = 8.0f;

    ImVec4* c = style.Colors;

    // -------------------------------------------------
    // Backgrounds
    // -------------------------------------------------
    c[ImGuiCol_WindowBg] = ImVec4(0.07f, 0.08f, 0.09f, 0.98f);
    c[ImGuiCol_ChildBg] = ImVec4(0.06f, 0.07f, 0.08f, 0.94f);
    c[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.06f, 0.07f, 0.98f);

    // -------------------------------------------------
    // Text
    // -------------------------------------------------
    c[ImGuiCol_Text] = ImVec4(0.88f, 0.90f, 0.92f, 1.00f);
    c[ImGuiCol_TextDisabled] = ImVec4(0.52f, 0.58f, 0.60f, 1.00f);

    // -------------------------------------------------
    // Title Bars (THE IMPORTANT PART)
    // -------------------------------------------------
    // Inactive: mossy, desaturated green — NOT black
    c[ImGuiCol_TitleBg] =
        ImVec4(0.10f, 0.18f, 0.16f, 1.00f);

    // Active: emerald authority
    c[ImGuiCol_TitleBgActive] =
        ImVec4(0.12f, 0.35f, 0.30f, 1.00f);

    // Collapsed: same family, lighter & calmer
    c[ImGuiCol_TitleBgCollapsed] =
        ImVec4(0.11f, 0.20f, 0.18f, 0.90f);

    // -------------------------------------------------
    // Borders & Separators
    // -------------------------------------------------
    c[ImGuiCol_Border] = ImVec4(0.14f, 0.32f, 0.28f, 0.30f);
    c[ImGuiCol_Separator] = ImVec4(0.16f, 0.36f, 0.32f, 0.35f);
    c[ImGuiCol_SeparatorHovered] = ImVec4(0.25f, 0.70f, 0.55f, 0.80f);
    c[ImGuiCol_SeparatorActive] = ImVec4(0.30f, 0.85f, 0.65f, 1.00f);

    // -------------------------------------------------
    // Frames (inputs, sliders, checkboxes)
    // -------------------------------------------------
    c[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.12f, 0.14f, 1.00f);
    c[ImGuiCol_FrameBgHovered] = ImVec4(0.16f, 0.40f, 0.34f, 0.80f);
    c[ImGuiCol_FrameBgActive] = ImVec4(0.18f, 0.55f, 0.45f, 1.00f);

    // -------------------------------------------------
    // Buttons
    // -------------------------------------------------
    c[ImGuiCol_Button] = ImVec4(0.14f, 0.35f, 0.30f, 0.70f);
    c[ImGuiCol_ButtonHovered] = ImVec4(0.18f, 0.55f, 0.45f, 0.90f);
    c[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.75f, 0.55f, 1.00f);

    // -------------------------------------------------
    // Headers (menus, collapsing sections)
    // -------------------------------------------------
    c[ImGuiCol_Header] = ImVec4(0.16f, 0.45f, 0.36f, 0.55f);
    c[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.65f, 0.50f, 0.75f);
    c[ImGuiCol_HeaderActive] = ImVec4(0.22f, 0.80f, 0.60f, 1.00f);

    // -------------------------------------------------
    // Tabs
    // -------------------------------------------------
    c[ImGuiCol_Tab] = ImVec4(0.10f, 0.25f, 0.22f, 0.85f);
    c[ImGuiCol_TabHovered] = ImVec4(0.18f, 0.55f, 0.45f, 0.90f);
    c[ImGuiCol_TabActive] = ImVec4(0.20f, 0.70f, 0.55f, 1.00f);
    c[ImGuiCol_TabUnfocused] = ImVec4(0.11f, 0.22f, 0.20f, 0.75f);
    c[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.36f, 0.32f, 0.85f);

    // -------------------------------------------------
    // Scrollbars
    // -------------------------------------------------
    c[ImGuiCol_ScrollbarBg] = ImVec4(0.04f, 0.05f, 0.06f, 0.90f);
    c[ImGuiCol_ScrollbarGrab] = ImVec4(0.18f, 0.45f, 0.38f, 0.70f);
    c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.22f, 0.65f, 0.55f, 0.90f);
    c[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.25f, 0.80f, 0.65f, 1.00f);

    // -------------------------------------------------
    // Plots
    // -------------------------------------------------
    c[ImGuiCol_PlotLines] = ImVec4(0.55f, 0.85f, 0.70f, 0.90f);
    c[ImGuiCol_PlotLinesHovered] = ImVec4(0.80f, 1.00f, 0.90f, 1.00f);
}

YosunAdminPanel::YosunAdminPanel(CMapManager& mapMgr)
    : m_rkMapMgr(mapMgr)
{
    ApplyYosunAdminTheme();
    LoadAtlasInfo();
}

YosunAdminPanel::~YosunAdminPanel() = default;

void YosunAdminPanel::Render()
{
    constexpr char kMainPanelName[] = "Yosun Admin Panel";

    ImGuiWindowFlags mainFlags =
        ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_AlwaysAutoResize;

    if (ImGui::Begin(kMainPanelName, nullptr, mainFlags))
    {
        // -------------------------------------------------
        // Menu bar
        // -------------------------------------------------
        RenderMenuBar();

        ImGui::Spacing();
        ImGui::Spacing();

        // -------------------------------------------------
        // Hero header (centered)
        // -------------------------------------------------
        {
            const char* title = "YOSUN ADMIN PANEL";
            const char* subtitle = "Kaptan Yosun @ mmotutkunlari.com";

            // Center block horizontally
            float windowWidth = ImGui::GetWindowSize().x;

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 6));

            // Title
            float titleWidth = ImGui::CalcTextSize(title).x;
            ImGui::SetCursorPosX((windowWidth - titleWidth) * 0.5f);
            ImGui::TextUnformatted(title);

            // Subtitle
            float subtitleWidth = ImGui::CalcTextSize(subtitle).x;
            ImGui::SetCursorPosX((windowWidth - subtitleWidth) * 0.5f);
            ImGui::TextDisabled("%s", subtitle);

            ImGui::PopStyleVar();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // -------------------------------------------------
        // System / meta information row
        // -------------------------------------------------
        {
            ImGui::BeginGroup();

            ImGui::TextDisabled("Engine");
            ImGui::SameLine();
            ImGui::TextUnformatted("Metin2 Client");

            ImGui::TextDisabled("UI");
            ImGui::SameLine();
            ImGui::Text("Dear ImGui %s", ImGui::GetVersion());

            ImGui::EndGroup();
        }

        ImGui::Spacing();
    }
    ImGui::End();

    // -------------------------------------------------
    // Sub-windows
    // -------------------------------------------------
    if (show_text_metrics)
    {
        RenderTextMetrics(&show_text_metrics);
    }
    if (show_terrain_metrics)
    {
        RenderTerrainMetrics(&show_terrain_metrics);
    }

    if (show_world_editor)
    {
        RenderWorldEditor(&show_world_editor);
    }

    /* - SHADER [PANEL] ------------------------------------ */
    if (show_shader_manager)
    {
        RenderShaderManager(&show_shader_manager);
    }
    /* ----------------------------------------------------- */

    if (show_gm_commands)
    {
        RenderGMCommands(&show_gm_commands);
    }
    if (show_debug_overlay)
    {
        RenderDebugOverlay(&show_debug_overlay);
    }
}

void YosunAdminPanel::RenderMenuBar()
{
    if (!ImGui::BeginMenuBar())
    {
        return;
    }

    if (ImGui::BeginMenu("Options"))
    {
        ImGui::SeparatorText("FPS Graphs & Debug");
        ImGui::MenuItem("Debug Overlay", nullptr, &show_debug_overlay);
        ImGui::MenuItem("Syserr & Syslog", nullptr);

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Tools"))
    {
        ImGui::SeparatorText("Metrics");
        ImGui::MenuItem("Text Metrics", nullptr, &show_text_metrics);
        ImGui::MenuItem("Terrain Metrics", nullptr, &show_terrain_metrics);

        ImGui::SeparatorText("Map-Related");
        ImGui::MenuItem("World Editor", nullptr, &show_world_editor);

        /* - SHADER [PANEL] ------------------------------------ */
        ImGui::SeparatorText("Graphics");
        ImGui::MenuItem("Shader Manager", nullptr, &show_shader_manager);
        /* ----------------------------------------------------- */

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("GM"))
    {
        ImGui::SeparatorText("Quick Commands");
        ImGui::MenuItem("GM Commands", nullptr, &show_gm_commands);

        ImGui::EndMenu();
    }

    ImGui::EndMenuBar();
}

void YosunAdminPanel::RenderTextMetrics(bool* p_open) const
{
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_AlwaysAutoResize;

    auto& tm = GetYosunControlSettings().textMetrics;

    if (!ImGui::Begin("Text Rendering Metrics", p_open, flags))
    {
        ImGui::End();
        return;
    }

    ImGui::SeparatorText("Frame Metrics");

    float fps = ImGui::GetIO().Framerate;
    float frameMs = 1000.0f / (fps > 0.0f ? fps : 1.0f);

    ImGui::Text("FPS");
    ImGui::SameLine(200);
    ImGui::Text("%.1f", fps);

    ImGui::Text("Frame Time (ms)");
    ImGui::SameLine(200);
    ImGui::Text("%.3f", frameMs);

    ImGui::Spacing();
    ImGui::Separator();

    ImGui::SeparatorText("Text Rendering");

    ImGui::Text("Text Instances");
    ImGui::SameLine(200);
    ImGui::Text("%llu", tm.totalTextInstances);

    ImGui::Text("Total Characters");
    ImGui::SameLine(200);
    ImGui::Text("%llu", tm.totalChars);

    ImGui::Text("Draw Calls");
    ImGui::SameLine(200);
    ImGui::Text("%llu", tm.totalDrawCalls);

    ImGui::Spacing();
    ImGui::SeparatorText("Derived");

    float drawsPerChar = tm.totalChars > 0 ? float(tm.totalDrawCalls) / float(tm.totalChars) : 0.0f;
    float charsPerInstance = tm.totalTextInstances > 0 ? float(tm.totalChars) / float(tm.totalTextInstances) : 0.0f;

    ImGui::Text("Draws / Character");
    ImGui::SameLine(200);
    ImGui::Text("%.2f", drawsPerChar);

    ImGui::Text("Chars / Instance");
    ImGui::SameLine(200);
    ImGui::Text("%.2f", charsPerInstance);

    ImGui::Spacing();
    ImGui::SeparatorText("Peaks (Since Launch)");

    ImGui::Text("Peak Instances");
    ImGui::SameLine(200);
    ImGui::Text("%llu", tm.peakTextInstances);

    ImGui::Text("Peak Characters");
    ImGui::SameLine(200);
    ImGui::Text("%llu", tm.peakChars);

    ImGui::Text("Peak Draw Calls");
    ImGui::SameLine(200);
    ImGui::Text("%llu", tm.peakDrawCalls);

    ImGui::Spacing();
    ImGui::SeparatorText("Controls");

    ImGui::Checkbox("Disable Outline (Global)", &tm.forceDisableOutline);

    ImGui::Spacing();
    ImGui::Separator();

    if (ImGui::Button("Reset Counters"))
    {
        tm.totalTextInstances = YosunControlDefaults::TextMetrics::TotalTextInstances;
        tm.totalChars         = YosunControlDefaults::TextMetrics::TotalChars;
        tm.totalDrawCalls     = YosunControlDefaults::TextMetrics::TotalDrawCalls;
        tm.peakTextInstances  = YosunControlDefaults::TextMetrics::PeakTextInstances;
        tm.peakChars          = YosunControlDefaults::TextMetrics::PeakChars;
        tm.peakDrawCalls      = YosunControlDefaults::TextMetrics::PeakDrawCalls;
    }

    ImGui::End();
}

void YosunAdminPanel::RenderTerrainMetrics(bool* p_open) const
{
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_AlwaysAutoResize;

    auto& tm = GetYosunControlSettings().terrainMetrics;

    if (!ImGui::Begin("Terrain Rendering Metrics", p_open, flags))
    {
        ImGui::End();
        return;
    }

    ImGui::SeparatorText("Frame Metrics");

    float fps = ImGui::GetIO().Framerate;
    float frameMs = 1000.0f / (fps > 0.0f ? fps : 1.0f);

    ImGui::Text("FPS");
    ImGui::SameLine(200);
    ImGui::Text("%.1f", fps);

    ImGui::Text("Frame Time (ms)");
    ImGui::SameLine(200);
    ImGui::Text("%.3f", frameMs);

    ImGui::Spacing();
    ImGui::Separator();

    ImGui::SeparatorText("Terrain Rendering");

    ImGui::Text("Rendered Patches");
    ImGui::SameLine(200);
    ImGui::Text("%llu", tm.renderedPatches);

    ImGui::Text("Rendered Splats");
    ImGui::SameLine(200);
    ImGui::Text("%llu", tm.renderedSplats);

    ImGui::Text("Draw Calls");
    ImGui::SameLine(200);
    ImGui::Text("%llu", tm.totalDrawCalls);

    float splatsPerPatch = tm.renderedPatches > 0 ? float(tm.renderedSplats) / float(tm.renderedPatches) : 0.0f;

    ImGui::Text("Splats / Patch");
    ImGui::SameLine(200);
    ImGui::Text("%.2f", splatsPerPatch);

    ImGui::Spacing();
    ImGui::SeparatorText("Peaks (Since Launch)");

    ImGui::Text("Peak Draw Calls");
    ImGui::SameLine(200);
    ImGui::Text("%llu", tm.peakDrawCalls);

    ImGui::Spacing();
    ImGui::Separator();

    if (ImGui::Button("Reset Counters"))
    {
        tm.totalDrawCalls = YosunControlDefaults::TerrainMetrics::TotalDrawCalls;
        tm.peakDrawCalls = YosunControlDefaults::TerrainMetrics::PeakDrawCalls;
    }

    ImGui::End();
}

void YosunAdminPanel::RenderWorldEditor(bool* p_open) const
{
    constexpr char kWorldEditorTitle[] = "World Editor";
    if (ImGuiWindowFlags worldEditorFlags = ImGuiWindowFlags_None;
        ImGui::Begin(kWorldEditorTitle, p_open, worldEditorFlags))
    {
        {
            ImGui::SeparatorText("WorldEditor");
            {
                auto& ycc = GetYosunControlSettings().worldEditor;
                {
                    ImGui::Checkbox("Collision", &ycc.drawCollision);
                    ImGui::SameLine(0.0f, 12.0f);

                    const bool enabled = ycc.drawCollision;

                    ImGui::PushStyleColor(ImGuiCol_Text, enabled ? ImVec4(0.2f, 1.0f, 0.2f, 1.0f) : ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
                    ImGui::Text("%s", enabled ? "ON" : "OFF");
                    ImGui::PopStyleColor();
                }

                auto& ycct = ycc.terrain;
                {
                    /* - YOSUN_CONTROL_CENTER [Terrain Wireframe] ---------- */
                    ImGui::Checkbox("Terrain Wireframe", &ycct.drawTerrainWireFrame);
                    ImGui::SameLine(0.0f, 12.0f);
                    const bool enabled = ycct.drawTerrainWireFrame;
                    ImGui::PushStyleColor(ImGuiCol_Text, enabled ? ImVec4(0.2f, 1.0f, 0.2f, 1.0f) : ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
                    ImGui::Text("%s", enabled ? "ON" : "OFF");
                    ImGui::PopStyleColor();
                    /* ----------------------------------------------------- */
                }
                {
                    /* - YOSUN_CONTROL_CENTER [Water Wireframe] ------------ */
                    ImGui::Checkbox("Water Wireframe", &ycct.drawWaterWireFrame);
                    ImGui::SameLine(0.0f, 12.0f);
                    const bool enabled = ycct.drawWaterWireFrame;
                    ImGui::PushStyleColor(ImGuiCol_Text, enabled ? ImVec4(0.2f, 1.0f, 0.2f, 1.0f) : ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
                    ImGui::Text("%s", enabled ? "ON" : "OFF");
                    ImGui::PopStyleColor();
                    /* ----------------------------------------------------- */
                }
                {
                    /* - YOSUN_CONTROL_CENTER [Patch Grid] ------------------ */
                    ImGui::Checkbox("Patch Grid", &ycct.drawPatchGrid);
                    ImGui::SameLine(0.0f, 12.0f);
                    const bool enabled = ycct.drawPatchGrid;
                    ImGui::PushStyleColor(ImGuiCol_Text, enabled ? ImVec4(0.2f, 1.0f, 0.2f, 1.0f) : ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
                    ImGui::Text("%s", enabled ? "ON" : "OFF");
                    ImGui::PopStyleColor();
                    /* ----------------------------------------------------- */
                }
            }
        }
        {
            auto& ycc = GetYosunControlSettings().worldEditor.speedTree;
            ImGui::SeparatorText("SpeedTree");

            ImGui::Text("Wind Strength");
            ImGui::SliderFloat("##WindStrengthSlider", &ycc.windStrength, 0.0f, 400.0f);
            ImGui::SameLine();
            ImGui::PushID("WindStrengthResetButton");
            if (ImGui::Button("Reset"))
            {
                ycc.windStrength = YosunControlDefaults::WorldEditor::SpeedTree::WindStrength;
            }
            ImGui::PopID(); /* WindStrengthSlider */

            ImGui::Text("Wind Oscillation Speed");
            ImGui::SliderFloat("##WindOscillationSpeedSlider", &ycc.windOscillationSpeed, 0.0f, 200.0f);
            ImGui::SameLine();
            ImGui::PushID("WindOscillationSpeedResetButton");
            if (ImGui::Button("Reset"))
            {
                ycc.windOscillationSpeed = YosunControlDefaults::WorldEditor::SpeedTree::WindOscillationSpeed;
            }
            ImGui::PopID(); /* WindOscillationSpeedResetButton */
        }

        {
            ImGui::SeparatorText("Environment");

            CMapOutdoor& rkMap = m_rkMapMgr.GetMapOutdoorRef();
            TEnvironmentData* env = rkMap.GetEnvironmentData();
            if (!env) return;

            // -------------------------------------------------
            // Directional Light
            // -------------------------------------------------
            if (ImGui::CollapsingHeader("Directional Light", ImGuiTreeNodeFlags_DefaultOpen))
            {
                bool dirLightEnabled =
                    (env->bDirLightsEnable[ENV_DIRLIGHT_BACKGROUND] != 0);

                if (ImGui::Checkbox("Enable", &dirLightEnabled))
                {
                    env->bDirLightsEnable[ENV_DIRLIGHT_BACKGROUND] =
                        dirLightEnabled ? 1 : 0;
                }

                {
                    ImGui::Separator();
                    ImGui::Text("Sun Direction");

                    static float sunX = 0.0f;   // internal normalized [-1..1]
                    static float sunY = 0.5f;   // internal normalized [-1..1]

                    const float size = 140.0f;
                    const float radiusFactor = 0.45f;

                    ImDrawList* draw = ImGui::GetWindowDrawList();

                    ImVec2 pos = ImGui::GetCursorScreenPos();
                    ImVec2 center = ImVec2(
                        pos.x + size * 0.5f,
                        pos.y + size * 0.5f
                    );

                    float radius = size * radiusFactor;

                    ImGui::InvisibleButton("##SunDome", ImVec2(size, size));
                    bool hovered = ImGui::IsItemHovered();

                    draw->AddCircle(center, radius, IM_COL32(180, 180, 180, 255), 64, 2.0f);

                    // Cross guides (horizontal + vertical)
                    draw->AddLine(ImVec2(center.x - radius, center.y),
                        ImVec2(center.x + radius, center.y),
                        IM_COL32(120, 120, 120, 255));

                    draw->AddLine(ImVec2(center.x, center.y - radius),
                        ImVec2(center.x, center.y + radius),
                        IM_COL32(120, 120, 120, 255));

                    if (hovered && ImGui::IsMouseDown(ImGuiMouseButton_Left))
                    {
                        ImVec2 mouse = ImGui::GetIO().MousePos;

                        float dx = (mouse.x - center.x) / radius;   // right = positive
                        float dy = (center.y - mouse.y) / radius;   // up = positive

                        // Clamp to circle
                        float len = sqrtf(dx * dx + dy * dy);
                        if (len > 1.0f)
                        {
                            dx /= len;
                            dy /= len;
                        }

                        sunX = dx;
                        sunY = dy;
                    }

                    ImVec2 handle = ImVec2(
                        center.x + sunX * radius,
                        center.y - sunY * radius
                    );

                    ImU32 sunColor = (sunY <= 0.0f)
                        ? IM_COL32(255, 200, 50, 255)
                        : IM_COL32(255, 120, 60, 255);

                    draw->AddLine(center, handle, sunColor, 3.0f);
                    draw->AddCircleFilled(handle, 6.0f, sunColor);

                    // -------------------------------------------------
                    // Convert to Direction (Z-Up)
                    // - elevation affects Z
                    // - azimuth rotates in XY around Z
                    // -------------------------------------------------

                    // Elevation (up = positive Z)
                    float elevation = -sunY;

                    // UI-intuitive azimuth:
                    // drag right should rotate right
                    float azimuth = -sunX * D3DX_PI;

                    float planar = sqrtf(fmaxf(0.0f, 1.0f - elevation * elevation));

                    D3DXVECTOR3 dir;
                    dir.z = elevation;
                    dir.x = planar * cosf(azimuth);
                    dir.y = planar * sinf(azimuth);

                    D3DXVec3Normalize(&dir, &dir);

                    env->DirLights[ENV_DIRLIGHT_BACKGROUND].Direction = dir;
                }
                ImGui::Text("Directional Light");
                ImGui::ColorEdit4("Diffuse##BG", &env->DirLights[ENV_DIRLIGHT_BACKGROUND].Diffuse.r, ImGuiColorEditFlags_Float);
                ImGui::ColorEdit4("Ambient##BG", &env->DirLights[ENV_DIRLIGHT_BACKGROUND].Ambient.r, ImGuiColorEditFlags_Float);
            }

            // -------------------------------------------------
            // Material
            // -------------------------------------------------
            if (ImGui::CollapsingHeader("Material"))
            {
                ImGui::ColorEdit4("Diffuse", &env->Material.Diffuse.r, ImGuiColorEditFlags_Float);
                ImGui::ColorEdit4("Ambient", &env->Material.Ambient.r, ImGuiColorEditFlags_Float);
                ImGui::ColorEdit4("Emissive", &env->Material.Emissive.r, ImGuiColorEditFlags_Float);
            }

            // -------------------------------------------------
            // Skybox & Clouds
            // -------------------------------------------------
            if (ImGui::CollapsingHeader("Sky & Clouds"))
            {
                bool skyChanged = false;

                skyChanged |= ImGui::SliderFloat3("Skybox Scale", &env->v3SkyBoxScale.x, 1000.0f, 10000.0f);

                ImGui::Separator();
                ImGui::Text("Clouds");

                skyChanged |= ImGui::SliderFloat2("Scale", &env->v2CloudScale.x, 10000.0f, 500000.0f);
                skyChanged |= ImGui::SliderFloat2("Texture Scale", &env->v2CloudTextureScale.x, 0.1f, 20.0f);
                skyChanged |= ImGui::SliderFloat2("Speed", &env->v2CloudSpeed.x, 0.00f, 0.50f);
                skyChanged |= ImGui::SliderFloat("Height", &env->fCloudHeight, 0.0f, 100000.0f);

                // Cloud tint multiplier (shader PS c0)
                skyChanged |= ImGui::ColorEdit4("Tint Multiplier", &env->cloudTintMultiplier.r, ImGuiColorEditFlags_Float);

                if (skyChanged) rkMap.SetEnvironmentSkyBox();
            }

            // -------------------------------------------------
            // Lens Flare
            // -------------------------------------------------
            if (ImGui::CollapsingHeader("Lens Flare"))
            {
                bool flareChanged = false;

                bool lensEnable = (env->bLensFlareEnable != 0);
                bool mainFlareEnable = (env->bMainFlareEnable != 0);

                flareChanged |= ImGui::Checkbox("Enable Lens Flare", &lensEnable);
                flareChanged |= ImGui::Checkbox("Main Flare", &mainFlareEnable);

                env->bLensFlareEnable = lensEnable ? 1 : 0;
                env->bMainFlareEnable = mainFlareEnable ? 1 : 0;

                flareChanged |= ImGui::ColorEdit4("Brightness Color", &env->LensFlareBrightnessColor.r, ImGuiColorEditFlags_Float);
                flareChanged |= ImGui::SliderFloat("Max Brightness", &env->fLensFlareMaxBrightness, 0.0f, 2.0f);
                flareChanged |= ImGui::SliderFloat("Main Flare Size", &env->fMainFlareSize, 0.0f, 2.0f);

                if (flareChanged) rkMap.SetEnvironmentLensFlare();
            }
        }
    }
    ImGui::End();
}

/* - SHADER [PANEL] ------------------------------------ */
void YosunAdminPanel::RenderShaderManager(bool* p_open) const
{
    constexpr char kShaderManagerTitle[] = "Shader Manager";
    if (ImGui::Begin(kShaderManagerTitle, p_open))
    {
        if (ImGui::BeginTabBar("ShaderManagerTabs"))
        {
            /* ⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘ */
            const float resetButtonPosition = ImGui::GetWindowWidth() - 80;
            /* ⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘ */
            if (ImGui::BeginTabItem("Environment"))
            {
                ImGui::TextDisabled("Global Scene Environment");
                ImGui::Separator();

                auto& es = GetEnvironmentShaderSettings();

                /* ⎯⎯⎯⎯⎯⎯⎯⎯ TIME ⎯⎯⎯⎯⎯⎯⎯⎯ */
                {
                    ImGui::SeparatorText("Time");

                    ImGui::SameLine(resetButtonPosition);
                    if (ImGui::Button("Reset##EnvTime"))
                    {
                        es.runtime.timeSeconds = ShaderDefaults::Environment::TimeSeconds;
                    }

                    ImGui::Text("Environment Time (seconds)");
                    ImGui::InputFloat("##EnvTime", &es.runtime.timeSeconds, 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_ReadOnly);

                    ImGui::TextDisabled("Driven by CPythonApplication::Process()");
                    ImGui::TextDisabled("Resets with CMapOutdoor::Initialize()");
                }

                /* ⎯⎯⎯⎯⎯⎯⎯⎯ WIND ⎯⎯⎯⎯⎯⎯⎯⎯ */
                {
                    ImGui::SeparatorText("Wind");

                    ImGui::SameLine(resetButtonPosition);
                    if (ImGui::Button("Reset##EnvWind"))
                    {
                        using D = ShaderDefaults::Environment;
                        es.authoring.windDirection = D::WindDirection;
                        es.authoring.windStrength = D::WindStrength;
                    }

                    ImGui::Text("Direction");
                    ImGui::DragFloat("X", &es.authoring.windDirection[0], 0.01f, -1.0f, 1.0f);
                    ImGui::DragFloat("Y", &es.authoring.windDirection[1], 0.01f, -1.0f, 1.0f);
                    ImGui::Separator();
                    ImGui::SliderFloat("Strength", &es.authoring.windStrength, 0.0f, 5.0f, "%.2f");

                    ImGui::TextDisabled("Consumed by water, clouds, foliage, etc.");
                }

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Water"))
            {
                ImGui::TextDisabled("Water Shader");
                {
                    ImGui::SeparatorText("Features");

                    auto& wss = GetWaterShaderSettings().features;

                    ImGui::Checkbox("Displacement", &wss.enableDisplacement);
                    ImGui::Checkbox("Mean sky radiance", &wss.enableMeanSky);
                    ImGui::Checkbox("Filtering", &wss.enableFiltering);
                    ImGui::Checkbox("Blinn Phong", &wss.enableBlinnPhong);

                    ImGui::SeparatorText("Debug");
                    static constexpr std::array<const char*, 6> kWaterDebugModes =
                    {
                        "Off",
                        "Normals",
                        "Height",
                        "UVs",
                        "Specular",
                        "Fresnel"
                    };

                    ImGui::Combo("Debug View", &wss.debugView, kWaterDebugModes.data(), static_cast<int>(kWaterDebugModes.size()));
                }
                {
                    ImGui::SeparatorText("Basic settings");

                    auto& wss = GetWaterShaderSettings().basic;

                    ImGui::SameLine(resetButtonPosition);
                    if (ImGui::Button("Reset##Basic"))
                    {
                        using D = ShaderDefaults::Water::Basic;
                        wss.ambientIntensity  = D::AmbientIntensity;
                        wss.diffuseIntensity  = D::DiffuseIntensity;
                        wss.normalIntensity   = D::NormalIntensity;
                        wss.textureTiling     = D::TextureTiling;
                        wss.surfaceColor      = D::SurfaceColor;
                        wss.shoreColor        = D::ShoreColor;
                        wss.deepColor         = D::DeepColor;
                    }

                    ImGui::SliderFloat("Ambient Intensity", &wss.ambientIntensity, 0.0f, 1.0f);
                    ImGui::SliderFloat("Diffuse Intensity", &wss.diffuseIntensity, 0.0f, 1.0f);

                    ImGui::ColorEdit3("Surface Color", wss.surfaceColor.data());
                    ImGui::ColorEdit3("Shore Tint Color", wss.shoreColor.data());
                    ImGui::ColorEdit3("Deep Color", wss.deepColor.data());

                    ImGui::Dummy(ImVec2(0, 8));
                    ImGui::Text("Sky Texture");
                    ImGui::Button("Select##SkyTexture", ImVec2(120, 0));
                    ImGui::Dummy(ImVec2(0, 8));

                    ImGui::Text("Normal Texture");

                    if (wss.normalMapImage && wss.normalMapImage->GetTexturePointer())
                    {
                        ImGui::Image((ImTextureID)wss.normalMapImage->GetTexturePointer()->GetD3DTexture(), ImVec2(64, 64));
                    }
                    else
                    {
                        ImGui::Dummy(ImVec2(64, 64));
                    }

                    ImGui::SameLine();
                    ImGui::TextWrapped("%s", wss.normalMapTexturePath);

                    if (ImGui::Button("Select##NormalTexture", ImVec2(64, 0)))
                    {
                        wss.normalMapTexturePath = ShaderDefaults::Water::Basic::NormalMapTexturePath;
                    }

                    ImGui::SliderFloat("Normal Intensity", &wss.normalIntensity, 0.0f, 1.0f);
                    ImGui::SliderFloat("Texture Tiling", &wss.textureTiling, 0.1f, 10.0f);
                }
                {
                    ImGui::SeparatorText("Displacement settings");

                    auto& wss = GetWaterShaderSettings().displacement;

                    ImGui::SameLine(resetButtonPosition);
                    if (ImGui::Button("Reset##Displacement"))
                    {
                        using D = ShaderDefaults::Water::Displacement;
                        wss.heightIntensity     = D::HeightIntensity;
                        wss.waveTiling          = D::WaveTiling;
                        wss.waveAmplitudeFactor = D::WaveAmplitudeFactor;
                        wss.waveSteepness       = D::WaveSteepness;
                        wss.waveAmplitude       = D::WaveAmplitude;
                        wss.wavesIntensity      = D::WavesIntensity;
                        wss.wavesNoise          = D::WavesNoise;
                    }

                    ImGui::Text("Height Texture");

                    if (wss.heightMapImage && wss.heightMapImage->GetTexturePointer())
                    {
                        ImGui::Image((ImTextureID)wss.heightMapImage->GetTexturePointer()->GetD3DTexture(), ImVec2(64, 64));
                    }
                    else
                    {
                        ImGui::Dummy(ImVec2(64, 64));
                    }

                    ImGui::SameLine();
                    ImGui::TextWrapped("%s", wss.heightMapTexturePath);

                    if (ImGui::Button("Select##HeightTexture", ImVec2(64, 0)))
                    {
                        wss.heightMapTexturePath = ShaderDefaults::Water::Displacement::HeightMapTexturePath;
                    }

                    ImGui::SliderFloat("Height Intensity", &wss.heightIntensity, 0.0f, 1.0f);
                    ImGui::SliderFloat("Wave Tiling", &wss.waveTiling, 0.1f, 10.0f);
                    ImGui::SliderFloat("Wave Amplitude Factor", &wss.waveAmplitudeFactor, 0.0f, 1.0f);
                    ImGui::SliderFloat("Wave Steepness", &wss.waveSteepness, 0.0f, 1.0f);
                    ImGui::DragFloat4("Waves Amplitude", wss.waveAmplitude.data());
                    ImGui::DragFloat4("Waves Intensity", wss.wavesIntensity.data());
                    ImGui::DragFloat4("Waves Noise", wss.wavesNoise.data());
                }
                {
                    ImGui::SeparatorText("Refraction settings");

                    auto& wss = GetWaterShaderSettings().refraction;

                    ImGui::SameLine(resetButtonPosition);
                    if (ImGui::Button("Reset##Refraction"))
                    {
                        using D = ShaderDefaults::Water::Refraction;
                        wss.waterClarity         = D::WaterClarity;
                        wss.waterTransparency    = D::WaterTransparency;
                        wss.refractionScale      = D::RefractionScale;
                        wss.horizontalExtinction = D::HorizontalExtinction;
                        wss.refractionReflection = D::RefractionReflection;
                    }

                    ImGui::SliderFloat("Water Clarity", &wss.waterClarity, 0.0f, 1.0f);
                    ImGui::SliderFloat("Water Transparency", &wss.waterTransparency, 0.0f, 50.0f);
                    ImGui::DragFloat4("Horizontal Extinction", wss.horizontalExtinction.data());
                    ImGui::DragFloat4("Refraction / Reflection", wss.refractionReflection.data());
                    ImGui::SliderFloat("Refraction Scale", &wss.refractionScale, 0.0f, 0.02f);
                }
                {
                    ImGui::SeparatorText("Reflection settings");

                    auto& wss = GetWaterShaderSettings().reflection;

                    float& shininess                         = wss.shininess;
                    std::array<float, 4>& specular_intensity = wss.specularIntensity;
                    float& distortion                        = wss.distortion;
                    float& radiance_factor                   = wss.radianceFactor;

                    ImGui::SameLine(resetButtonPosition);
                    if (ImGui::Button("Reset##Reflection"))
                    {
                        using D = ShaderDefaults::Water::Reflection;
                        wss.shininess          = D:: Shininess;
                        wss.distortion         = D:: Distortion;
                        wss.radianceFactor     = D:: RadianceFactor;
                        wss.specularIntensity  = D:: SpecularIntensity;
                    }

                    ImGui::SliderFloat("Shininess", &shininess, 0.0f, 1.0f);
                    ImGui::DragFloat4("Specular Intensity", specular_intensity.data());
                    ImGui::SliderFloat("Distortion", &distortion, 0.0f, 0.1f);
                    ImGui::SliderFloat("Radiance Factor", &radiance_factor, 0.0f, 1.0f);
                }
                {
                    ImGui::SeparatorText("Foam settings");

                    auto& wss = GetWaterShaderSettings().foam;

                    ImGui::SameLine(resetButtonPosition);
                    if (ImGui::Button("Reset##Foam"))
                    {
                        using D = ShaderDefaults::Water::Foam;
                        wss.foamSpeed     = D::FoamSpeed;
                        wss.foamIntensity = D::FoamIntensity;
                        wss.shoreFade     = D::ShoreFade;
                        wss.foamTiling    = D::FoamTiling;
                        wss.foamRanges    = D::FoamRanges;
                        wss.foamNoise     = D::FoamNoise;
                    }

                    ImGui::Text("Foam Texture"); ImGui::Button("Select##FoamTexture", ImVec2(120, 0));
                    ImGui::Text("Shore Texture"); ImGui::Button("Select##ShoreTexture", ImVec2(120, 0));
                    ImGui::DragFloat4("Foam Tiling", wss.foamTiling.data());
                    ImGui::DragFloat4("Foam Ranges", wss.foamRanges.data());
                    ImGui::DragFloat4("Foam Noise", wss.foamNoise.data());
                    ImGui::SliderFloat("Foam Speed", &wss.foamSpeed, 0.0f, 50.0f);
                    ImGui::SliderFloat("Foam Intensity", &wss.foamIntensity, 0.0f, 1.0f);
                    ImGui::SliderFloat("Shore Fade", &wss.shoreFade, 0.0f, 1.0f);
                }
                /* ⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘⫘ */

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("UI"))
            {
                ImGui::TextDisabled("UI Shaders");
                ImGui::Separator();
                ImGui::Text("UI shader controls coming later.");
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("PostFX"))
            {
                ImGui::TextDisabled("Post-Processing");
                ImGui::Separator();
                ImGui::Text("PostFX controls coming later.");
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
    }
    ImGui::End();
}
/* ----------------------------------------------------- */

void YosunAdminPanel::RenderGMCommands(bool* p_open)
{
    constexpr char kGMCommandsTitle[] = "GM Commands";

    if (ImGuiWindowFlags gmCommandsFlags = ImGuiWindowFlags_AlwaysAutoResize;
        ImGui::Begin(kGMCommandsTitle, p_open, gmCommandsFlags))
    {
        GMCommandsList();

        ImGui::Separator();

        {
            static bool isNightMode = false;

            if (ImGui::Checkbox("Night Mode", &isNightMode))
            {
                if (isNightMode)
                {
                    CPythonNetworkStream::Instance().SendChatPacket("/eclipse 1");
                }
                else
                {
                    CPythonNetworkStream::Instance().SendChatPacket("/eclipse 0");
                }
            }
        }

        ImGui::Separator();

        Warp();

    }
    ImGui::End();
}

void YosunAdminPanel::RenderDebugOverlay(bool* p_open)
{
    auto const& io = ImGui::GetIO();
    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoDecoration       |
        ImGuiWindowFlags_NoDocking          |
        ImGuiWindowFlags_AlwaysAutoResize   |
        ImGuiWindowFlags_NoSavedSettings    |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav;

    if (location >= 0)
    {
        const float PAD = 10.0f;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
        ImVec2 work_size = viewport->WorkSize;
        ImVec2 window_pos;
        ImVec2 window_pos_pivot;
        window_pos.x = (location & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
        window_pos.y = (location & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
        window_pos_pivot.x = (location & 1) ? 1.0f : 0.0f;
        window_pos_pivot.y = (location & 2) ? 1.0f : 0.0f;
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        ImGui::SetNextWindowViewport(viewport->ID);
        window_flags |= ImGuiWindowFlags_NoMove;
    }
    else if (location == -2)
    {
        // Center window
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        window_flags |= ImGuiWindowFlags_NoMove;
    }
    ImGui::SetNextWindowBgAlpha(0.35f);  // Transparent background
    if (ImGui::Begin("YosunAdminPanel Debug Overlay", p_open, window_flags))
    {
        ImGui::Text("Debug Overlay\n(right-click to change position)");
        ImGui::Separator();
        if (ImGui::IsMousePosValid())
        {
            ImGui::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
        }
        else
        {
            ImGui::Text("Mouse Position: <invalid>");
        }

        RenderFpsLineGraph();
        DebugOverlayChangePosition(p_open);
    }
    ImGui::End();
}

void YosunAdminPanel::DebugOverlayChangePosition(bool* p_open)
{
    if (ImGui::BeginPopupContextWindow())
    {
        if (ImGui::MenuItem("Custom", nullptr, location == -1))
        {
            location = -1;
        }
        if (ImGui::MenuItem("Center", nullptr, location == -2))
        {
            location = -2;
        }
        if (ImGui::MenuItem("Top-left", nullptr, location == 0))
        {
            location = 0;
        }
        if (ImGui::MenuItem("Top-right", nullptr, location == 1))
        {
            location = 1;
        }
        if (ImGui::MenuItem("Bottom-left", nullptr, location == 2))
        {
            location = 2;
        }
        if (ImGui::MenuItem("Bottom-right", nullptr, location == 3))
        {
            location = 3;
        }
        if (p_open && ImGui::MenuItem("Close"))
        {
            *p_open = false;
        }
        ImGui::EndPopup();
    }
}

void YosunAdminPanel::RenderFpsLineGraph() const
{
    auto const& io = ImGui::GetIO();

    // -----------------------------
    // --- Persistent Buffers
    // -----------------------------
    static float fpsValue = 0.0f;
    static std::array<float, 100> fpsHistory{};
    static size_t fpsHistoryIndex = 0;

    static float frametimeValue = 0.0f;
    static std::array<float, 100> frametimeHistory{};
    static size_t frametimeHistoryIndex = 0;

    static float ramProcessMB = 0.0f;
    static std::array<float, 100> ramHistory{};
    static size_t ramHistoryIndex = 0;

    // -----------------------------
    // --- Update Timer
    // -----------------------------
    static auto lastUpdate = std::chrono::steady_clock::now();
    constexpr auto updateInterval = std::chrono::duration<float> (1.0f);

    if (auto now = std::chrono::steady_clock::now();
        now - lastUpdate >= updateInterval)
    {
        lastUpdate = now;

        // ================================
        // FPS + Frametime
        // ================================
        fpsValue = io.Framerate;
        frametimeValue = 1000.0f / io.Framerate;

        fpsHistory[fpsHistoryIndex] = fpsValue;
        fpsHistoryIndex = (fpsHistoryIndex + 1) % fpsHistory.size();

        frametimeHistory[frametimeHistoryIndex] = frametimeValue;
        frametimeHistoryIndex = (frametimeHistoryIndex + 1) % frametimeHistory.size();

        // ================================
        // RAM Usage (Process Working Set)
        // ================================
        if (PROCESS_MEMORY_COUNTERS_EX pmc{};
            GetProcessMemoryInfo(GetCurrentProcess(),
                                 reinterpret_cast<PROCESS_MEMORY_COUNTERS*> (&pmc),
                                 sizeof(pmc)))
        {
            ramProcessMB = pmc.WorkingSetSize / (1024.0f * 1024.0f);
        }

        ramHistory[ramHistoryIndex] = ramProcessMB;
        ramHistoryIndex = (ramHistoryIndex + 1) % ramHistory.size();
    }

    // -----------------------------
    // --- UI Rendering
    // -----------------------------
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0.25f));
    ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(1, 1, 1, 0.6f));

    // Frametime Graph
    ImGui::Text("Frametime: %.1f ms", frametimeValue);
    ImGui::PlotLines(
        "##FrameTimeGraph",
        frametimeHistory.data(),
        static_cast<int> (frametimeHistory.size()),
        static_cast<int> (frametimeHistoryIndex),
        nullptr,
        0.0f,
        *std::ranges::max_element(frametimeHistory),
        ImVec2(0, 80)
    );

    // FPS Graph
    ImGui::Text("FPS: %.1f", fpsValue);
    ImGui::PlotLines(
        "##FPSGraph",
        fpsHistory.data(),
        static_cast<int> (fpsHistory.size()),
        static_cast<int> (fpsHistoryIndex),
        nullptr,
        0.0f,
        *std::ranges::max_element(fpsHistory),
        ImVec2(0, 80)
    );

    // RAM History Graph
    ImGui::Text("RAM: %.1f MB", ramProcessMB);
    ImGui::PlotLines(
        "##RAMGraph",
        ramHistory.data(),
        static_cast<int> (ramHistory.size()),
        static_cast<int> (ramHistoryIndex),
        nullptr,
        0.0f,
        *std::ranges::max_element(ramHistory),
        ImVec2(0, 80)
    );

    ImGui::Separator();

    ImGui::PopStyleColor(2);
}

void YosunAdminPanel::GMCommandsList() const
{
    struct Action
    {
        const char* label;
        const char* cmd;
    };

    static constexpr std::array actions{
        Action{ "Invisibility",           "/invisible"     },
        Action{ "Purge inventory",        "/ipurge"        },
        Action{ "Purge nearby entities",  "/purge"         },
        Action{ "Weaken nearby monsters", "/weak"          },
        Action{ "GM Full Set",            "/full_set"      },
        Action{ "Item Full Set",          "/item_full_set" },
        Action{ "Attribute Full Set",     "/attr_full_set" },
        Action{ "Call Horse",             "/horse_summon"  },
        Action{ "HP/SP Full",             "/reset"         },
        Action{ "Walk Mode",              "/set_walk_mode" },
        Action{ "Run Mode",               "/set_run_mode"  }
    };

    static int action_idx = 0;
    ImGui::Text("Generic GM Commands");
    if (ImGui::BeginCombo("##Action", actions[action_idx].label))
    {
        for (int i = 0; i < static_cast<int> (actions.size()); ++i)
        {
            bool is_selected = (action_idx == i);
            if (ImGui::Selectable(actions[i].label, is_selected))
            {
                action_idx = i;
            }
            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();    // scroll to selected on open
            }
        }
        ImGui::EndCombo();
    }

    ImGui::SameLine();
    if (ImGui::Button("Apply") && (action_idx >= 0 && action_idx < static_cast<int> (actions.size())))
    {
        CPythonNetworkStream::Instance().SendChatPacket(actions[action_idx].cmd);
    }
}

void YosunAdminPanel::LoadAtlasInfo()
{
    m_atlasEntries.clear();

    CMappedFile      kFile;
    LPCVOID          pData;
    if (!CEterPackManager::Instance().Get(kFile, "AtlasInfo.txt", &pData))
    {
        return;    // no atlas data
    }

    CMemoryTextFileLoader loader;
    loader.Bind(kFile.Size(), pData);

    const std::size_t lineCount = loader.GetLineCount();
    m_atlasEntries.reserve(lineCount);

    for (std::size_t i = 0; i < lineCount; ++i)
    {
        // get the full line as a std::string, then view it
        std::string_view sv = loader.GetLineString(i);

        // split out first three columns by '\t'
        auto pos0 = sv.find('\t');       // end of name
        if (pos0 == std::string_view::npos)
        {
            continue;
        }

        auto pos1 = sv.find('\t', pos0 + 1);  // end of x100
        if (pos1 == std::string_view::npos)
        {
            continue;
        }

        auto pos2 = sv.find('\t', pos1 + 1);  // end of y100
        if (pos2 == std::string_view::npos)
        {
            continue;
        }

        // name field (could be empty, skip if so)
        auto name_sv = sv.substr(0, pos0);
        if (name_sv.empty())
        {
            continue;
        }

        // numeric fields as views
        auto x_sv = sv.substr(pos0 + 1, pos1 - pos0 - 1);
        auto y_sv = sv.substr(pos1 + 1, pos2 - pos1 - 1);

        int x100{};
        int y100{};
        // parse without allocations
        if (auto[ptr, ec] = std::from_chars(x_sv.data(), x_sv.data() + x_sv.size(), x100); ec != std::errc{})
            continue;
        if (auto[ptr, ec] = std::from_chars(y_sv.data(), y_sv.data() + y_sv.size(), y100); ec != std::errc{})
            continue;

        // store a real string for the name
        m_atlasEntries.emplace_back(std::string{ name_sv }, x100, y100);
    }
}

void YosunAdminPanel::Warp()
{
    ImGui::TextUnformatted("Warp to Map");

    if (m_atlasEntries.empty())
    {
        ImGui::TextUnformatted("No map data loaded.");
        return;
    }

    // build C‐string list
    std::vector<const char*> names;
    names.reserve(m_atlasEntries.size());
    for (auto const& e : m_atlasEntries)
    {
        names.push_back(e.name.c_str());
    }

    ImGui::Combo("##Warp", (&m_warpIdx), names.data(), static_cast<int> (names.size()));
    ImGui::SameLine();
    if (ImGui::Button("Warp"))
    {
        const auto& e = m_atlasEntries[m_warpIdx];
        int realX = e.x / 100;
        int realY = e.y / 100;

        // build and send
        auto cmd = std::format("/warp {} {}", realX, realY);
        CPythonNetworkStream::Instance().SendChatPacket(cmd.c_str());
    }
}
#endif /* _DEBUG */
