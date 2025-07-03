@echo off

shadercross "resources/shaders/src/ascii2D.vert.hlsl" -o "resources/shaders/comp/SPV/ascii2D.vert.spv"
shadercross "resources/shaders/src/ascii2D.vert.hlsl" -o "resources/shaders/comp/MSL/ascii2D.vert.msl"
shadercross "resources/shaders/src/ascii2D.vert.hlsl" -o "resources/shaders/comp/DXIL/ascii2D.vert.dxil"

shadercross "resources/shaders/src/ascii2D.frag.hlsl" -o "resources/shaders/comp/SPV/ascii2D.frag.spv"
shadercross "resources/shaders/src/ascii2D.frag.hlsl" -o "resources/shaders/comp/MSL/ascii2D.frag.msl"
shadercross "resources/shaders/src/ascii2D.frag.hlsl" -o "resources/shaders/comp/DXIL/ascii2D.frag.dxil"

shadercross "resources/shaders/src/box.vert.hlsl" -o "resources/shaders/comp/SPV/box.vert.spv"
shadercross "resources/shaders/src/box.vert.hlsl" -o "resources/shaders/comp/MSL/box.vert.msl"
shadercross "resources/shaders/src/box.vert.hlsl" -o "resources/shaders/comp/DXIL/box.vert.dxil"

shadercross "resources/shaders/src/box.frag.hlsl" -o "resources/shaders/comp/SPV/box.frag.spv"
shadercross "resources/shaders/src/box.frag.hlsl" -o "resources/shaders/comp/MSL/box.frag.msl"
shadercross "resources/shaders/src/box.frag.hlsl" -o "resources/shaders/comp/DXIL/box.frag.dxil"

shadercross "resources/shaders/src/asciiLayers.vert.hlsl" -o "resources/shaders/comp/SPV/asciiLayers.vert.spv"
shadercross "resources/shaders/src/asciiLayers.vert.hlsl" -o "resources/shaders/comp/MSL/asciiLayers.vert.msl"
shadercross "resources/shaders/src/asciiLayers.vert.hlsl" -o "resources/shaders/comp/DXIL/asciiLayers.vert.dxil"

shadercross "resources/shaders/src/test3D.vert.hlsl" -o "resources/shaders/comp/SPV/test3D.vert.spv"
shadercross "resources/shaders/src/test3D.vert.hlsl" -o "resources/shaders/comp/MSL/test3D.vert.msl"
shadercross "resources/shaders/src/test3D.vert.hlsl" -o "resources/shaders/comp/DXIL/test3D.vert.dxil"

shadercross "resources/shaders/src/test3D.frag.hlsl" -o "resources/shaders/comp/SPV/test3D.frag.spv"
shadercross "resources/shaders/src/test3D.frag.hlsl" -o "resources/shaders/comp/MSL/test3D.frag.msl"
shadercross "resources/shaders/src/test3D.frag.hlsl" -o "resources/shaders/comp/DXIL/test3D.frag.dxil"

shadercross "resources/shaders/src/odyssey.vert.hlsl" -o "resources/shaders/comp/SPV/odyssey.vert.spv"
shadercross "resources/shaders/src/odyssey.vert.hlsl" -o "resources/shaders/comp/MSL/odyssey.vert.msl"
shadercross "resources/shaders/src/odyssey.vert.hlsl" -o "resources/shaders/comp/DXIL/odyssey.vert.dxil"

shadercross "resources/shaders/src/odyssey.frag.hlsl" -o "resources/shaders/comp/SPV/odyssey.frag.spv"
shadercross "resources/shaders/src/odyssey.frag.hlsl" -o "resources/shaders/comp/MSL/odyssey.frag.msl"
shadercross "resources/shaders/src/odyssey.frag.hlsl" -o "resources/shaders/comp/DXIL/odyssey.frag.dxil"

shadercross "resources/shaders/src/debug3D.vert.hlsl" -o "resources/shaders/comp/SPV/debug3D.vert.spv"
shadercross "resources/shaders/src/debug3D.vert.hlsl" -o "resources/shaders/comp/MSL/debug3D.vert.msl"
shadercross "resources/shaders/src/debug3D.vert.hlsl" -o "resources/shaders/comp/DXIL/debug3D.vert.dxil"