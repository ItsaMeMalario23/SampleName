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