﻿/*
    MIT License

    Copyright (c) 2018-2020 NovusCore

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/
#pragma once
#include <NovusTypes.h>

constexpr u16 CELL_OUTER_GRID_SIDE = 9;
constexpr u16 CELL_OUTER_GRID_SIZE = CELL_OUTER_GRID_SIDE * CELL_OUTER_GRID_SIDE;

constexpr u16 CELL_INNER_GRID_SIDE = 8;
constexpr u16 CELL_INNER_GRID_SIZE = CELL_INNER_GRID_SIDE * CELL_INNER_GRID_SIDE;

constexpr u16 CELL_TOTAL_GRID_SIZE = CELL_OUTER_GRID_SIZE + CELL_INNER_GRID_SIZE;

constexpr u32 CELL_ALPHAMAP_SIZE = 64 * 64;

#pragma pack(push, 1)
constexpr u32 TextureIdInvalid = std::numeric_limits<u32>().max();
struct LayerData
{
    u32 textureId = TextureIdInvalid;
};

struct Cell
{
    u16 areaId = 0;

    f32 heightData[CELL_TOTAL_GRID_SIZE] = { 0 };
    u8 normalData[CELL_TOTAL_GRID_SIZE][3] = { {127}, {255}, {127} }; // This is ugly but lets us pack this data into 25% of the original size
    u8 colorData[CELL_TOTAL_GRID_SIZE][3] = { {127}, {127}, {127} }; // This is ugly but lets us pack this data into 25% of the original size

    u16 hole = 0;

    LayerData layers[4];
};
#pragma pack(pop)