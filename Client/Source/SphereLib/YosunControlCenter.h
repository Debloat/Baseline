/*
By Kaptan Yosun @ mmotutkunlari.com

This header is used to store various values for debugging purposes.
*/

#pragma once

#include <cstdint>

namespace YosunControlDefaults
{
    namespace TextMetrics
    {
        constexpr uint64_t TotalChars         = 0;
        constexpr uint64_t TotalDrawCalls     = 0;
        constexpr uint64_t TotalTextInstances = 0;
        constexpr uint64_t PeakChars          = 0;
        constexpr uint64_t PeakDrawCalls      = 0;
        constexpr uint64_t PeakTextInstances  = 0;
        constexpr bool ForceDisableOutline = false;
    }

    namespace TerrainMetrics
    {
        constexpr uint64_t TotalDrawCalls = 0;
        constexpr uint64_t PeakDrawCalls = 0;
        constexpr uint64_t RenderedPatches = 0;
        constexpr uint64_t RenderedSplats = 0;
    }

    namespace WorldEditor
    {
        constexpr bool DrawCollision = false;

        namespace Terrain
        {
            constexpr bool DrawTerrainWireFrame = false;
            constexpr bool DrawWaterWireFrame   = false;
            constexpr bool DrawPatchGrid        = false;
        }

        namespace SpeedTree
        {
            constexpr float WindStrength         = 40.0f;
            constexpr float WindOscillationSpeed = 20.0f;
        }
    }

}

struct YosunControlSettings
{
    struct TextMetrics
    {
        uint64_t totalChars         = YosunControlDefaults::TextMetrics::TotalChars;
        uint64_t totalDrawCalls     = YosunControlDefaults::TextMetrics::TotalDrawCalls;
        uint64_t totalTextInstances = YosunControlDefaults::TextMetrics::TotalTextInstances;
        uint64_t peakChars          = YosunControlDefaults::TextMetrics::PeakChars;
        uint64_t peakDrawCalls      = YosunControlDefaults::TextMetrics::PeakDrawCalls;
        uint64_t peakTextInstances  = YosunControlDefaults::TextMetrics::PeakTextInstances;
        bool forceDisableOutline    = YosunControlDefaults::TextMetrics::ForceDisableOutline;
    } textMetrics;

    struct TerrainMetrics
    {
        uint64_t totalDrawCalls     = YosunControlDefaults::TerrainMetrics::TotalDrawCalls;
        uint64_t peakDrawCalls      = YosunControlDefaults::TerrainMetrics::PeakDrawCalls;
        uint64_t renderedPatches    = YosunControlDefaults::TerrainMetrics::RenderedPatches;
        uint64_t renderedSplats     = YosunControlDefaults::TerrainMetrics::RenderedSplats;
    } terrainMetrics;

    struct WorldEditor
    {
        bool drawCollision          = YosunControlDefaults::WorldEditor::DrawCollision;

        struct Terrain
        {
            bool drawTerrainWireFrame  = YosunControlDefaults::WorldEditor::Terrain::DrawTerrainWireFrame;
            bool drawWaterWireFrame    = YosunControlDefaults::WorldEditor::Terrain::DrawWaterWireFrame;
            bool drawPatchGrid         = YosunControlDefaults::WorldEditor::Terrain::DrawPatchGrid;
        } terrain;

        struct SpeedTree
        {
            float windStrength         = YosunControlDefaults::WorldEditor::SpeedTree::WindStrength;
            float windOscillationSpeed = YosunControlDefaults::WorldEditor::SpeedTree::WindOscillationSpeed;
        } speedTree;

    } worldEditor;
};

inline YosunControlSettings& GetYosunControlSettings()
{
    static YosunControlSettings s_settings;
    return s_settings;
}

