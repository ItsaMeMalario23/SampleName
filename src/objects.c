#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL3/SDL.h>

#include <objects.h>
#include <algebra.h>
#include <input.h>
#include <render/render.h>
#include <render/camera.h>
#include <debug/rdebug.h>
#include <debug/memtrack.h>

//
//  Object char data
//
const ascii2info_t birddata[16] = {
    {'(', 0xffffffff, {-0.0187500, -0.0694444}}, {'@', 0xfcd303ff, {0.0046875, -0.0722222}}, {'-', 0xffffffff, {0.0484375, -0.0305556}}, {'\\', 0xffffffff, {0.0734375, -0.0500000}},
    {'\\', 0xffffffff, {0.0031250, -0.1361111}}, {')', 0xffffffff, {0.0265625, -0.0750000}}, {'O', 0xffffffff, {0.0500000, -0.0611111}}, {'>', 0xff0000ff, {0.0859375, -0.0944444}},
    {'\'', 0xff, {0.0515625, -0.0722222}}, {'_', 0xffffffff, {0.0281250, -0.1333333}}, {'_', 0xffffffff, {0.0515625, -0.1333333}}, {'/', 0xffffffff, {0.0765625, -0.1333333}},
    {'>', 0xff0000ff, {0.0718750, -0.0944444}}, {'-', 0xffffffff, {0.0218750, -0.0305556}}, {'B', 0xfcd303ff, {0.0265625, -0.1222222}}, {'D', 0xfcd303ff, {0.0531250, -0.1222222}},
};

const ascii2info_t cherrydata[6] = {
    {'O', 0xff0000ff, {0.0218750f, 0.0166667f}}, {'-', 0xff0000ff, {0.0203125f, -0.0166667f}}, {')', 0xff0000ff, {0.0421875f, 0.0083333f}}, {'(', 0xff0000ff, {0.0000000f, 0.0083333f}},
    {',', 0x40d800ff, {0.0265625f, 0.0694444f}}, {'.', 0x40d800ff, {0.0250000f, 0.0888889f}},
};

const ascii2info_t flagdata[195] = {
    {'#', 0x00690aff, {0.0109375f, -0.1611111f}}, {'#', 0x00690aff, {0.0531250f, -0.1611111f}}, {'#', 0x00690aff, {0.0953125f, -0.1611111f}}, {'#', 0x00690aff, {0.1375000f, -0.1611111f}},
    {'#', 0x00690aff, {0.1796875f, -0.1611111f}}, {'*', 0x000000ff, {0.0312500f, -0.1833333f}}, {'*', 0x000000ff, {0.0734375f, -0.1833333f}}, {'*', 0x000000ff, {0.1156250f, -0.1833333f}},
    {'*', 0x000000ff, {0.1578125f, -0.1833333f}}, {'*', 0x000000ff, {0.0531250f, -0.2194445f}}, {'*', 0x000000ff, {0.0953125f, -0.2194445f}}, {'*', 0x000000ff, {0.1375000f, -0.2194445f}},
    {'*', 0x000000ff, {0.1578125f, -0.2555556f}}, {'*', 0x000000ff, {0.0312500f, -0.2555556f}}, {'*', 0x000000ff, {0.0734375f, -0.2555556f}}, {'*', 0x000000ff, {0.1156250f, -0.2555556f}},
    {'#', 0x00690aff, {0.0109375f, -0.2777778f}}, {'#', 0x00690aff, {0.0109375f, -0.2194445f}}, {'#', 0x00690aff, {0.0531250f, -0.2777778f}}, {'#', 0x00690aff, {0.0953125f, -0.2777778f}},
    {'#', 0x00690aff, {0.1375000f, -0.2777778f}}, {'#', 0x00690aff, {0.1796875f, -0.2777778f}}, {'#', 0x00690aff, {0.1796875f, -0.2194445f}}, {'+', 0x00690aff, {0.0296875f, -0.2194445f}},
    {'+', 0x00690aff, {0.0734375f, -0.2194445f}}, {'+', 0x00690aff, {0.1140625f, -0.2194445f}}, {'+', 0x00690aff, {0.1578125f, -0.2194445f}}, {'^', 0x00690aff, {0.0531250f, -0.2055556f}},
    {'"', 0x00690aff, {0.0312500f, -0.1638889f}}, {'^', 0x00690aff, {0.0953125f, -0.2055556f}}, {'^', 0x00690aff, {0.1375000f, -0.2055556f}}, {'^', 0x00690aff, {0.1375000f, -0.2583333f}},
    {'^', 0x00690aff, {0.0531250f, -0.2583333f}}, {'^', 0x00690aff, {0.0953125f, -0.2583333f}}, {'"', 0x00690aff, {0.0734375f, -0.1638889f}}, {'"', 0x00690aff, {0.1156250f, -0.1638889f}},
    {'"', 0x00690aff, {0.1578125f, -0.1638889f}}, {'"', 0x00690aff, {0.0312500f, -0.2972222f}}, {'"', 0x00690aff, {0.0734375f, -0.2972222f}}, {'"', 0x00690aff, {0.1156250f, -0.2972222f}},
    {'"', 0x00690aff, {0.1578125f, -0.2972222f}}, {'-', 0x00690aff, {0.1781250f, -0.1916667f}}, {'-', 0x00690aff, {0.1781250f, -0.2472222f}}, {'-', 0x00690aff, {0.0078125f, -0.2472222f}},
    {'-', 0x00690aff, {0.0078125f, -0.1916667f}}, {'=', 0xff0000ff, {0.2093750f, -0.1527778f}}, {'=', 0xff0000ff, {0.2343750f, -0.1527778f}}, {'=', 0xff0000ff, {0.2593750f, -0.1527778f}},
    {'=', 0xff0000ff, {0.2843750f, -0.1527778f}}, {'=', 0xff0000ff, {0.3093750f, -0.1527778f}}, {'=', 0xff0000ff, {0.3343750f, -0.1527778f}}, {'=', 0xff0000ff, {0.3593750f, -0.1527778f}},
    {'=', 0x00000000, {0.3843750f, -0.1527778f}}, {'=', 0x00000000, {0.4093750f, -0.1527778f}}, {'=', 0xff000000, {0.4343750f, -0.1527778f}}, {'=', 0x000000ff, {0.2093750f, -0.1805556f}},
    {'=', 0x000000ff, {0.2343750f, -0.1805556f}}, {'=', 0xff0000ff, {0.2093750f, -0.2083333f}}, {'=', 0x000000ff, {0.2093750f, -0.2361111f}}, {'=', 0xff0000ff, {0.2093750f, -0.2638889f}},
    {'=', 0x000000ff, {0.2093750f, -0.2916667f}}, {'=', 0xff0000ff, {0.2093750f, -0.3194444f}}, {'=', 0x000000ff, {0.2093750f, -0.3472222f}}, {'=', 0xff0000ff, {0.2093750f, -0.3750000f}},
    {'=', 0x000000ff, {0.2093750f, -0.4027778f}}, {'=', 0xff0000ff, {0.2093750f, -0.4305556f}}, {'=', 0x000000ff, {0.2593750f, -0.1805556f}}, {'=', 0x000000ff, {0.2843750f, -0.1805556f}},
    {'=', 0x000000ff, {0.3093750f, -0.1805556f}}, {'=', 0x000000ff, {0.3343750f, -0.1805556f}}, {'=', 0x000000ff, {0.3593750f, -0.1805556f}}, {'=', 0x00000000, {0.3843750f, -0.1805556f}},
    {'=', 0x00000000, {0.4093750f, -0.1805556f}}, {'=', 0x00000000, {0.4343750f, -0.1805556f}}, {'=', 0xff0000ff, {0.2343750f, -0.2083333f}}, {'=', 0xff0000ff, {0.2593750f, -0.2083333f}},
    {'=', 0xff0000ff, {0.2843750f, -0.2083333f}}, {'=', 0xff0000ff, {0.3093750f, -0.2083333f}}, {'=', 0xff0000ff, {0.3343750f, -0.2083333f}}, {'=', 0xff0000ff, {0.3593750f, -0.2083333f}},
    {'=', 0x00000000, {0.3843750f, -0.2083333f}}, {'=', 0x00000000, {0.4093750f, -0.2083333f}}, {'=', 0xff000000, {0.4343750f, -0.2083333f}}, {'=', 0x000000ff, {0.2343750f, -0.2361111f}},
    {'=', 0x000000ff, {0.2593750f, -0.2361111f}}, {'=', 0x000000ff, {0.2843750f, -0.2361111f}}, {'=', 0x000000ff, {0.3093750f, -0.2361111f}}, {'=', 0x000000ff, {0.3343750f, -0.2361111f}},
    {'=', 0x000000ff, {0.3593750f, -0.2361111f}}, {'=', 0x00000000, {0.3843750f, -0.2361111f}}, {'=', 0x00000000, {0.4093750f, -0.2361111f}}, {'=', 0x00000000, {0.4343750f, -0.2361111f}},
    {'=', 0xff0000ff, {0.2343750f, -0.2638889f}}, {'=', 0xff0000ff, {0.2593750f, -0.2638889f}}, {'=', 0xff0000ff, {0.2843750f, -0.2638889f}}, {'=', 0xff0000ff, {0.3093750f, -0.2638889f}},
    {'=', 0xff0000ff, {0.3343750f, -0.2638889f}}, {'=', 0xff0000ff, {0.3593750f, -0.2638889f}}, {'=', 0x00000000, {0.3843750f, -0.2638889f}}, {'=', 0x00000000, {0.4093750f, -0.2638889f}},
    {'=', 0xff000000, {0.4343750f, -0.2638889f}}, {'=', 0x000000ff, {0.2343750f, -0.2916667f}}, {'=', 0x000000ff, {0.2593750f, -0.2916667f}}, {'=', 0x000000ff, {0.2843750f, -0.2916667f}},
    {'=', 0x000000ff, {0.3093750f, -0.2916667f}}, {'=', 0x000000ff, {0.3343750f, -0.2916667f}}, {'=', 0x000000ff, {0.3593750f, -0.2916667f}}, {'=', 0x00000000, {0.3843750f, -0.2916667f}},
    {'=', 0x00000000, {0.4093750f, -0.2916667f}}, {'=', 0x00000000, {0.4343750f, -0.2916667f}}, {'=', 0xff0000ff, {0.2343750f, -0.3194444f}}, {'=', 0xff0000ff, {0.2593750f, -0.3194444f}},
    {'=', 0xff0000ff, {0.2843750f, -0.3194444f}}, {'=', 0xff0000ff, {0.3093750f, -0.3194444f}}, {'=', 0xff0000ff, {0.3343750f, -0.3194444f}}, {'=', 0xff0000ff, {0.3593750f, -0.3194444f}},
    {'=', 0x00000000, {0.3843750f, -0.3194444f}}, {'=', 0x00000000, {0.4093750f, -0.3194444f}}, {'=', 0xff000000, {0.4343750f, -0.3194444f}}, {'=', 0x000000ff, {0.2343750f, -0.3472222f}},
    {'=', 0x000000ff, {0.2593750f, -0.3472222f}}, {'=', 0x000000ff, {0.2843750f, -0.3472222f}}, {'=', 0x000000ff, {0.3093750f, -0.3472222f}}, {'=', 0x000000ff, {0.3343750f, -0.3472222f}},
    {'=', 0x000000ff, {0.3593750f, -0.3472222f}}, {'=', 0x00000000, {0.3843750f, -0.3472222f}}, {'=', 0x00000000, {0.4093750f, -0.3472222f}}, {'=', 0x00000000, {0.4343750f, -0.3472222f}},
    {'=', 0xff0000ff, {0.2343750f, -0.3750000f}}, {'=', 0xff0000ff, {0.2593750f, -0.3750000f}}, {'=', 0xff0000ff, {0.2843750f, -0.3750000f}}, {'=', 0xff0000ff, {0.3093750f, -0.3750000f}},
    {'=', 0xff0000ff, {0.3343750f, -0.3750000f}}, {'=', 0xff0000ff, {0.3593750f, -0.3750000f}}, {'=', 0x00000000, {0.3843750f, -0.3750000f}}, {'=', 0x00000000, {0.4093750f, -0.3750000f}},
    {'=', 0xff000000, {0.4343750f, -0.3750000f}}, {'=', 0x000000ff, {0.2343750f, -0.4027778f}}, {'=', 0x000000ff, {0.2593750f, -0.4027778f}}, {'=', 0x000000ff, {0.2843750f, -0.4027778f}},
    {'=', 0x000000ff, {0.3093750f, -0.4027778f}}, {'=', 0x000000ff, {0.3343750f, -0.4027778f}}, {'=', 0x000000ff, {0.3593750f, -0.4027778f}}, {'=', 0x00000000, {0.3843750f, -0.4027778f}},
    {'=', 0x00000000, {0.4093750f, -0.4027778f}}, {'=', 0x00000000, {0.4343750f, -0.4027778f}}, {'=', 0xff0000ff, {0.2343750f, -0.4305556f}}, {'=', 0xff0000ff, {0.2593750f, -0.4305556f}},
    {'=', 0xff0000ff, {0.2843750f, -0.4305556f}}, {'=', 0xff0000ff, {0.3093750f, -0.4305556f}}, {'=', 0xff0000ff, {0.3343750f, -0.4305556f}}, {'=', 0xff0000ff, {0.3593750f, -0.4305556f}},
    {'=', 0x00000000, {0.3843750f, -0.4305556f}}, {'=', 0x00000000, {0.4093750f, -0.4305556f}}, {'=', 0xff000000, {0.4343750f, -0.4305556f}}, {'=', 0xff0000ff, {0.1843750f, -0.4305556f}},
    {'=', 0xff0000ff, {0.1593750f, -0.4305556f}}, {'=', 0xff0000ff, {0.1343750f, -0.4305556f}}, {'=', 0xff0000ff, {0.1093750f, -0.4305556f}}, {'=', 0xff0000ff, {0.0843750f, -0.4305556f}},
    {'=', 0xff0000ff, {0.0593750f, -0.4305556f}}, {'=', 0xff0000ff, {0.0343750f, -0.4305556f}}, {'=', 0xff0000ff, {0.0093750f, -0.4305556f}}, {'=', 0x000000ff, {0.0093750f, -0.4027778f}},
    {'=', 0x000000ff, {0.0343750f, -0.4027778f}}, {'=', 0x000000ff, {0.0593750f, -0.4027778f}}, {'=', 0x000000ff, {0.0843750f, -0.4027778f}}, {'=', 0x000000ff, {0.1093750f, -0.4027778f}},
    {'=', 0x000000ff, {0.1343750f, -0.4027778f}}, {'=', 0x000000ff, {0.1593750f, -0.4027778f}}, {'=', 0x000000ff, {0.1843750f, -0.4027778f}}, {'=', 0xff0000ff, {0.0093750f, -0.3750000f}},
    {'=', 0xff0000ff, {0.0343750f, -0.3750000f}}, {'=', 0xff0000ff, {0.0593750f, -0.3750000f}}, {'=', 0xff0000ff, {0.0843750f, -0.3750000f}}, {'=', 0xff0000ff, {0.1093750f, -0.3750000f}},
    {'=', 0xff0000ff, {0.1343750f, -0.3750000f}}, {'=', 0xff0000ff, {0.1593750f, -0.3750000f}}, {'=', 0xff0000ff, {0.1843750f, -0.3750000f}}, {'=', 0x000000ff, {0.0093750f, -0.3472222f}},
    {'=', 0x000000ff, {0.0343750f, -0.3472222f}}, {'=', 0x000000ff, {0.0593750f, -0.3472222f}}, {'=', 0x000000ff, {0.0843750f, -0.3472222f}}, {'=', 0x000000ff, {0.1093750f, -0.3472222f}},
    {'=', 0x000000ff, {0.1343750f, -0.3472222f}}, {'=', 0x000000ff, {0.1593750f, -0.3472222f}}, {'=', 0x000000ff, {0.1843750f, -0.3472222f}}, {'=', 0xff0000ff, {0.0093750f, -0.3194444f}},
    {'=', 0xff0000ff, {0.0343750f, -0.3194444f}}, {'=', 0xff0000ff, {0.0593750f, -0.3194444f}}, {'=', 0xff0000ff, {0.0843750f, -0.3194444f}}, {'=', 0xff0000ff, {0.1093750f, -0.3194444f}},
    {'=', 0xff0000ff, {0.1343750f, -0.3194444f}}, {'=', 0xff0000ff, {0.1593750f, -0.3194444f}}, {'=', 0xff0000ff, {0.1843750f, -0.3194444f}},
};

const ascii2info_t pillardata[22] = {
    {'=', 0xffffffff, {0.0031250f, -0.0388889f}}, {'=', 0xffffffff, {0.0281250f, -0.0388889f}}, {'=', 0xffffffff, {0.0531250f, -0.0388889f}}, {'=', 0xffffffff, {0.0781250f, -0.0388889f}},
    {'H', 0xffffffff, {0.0109375f, -0.0750000f}}, {'H', 0xffffffff, {0.0406250f, -0.0750000f}}, {'H', 0xffffffff, {0.0687500f, -0.0750000f}}, {'|', 0xffffffff, {0.0093750f, -0.1250000f}},
    {'-', 0xffffffff, {0.0390625f, -0.1250000f}}, {'|', 0xffffffff, {0.0671875f, -0.1250000f}}, {'|', 0xffffffff, {0.0093750f, -0.1805556f}}, {'-', 0xffffffff, {0.0390625f, -0.1805556f}},
    {'|', 0xffffffff, {0.0671875f, -0.1805556f}},
    {'|', 0xffffffff, {0.0093750f, -0.2361112f}}, {'-', 0xffffffff, {0.0390625f, -0.2361112f}}, {'|', 0xffffffff, {0.0671875f, -0.2361112f}},
    {'|', 0xffffffff, {0.0093750f, -0.2916668f}}, {'-', 0xffffffff, {0.0390625f, -0.2916668f}}, {'|', 0xffffffff, {0.0671875f, -0.2916668f}},
    {'|', 0xffffffff, {0.0093750f, -0.3472224f}}, {'-', 0xffffffff, {0.0390625f, -0.3472224f}}, {'|', 0xffffffff, {0.0671875f, -0.3472224f}},
};

const ascii2info_t flagpoledata[40] = {
    {'F', 0x291700ff, {0.0156250f, -0.0722222f}}, {'I', 0x382000ff, {0.0320000f, -0.0722222f}}, {'F', 0x291700ff, {0.0156250f, -0.1222222f}}, {'I', 0x382000ff, {0.0320000f, -0.1222222f}},
    {'F', 0x291700ff, {0.0156250f, -0.1722222f}}, {'I', 0x382000ff, {0.0320000f, -0.1722222f}}, {'F', 0x291700ff, {0.0156250f, -0.2222222f}}, {'I', 0x382000ff, {0.0320000f, -0.2222222f}},
    {'F', 0x291700ff, {0.0156250f, -0.2722222f}}, {'I', 0x382000ff, {0.0320000f, -0.2722222f}}, {'F', 0x291700ff, {0.0156250f, -0.3222222f}}, {'I', 0x382000ff, {0.0320000f, -0.3222222f}},
    {'F', 0x291700ff, {0.0156250f, -0.3722222f}}, {'I', 0x382000ff, {0.0320000f, -0.3722222f}}, {'F', 0x291700ff, {0.0156250f, -0.4222222f}}, {'I', 0x382000ff, {0.0320000f, -0.4222222f}},
    {'F', 0x291700ff, {0.0156250f, -0.4722222f}}, {'I', 0x382000ff, {0.0320000f, -0.4722222f}}, {'F', 0x291700ff, {0.0156250f, -0.5222222f}}, {'I', 0x382000ff, {0.0320000f, -0.5222222f}},
    {'F', 0x291700ff, {0.0156250f, -0.5722222f}}, {'I', 0x382000ff, {0.0320000f, -0.5722222f}}, {'F', 0x291700ff, {0.0156250f, -0.6222222f}}, {'I', 0x382000ff, {0.0320000f, -0.6222222f}},
    {'F', 0x291700ff, {0.0156250f, -0.6722222f}}, {'I', 0x382000ff, {0.0320000f, -0.6722222f}}, {'F', 0x291700ff, {0.0156250f, -0.7222222f}}, {'I', 0x382000ff, {0.0320000f, -0.7222222f}},
    {'F', 0x291700ff, {0.0156250f, -0.7722222f}}, {'I', 0x382000ff, {0.0320000f, -0.7722222f}}, {'F', 0x291700ff, {0.0156250f, -0.8222222f}}, {'I', 0x382000ff, {0.0320000f, -0.8222222f}},
    {'F', 0x291700ff, {0.0156250f, -0.8722222f}}, {'I', 0x382000ff, {0.0320000f, -0.8722222f}}, {'F', 0x291700ff, {0.0156250f, -0.9222222f}}, {'I', 0x382000ff, {0.0320000f, -0.9222222f}},
    {'F', 0x291700ff, {0.0156250f, -0.9722222f}}, {'I', 0x382000ff, {0.0320000f, -0.9722222f}}, {'F', 0x291700ff, {0.0156250f, -1.0222222f}}, {'I', 0x382000ff, {0.0320000f, -1.0222222f}},
};

const ascii2info_t terraindata[14] = {
    {'G', 0x0b8003ff, {0.0015625f, -0.0000000f}}, {'R', 0x0b8003ff, {0.0265625f, 0.0055556f}}, {'A', 0x0b8003ff, {0.0515625f, 0.0222222f}}, {'S', 0x0b8003ff, {0.0765625f, 0.0388889f}},
    {'S', 0x0b8003ff, {0.1031250f, 0.0472222f}}, {'Y', 0x0b8003ff, {0.1265625f, 0.0527778f}}, {'#', 0x14910cff, {0.1515625f, 0.0527778f}}, {'T', 0x0b8003ff, {0.1734375f, 0.0527778f}},
    {'E', 0x0b8003ff, {0.1984375f, 0.0583333f}}, {'R', 0x0b8003ff, {0.2234375f, 0.0638889f}}, {'R', 0x0b8003ff, {0.2484375f, 0.0666667f}}, {'A', 0x0b8003ff, {0.2750000f, 0.0805556f}},
    {'I', 0x0b8003ff, {0.2968750f, 0.0861111f}}, {'N', 0x0b8003ff, {0.3234375f, 0.0972222f}},
};

const ascii2info_t mariodata[49] = {
    {'c', 0xcf2611ff, {0.0062500f, 0.2305555f}}, {'o', 0xcf2611ff, {0.0546875f, 0.2305555f}}, {'=', 0xcf2611ff, {0.0312500f, 0.2194444f}}, {'-', 0xcf2611ff, {0.0750000f, 0.2138889f}},
    {'-', 0xcf2611ff, {0.0296875f, 0.2361111f}}, {'\'', 0xffffffff, {0.0593750f, 0.1805556f}}, {'T', 0x685d00ff, {0.0156250f, 0.1861111f}}, {'[', 0x685d00ff, {-0.0062500f, 0.1861111f}},
    {'e', 0x685d00ff, {0.0078125f, 0.1500000f}}, {'~', 0x685d00ff, {0.0718750f, 0.1555556f}}, {'-', 0x685d00ff, {0.0562500f, 0.1472222f}}, {',', 0xea9e22ff, {0.0734375f, 0.1944444f}},
    {'.', 0xea9e22ff, {0.0718750f, 0.2138889f}}, {'K', 0xea9e22ff, {0.0359375f, 0.1472222f}}, {'-', 0xea9e22ff, {0.0562500f, 0.1361111f}}, {'$', 0xea9e22ff, {0.0390625f, 0.1888889f}},
    {'\'', 0xea9e22ff, {0.0078125f, 0.1722222f}}, {',', 0xea9e22ff, {0.0578125f, 0.1805556f}}, {'|', 0xcf2611ff, {0.0140625f, 0.1027778f}}, {'|', 0xcf2611ff, {0.0484375f, 0.1027778f}},
    {'A', 0xcf2611ff, {0.0156250f, 0.0611111f}}, {'A', 0xcf2611ff, {0.0500000f, 0.0611111f}}, {'|', 0xcf2611ff, {0.0312500f, 0.0611111f}}, {'/', 0xcf2611ff, {0.0203125f, 0.0166667f}},
    {'\\', 0xcf2611ff, {0.0453125f, 0.0166667f}}, {'o', 0x685d00ff, {-0.0046875f, -0.0055556f}}, {'o', 0x685d00ff, {0.0718750f, -0.0055556f}}, {'\'', 0x685d00ff, {0.0125000f, -0.0250000f}},
    {'\'', 0x685d00ff, {0.0578125f, -0.0250000f}}, {'M', 0x685d00ff, {0.0328125f, 0.1027778f}}, {'R', 0xea9e22ff, {0.0750000f, 0.0638889f}}, {'R', 0xea9e22ff, {-0.0093750f, 0.0638889f}},
    {'/', 0x685d00ff, {-0.0046875f, 0.1083333f}}, {'\\', 0x685d00ff, {0.0687500f, 0.1083333f}}, {'.', 0xea9e22ff, {0.0171875f, 0.0777778f}}, {'.', 0xea9e22ff, {0.0515625f, 0.0777778f}},
    {'^', 0x685d00ff, {0.0015625f, 0.0805556f}}, {'^', 0x685d00ff, {0.0625000f, 0.0805556f}}, {'.', 0x685d00ff, {0.0593750f, 0.1222222f}}, {'.', 0x685d00ff, {0.0078125f, 0.1222222f}},
    {'<', 0xcf2611ff, {0.0046875f, 0.0166667f}}, {'>', 0xcf2611ff, {0.0609375f, 0.0166667f}}, {'.', 0xcf2611ff, {-0.0015625f, 0.0222222f}}, {'.', 0xcf2611ff, {0.0703125f, 0.0222222f}},
    {',', 0xcf2611ff, {0.0187500f, 0.0444444f}}, {'.', 0xcf2611ff, {0.0500000f, 0.0444444f}}, {'.', 0xcf2611ff, {0.0546875f, 0.0305556f}}, {'.', 0xcf2611ff, {0.0671875f, 0.0444444f}},
    {'.', 0xcf2611ff, {0.0015625f, 0.0444444f}},
};

const ascii2info_t brickdata[66] = {
    {'=', 0x381c01ff, {0.0015625f, 0.1777778f}}, {'\'', 0xd0c0bff, {0.0203125f, 0.1666666f}}, {'=', 0x381c01ff, {0.0375000f, 0.1777778f}}, {'=', 0x381c01ff, {0.0625000f, 0.1777778f}},
    {'\'', 0xd0c0bff, {0.0812500f, 0.1666666f}}, {'=', 0x381c01ff, {0.0984375f, 0.1777778f}}, {'\'', 0xd0c0bff, {-0.0046875f, 0.1333333f}}, {'=', 0x381c01ff, {0.0125000f, 0.1444444f}},
    {'=', 0x381c01ff, {0.0375000f, 0.1444444f}}, {'\'', 0xd0c0bff, {0.0562500f, 0.1333333f}}, {'=', 0x381c01ff, {0.0734375f, 0.1444444f}}, {'=', 0x381c01ff, {0.0984375f, 0.1444444f}},
    {'_', 0xd0c0bff, {0.0015625f, 0.1833333f}}, {'_', 0xd0c0bff, {0.0265625f, 0.1833333f}}, {'-', 0xd0c0bff, {0.0484375f, 0.1611111f}}, {'_', 0xd0c0bff, {0.0734375f, 0.1833333f}},
    {'_', 0xd0c0bff, {0.0984375f, 0.1833333f}}, {'_', 0xd0c0bff, {0.0015625f, 0.1500000f}}, {'_', 0xd0c0bff, {0.0265625f, 0.1500000f}}, {'-', 0xd0c0bff, {0.0484375f, 0.1277778f}},
    {'_', 0xd0c0bff, {0.0734375f, 0.1500000f}}, {'_', 0xd0c0bff, {0.0984375f, 0.1500000f}}, {'=', 0x381c01ff, {0.0015625f, 0.1111111f}}, {'=', 0x381c01ff, {0.0375000f, 0.1111111f}},
    {'=', 0x381c01ff, {0.0625000f, 0.1111111f}}, {'=', 0x381c01ff, {0.0984375f, 0.1111111f}}, {'\'', 0xd0c0bff, {0.0203125f, 0.1000000f}}, {'\'', 0xd0c0bff, {0.0812500f, 0.1000000f}},
    {'_', 0xd0c0bff, {0.0015625f, 0.1166666f}}, {'_', 0xd0c0bff, {0.0265625f, 0.1166666f}}, {'_', 0xd0c0bff, {0.0734375f, 0.1166666f}}, {'_', 0xd0c0bff, {0.0984375f, 0.1166666f}},
    {'-', 0xd0c0bff, {0.0484375f, 0.0944444f}}, {'=', 0x381c01ff, {0.0125000f, 0.0777778f}}, {'=', 0x381c01ff, {0.0375000f, 0.0777778f}}, {'=', 0x381c01ff, {0.0734375f, 0.0777778f}},
    {'=', 0x381c01ff, {0.0984375f, 0.0777778f}}, {'\'', 0xd0c0bff, {0.0562500f, 0.0666666f}}, {'\'', 0xd0c0bff, {-0.0046875f, 0.0666666f}}, {'_', 0xd0c0bff, {0.0015625f, 0.0833333f}},
    {'_', 0xd0c0bff, {0.0265625f, 0.0833333f}}, {'_', 0xd0c0bff, {0.0734375f, 0.0833333f}}, {'_', 0xd0c0bff, {0.0984375f, 0.0833333f}}, {'-', 0xd0c0bff, {0.0484375f, 0.0611111f}},
    {'=', 0x381c01ff, {0.0015625f, 0.0416666f}}, {'=', 0x381c01ff, {0.0375000f, 0.0416666f}}, {'=', 0x381c01ff, {0.0625000f, 0.0416666f}}, {'=', 0x381c01ff, {0.0984375f, 0.0416666f}},
    {'\'', 0xd0c0bff, {0.0203125f, 0.0305555f}}, {'\'', 0xd0c0bff, {0.0812500f, 0.0305555f}}, {'_', 0xd0c0bff, {0.0015625f, 0.0472222f}}, {'_', 0xd0c0bff, {0.0265625f, 0.0472222f}},
    {'_', 0xd0c0bff, {0.0734375f, 0.0472222f}}, {'_', 0xd0c0bff, {0.0984375f, 0.0472222f}}, {'-', 0xd0c0bff, {0.0484375f, 0.0250000f}}, {'=', 0x381c01ff, {0.0125000f, 0.0083333f}},
    {'=', 0x381c01ff, {0.0375000f, 0.0083333f}}, {'=', 0x381c01ff, {0.0734375f, 0.0083333f}}, {'=', 0x381c01ff, {0.0984375f, 0.0083333f}}, {'\'', 0xd0c0bff, {-0.0046875f, -0.0027778f}},
    {'\'', 0xd0c0bff, {0.0562500f, -0.0027778f}}, {'_', 0xd0c0bff, {0.0015625f, 0.0138888f}}, {'_', 0xd0c0bff, {0.0265625f, 0.0138888f}}, {'_', 0xd0c0bff, {0.0734375f, 0.0138888f}},
    {'_', 0xd0c0bff, {0.0984375f, 0.0138888f}}, {'-', 0xd0c0bff, {0.0484375f, -0.0083334f}},
};

const ascii2info_t qblockdata[44] = {
    {'+', 0xea9e22ff, {0.0093750f, 0.1583333f}}, {'+', 0xea9e22ff, {0.0906250f, 0.1583333f}}, {'+', 0xea9e22ff, {0.0906250f, 0.0083333f}}, {'+', 0xea9e22ff, {0.0093750f, 0.0083333f}},
    {'_', 0xe0c0aff, {0.0031250f, 0.0055555f}}, {'_', 0xe0c0aff, {0.0265625f, 0.0055555f}}, {'_', 0xe0c0aff, {0.0515625f, 0.0055555f}}, {'_', 0xe0c0aff, {0.0765625f, 0.0055555f}},
    {'_', 0xe0c0aff, {0.1000000f, 0.0055555f}}, {'_', 0x733400ff, {0.0031250f, 0.2055555f}}, {'_', 0x733400ff, {0.0265625f, 0.2055555f}}, {'_', 0x733400ff, {0.0515625f, 0.2055555f}},
    {'_', 0x733400ff, {0.0765625f, 0.2055555f}}, {'_', 0x733400ff, {0.1000000f, 0.2055555f}}, {'|', 0x733400ff, {-0.0062500f, 0.0111111f}}, {'|', 0x733400ff, {-0.0062500f, 0.0583333f}},
    {'|', 0x733400ff, {-0.0062500f, 0.1083333f}}, {'|', 0x733400ff, {-0.0062500f, 0.1555556f}}, {'|', 0xe0c0aff, {0.1046875f, 0.1555556f}}, {'|', 0xe0c0aff, {0.1046875f, 0.1083333f}},
    {'|', 0xe0c0aff, {0.1046875f, 0.0583333f}}, {'|', 0xe0c0aff, {0.1046875f, 0.0111111f}}, {'?', 0x733400ff, {0.0312500f, 0.1277778f}}, {'?', 0x733400ff, {0.0515625f, 0.1444445f}},
    {'?', 0x733400ff, {0.0687500f, 0.0833334f}}, {'?', 0x733400ff, {0.0515625f, 0.0583334f}}, {'*', 0x733400ff, {0.0515625f, 0.0083333f}}, {'?', 0x733400ff, {0.0718750f, 0.1277778f}},
    {'+', 0xea9e22ff, {0.0296875f, 0.0083333f}}, {'+', 0xea9e22ff, {0.0703125f, 0.0083333f}}, {'+', 0xea9e22ff, {0.0296875f, 0.0833334f}}, {'+', 0xea9e22ff, {0.0093750f, 0.0444445f}},
    {'+', 0xea9e22ff, {0.0703125f, 0.0444445f}}, {'+', 0xea9e22ff, {0.0906250f, 0.0444445f}}, {'+', 0xea9e22ff, {0.0296875f, 0.0444445f}}, {'+', 0xea9e22ff, {0.0906250f, 0.1222223f}},
    {'+', 0xea9e22ff, {0.0093750f, 0.0833334f}}, {'+', 0xea9e22ff, {0.0906250f, 0.0833334f}}, {'+', 0xea9e22ff, {0.0093750f, 0.1222223f}}, {'+', 0xea9e22ff, {0.0484375f, 0.1055556f}},
    {'+', 0xea9e22ff, {0.0296875f, 0.1638889f}}, {'+', 0xea9e22ff, {0.0703125f, 0.1638889f}}, {'-', 0xea9e22ff, {0.0500000f, 0.1722222f}}, {'-', 0xea9e22ff, {0.0500000f, 0.0305556f}},
};

//
//  Animation callbacks
//
static void animateFlag(gameobj_t* restrict flg)
{
    if (!flg)
        return;

    u64 t = SDL_GetTicks();
    u32 k = 0;

    for (asciidata_t* i = flg->data; i < flg->data + flg->len; i++, k++)
        i->y = flagdata[k].pos.y + flg->y + (sinf(((f32) t / 780.0f) + (i->x * 40.0f)) * FLAG_Y_OFFSET);
}

static void animateBird(gameobj_t* restrict bird)
{
    static u32 bcounter;

    if (!bird) {
        bcounter = 0;
        return;
    }

    if (bcounter == 0) {
        bird->data[0].y -= 0.006f;
        bird->data[1].y -= 0.003f;
        bcounter++;
        return;
    }

    if (bcounter == 3) {
        bird->data[0].y += 0.006f;
        bird->data[1].y += 0.003f;
        bcounter++;
        return;
    }

    if (++bcounter > 8)
        bcounter = 0;
}

//
//  Scenes
//
const sceneinfo_t scene_bird = {
    .kbmapping = set2DLayerInputMap,
    .rendermode = RENDER_MODE_LAYERED,
    .flags = SCENE_RENDER_ALL,
    .player_x = 0.7f,
    .player_y = 1.4f,
    .player = &(objectinfo_t) {
        .data = birddata,
        .len = sizeof(birddata) / sizeof(ascii2info_t),
        .xscale = BIRD_X_SCALE,
        .yscale = BIRD_Y_SCALE,
        .hitbox_dy = -BIRD_Y_SCALE,
        .numanimations = 1,
        .animations = (animinfo_t[]) {{
            .type = ANIM_TYPE_CALLBACK,
            .flags = ANIM_AUTOQ_SUSPEND | ANIM_RESET_POS | ANIM_RESET_CB | ANIM_KEEPALIVE,
            .ticks = 12,
            .callback = animateBird
        }}
    },
    .numobjects = 15,
    .objects = (objectinfo_t[15]) {{
        .data = flagdata,
        .len = sizeof(flagdata) / sizeof(ascii2info_t),
        .x = 0.1f,
        .y = 1.75f,
        .numanimations = 1,
        .animations = (animinfo_t[]) {{
            .type = ANIM_TYPE_CALLBACK,
            .flags = ANIM_AUTOQ_REPEAT,
            .ticks = 0,
            .callback = animateFlag
        }}
    }, {
        .data = flagpoledata,
        .len = sizeof(flagpoledata) / sizeof(ascii2info_t),
        .x = 0.056f,
        .y = 1.664f
    }, {
        .data = terraindata,
        .len = sizeof(terraindata) / sizeof(ascii2info_t),
        .x = 1.488f,
        .y = 0.66f
    }, {
        .data = cherrydata,
        .len = sizeof(cherrydata) / sizeof(ascii2info_t),
        .x = 0.8f,
        .y = 0.3f,
        .xscale = CHERRY_X_SCALE,
        .yscale = CHERRY_Y_SCALE
    }, {
        .data = cherrydata,
        .len = sizeof(cherrydata) / sizeof(ascii2info_t),
        .x = 1.3f,
        .y = 1.7f,
        .xscale = CHERRY_X_SCALE,
        .yscale = CHERRY_Y_SCALE
    }, {
        .data = cherrydata,
        .len = sizeof(cherrydata) / sizeof(ascii2info_t),
        .x = 0.9f,
        .y = 1.6f,
        .xscale = CHERRY_X_SCALE,
        .yscale = CHERRY_Y_SCALE
    }, {
        .data = cherrydata,
        .len = sizeof(cherrydata) / sizeof(ascii2info_t),
        .x = 1.0f,
        .y = 1.0f,
        .xscale = CHERRY_X_SCALE,
        .yscale = CHERRY_Y_SCALE
    }, {
        .data = pillardata,
        .len = sizeof(pillardata) / sizeof(ascii2info_t),
        .x = 0.4f,
        .y = 1.0f
    }, {
        .data = pillardata,
        .len = sizeof(pillardata) / sizeof(ascii2info_t),
        .x = 1.4f,
        .y = 1.0f
    }, {
        .data = pillardata,
        .len = sizeof(pillardata) / sizeof(ascii2info_t),
        .x = 0.4f,
        .y = 1.0f
    }, {
        .data = pillardata,
        .len = sizeof(pillardata) / sizeof(ascii2info_t),
        .x = 1.4f,
        .y = 1.0f
    }, {
        .data = pillardata,
        .len = sizeof(pillardata) / sizeof(ascii2info_t),
        .x = 0.4f,
        .y = 1.0f
    }, {
        .data = pillardata,
        .len = sizeof(pillardata) / sizeof(ascii2info_t),
        .x = 1.4f,
        .y = 1.0f
    }, {
        .data = pillardata,
        .len = sizeof(pillardata) / sizeof(ascii2info_t),
        .x = 0.4f,
        .y = 1.0f
    }, {
        .data = pillardata,
        .len = sizeof(pillardata) / sizeof(ascii2info_t),
        .x = 1.4f,
        .y = 1.0f
    }},
    .layers = (i8[16]) { 3, 3, 4, 0, 1, 2, 3, 1, 1, 2, 2, 3, 3, 4, 4, 5 }
};

const sceneinfo_t scene_mario = {
    .kbmapping = setStdInputMap,
    .flags = SCENE_RENDER_ALL | SCENE_NO_RENDERMODE,
    .player_x = 1.0f,
    .player_y = 1.0f,
    .player = &(objectinfo_t) {
        .data = mariodata,
        .len = sizeof(mariodata) / sizeof(ascii2info_t),
        .xscale = 0.094f,
        .yscale = 0.25f
    },
    .numobjects = 14,
    .objects = (objectinfo_t[]) {{
        .data = brickdata,
        .len = sizeof(brickdata) / sizeof(ascii2info_t),
        .x = 0.1f,
        .y = 0.2f
    }, {
        .data = brickdata,
        .len = sizeof(brickdata) / sizeof(ascii2info_t),
        .x = 0.227f,
        .y = 0.2f
    }, {
        .data = brickdata,
        .len = sizeof(brickdata) / sizeof(ascii2info_t),
        .x = 0.354f,
        .y = 0.2f
    }, {
        .data = brickdata,
        .len = sizeof(brickdata) / sizeof(ascii2info_t),
        .x = 0.481f,
        .y = 0.2f
    }, {
        .data = brickdata,
        .len = sizeof(brickdata) / sizeof(ascii2info_t),
        .x = 0.608f,
        .y = 0.2f
    }, {
        .data = brickdata,
        .len = sizeof(brickdata) / sizeof(ascii2info_t),
        .x = 0.735,
        .y = 0.2f
    }, {
        .data = brickdata,
        .len = sizeof(brickdata) / sizeof(ascii2info_t),
        .x = 0.862f,
        .y = 0.2f
    }, {
        .data = brickdata,
        .len = sizeof(brickdata) / sizeof(ascii2info_t),
        .x = 0.989f,
        .y = 0.2f
    }, {
        .data = brickdata,
        .len = sizeof(brickdata) / sizeof(ascii2info_t),
        .x = 1.116f,
        .y = 0.2f
    }, {
        .data = brickdata,
        .len = sizeof(brickdata) / sizeof(ascii2info_t),
        .x = 1.243f,
        .y = 0.2f
    }, {
        .data = brickdata,
        .len = sizeof(brickdata) / sizeof(ascii2info_t),
        .x = 1.370f,
        .y = 0.2f
    }, {
        .data = brickdata,
        .len = sizeof(brickdata) / sizeof(ascii2info_t),
        .x = 1.497f,
        .y = 0.2f
    }, {
        .data = brickdata,
        .len = sizeof(brickdata) / sizeof(ascii2info_t),
        .x = 1.624f,
        .y = 0.2f
    }, {
        .data = qblockdata,
        .len = sizeof(qblockdata) / sizeof(ascii2info_t),
        .x = 0.481,
        .y = 0.8f
    }}
};

//
//  3D Objects
//
static const vec3f_t pyramid_vtx[18] = {
    {-1, 0, -1, 0xea9e22ff}, {1, 0, -1, 0xea9e22ff}, {-1, 0, 1, 0xea9e22ff},
    {1, 0, 1, 0xea9e22ff}, {-1, 0, 1, 0xea9e22ff}, {1, 0, -1, 0xea9e22ff},
    {-1, 0, -1, 0xea9e22ff}, {-1, 0, 1, 0xea9e22ff}, {0, 1, 0, 0xFFFFFFFF},
    {-1, 0, -1, 0xea9e22ff}, {0, 1, 0, 0xFFFFFFFF}, {1, 0, -1, 0xea9e22ff},
    {1, 0, 1, 0xea9e22ff}, {0, 1, 0, 0xFFFFFFFF}, {-1, 0, 1, 0xea9e22ff},
    {1, 0, 1, 0xea9e22ff}, {1, 0, -1, 0xea9e22ff}, {0, 1, 0, 0xFFFFFFFF}
};

static obj3D_t pyramid_obj = {
    .numvtx = sizeof(pyramid_vtx) / sizeof(vec3f_t),
    .vtxbuf = pyramid_vtx,
    .flags = OBJECT_NEED_REBUILD | OBJECT_VISIBLE | OBJECT_RENDER
};

obj3D_t* pyramid_3D = &pyramid_obj;

//

static const vec3f_t terrain_vtx[24] = {
    {-8, 0, -8, 0xea9e22ff}, {-8, 0, -7, 0xea9e22ff}, {-7, 0, -8, 0xea9e22ff},
    {-7, 0, -7, 0xea9e22ff}, {-7, 0, -8, 0xea9e22ff}, {-8, 0, -7, 0xea9e22ff},

    {-7, 0, -7, 0xea9e22ff}, {-7, 0, -6, 0xea9e22ff}, {-6, 0, -7, 0xea9e22ff},
    {-6, 0, -6, 0xea9e22ff}, {-6, 0, -7, 0xea9e22ff}, {-7, 0, -6, 0xea9e22ff},

    {-6, 0, -6, 0xea9e22ff}, {-6, 0, -5, 0xea9e22ff}, {-5, 0, -6, 0xea9e22ff},
    {-5, 0, -5, 0xea9e22ff}, {-5, 0, -6, 0xea9e22ff}, {-6, 0, -5, 0xea9e22ff},

    {-5, 0, -5, 0xea9e22ff}, {-5, 0, -4, 0xea9e22ff}, {-4, 0, -5, 0xea9e22ff},
    {-4, 0, -4, 0xea9e22ff}, {-4, 0, -5, 0xea9e22ff}, {-5, 0, -4, 0xea9e22ff},
};

static obj3D_t terrain_obj = {
    .numvtx = sizeof(terrain_vtx) / sizeof(vec3f_t),
    .vtxbuf = terrain_vtx,
    .flags = OBJECT_NEED_REBUILD | OBJECT_VISIBLE | OBJECT_RENDER
};

obj3D_t* terrain_3D = &terrain_obj;

//

static const vec3f_t wall_vtx[6] = {
    {0, 0, 0, 0x191919ff}, {1.92f, 0, 0, 0x191919ff}, {1.92f, 1.08f, 0, 0x191919ff},
    {0, 0, 0, 0x191919ff}, {1.92f, 1.08f, 0, 0x191919ff}, {0, 1.08f, 0, 0x191919ff}
};

static const vec3f_t wall2_vtx[24] = {
    {0, 0, 0, 0x191919ff}, {0.48f, 0, 0, 0x191919ff}, {0.48f, 1.08f, 0, 0x191919ff},
    {0, 0, 0, 0x191919ff}, {0.48f, 1.08f, 0, 0x191919ff}, {0, 1.08f, 0, 0x191919ff},
    {0.48f, 0, 0, 0x191919ff}, {0.93f, 0, -0.2f, 0x191919ff}, {0.93f, 1.08f, -0.2f, 0x191919ff},
    {0.48f, 0, 0, 0x191919ff}, {0.93f, 1.08f, -0.2f, 0x191919ff}, {0.48f, 1.08f, 0, 0x191919ff},
    {0.93f, 0, -0.2f, 0x191919ff}, {1.27f, 0, -0.7f, 0x191919ff}, {1.27f, 1.08f, -0.7f, 0x191919ff},
    {0.93f, 0, -0.2f, 0x191919ff}, {1.27f, 1.08f, -0.7f, 0x191919ff}, {0.93f, 1.08f, -0.2f, 0x191919ff},
    {1.27f, 0, -0.7f, 0x191919ff}, {1.52f, 0, -1.5f, 0x191919ff}, {1.52f, 1.08f, -1.5f, 0x191919ff},
    {1.27f, 0, -0.7f, 0x191919ff}, {1.52f, 1.08f, -1.5f, 0x191919ff}, {1.27f, 1.08f, -0.7f, 0x191919ff}
};

static obj3D_t wall_obj = {
    .numvtx = sizeof(wall_vtx) / sizeof(vec3f_t),
    .vtxbuf = wall_vtx,
    .flags = OBJECT_NEED_REBUILD | OBJECT_VISIBLE | OBJECT_RENDER,
    .pos = {-4.0f, 0, -4.0f}
};

obj3D_t* wall_3D = &wall_obj;

const f32 wall_vtx_uv[30] = {
    0, 0, 0, 0, 1,   1.92f, 0, 0, 1, 1,   1.92f, 1.08f, 0, 1, 0,
    0, 0, 0, 0, 1,   1.92f, 1.08f, 0, 1, 0,   0, 1.08f, 0, 0, 0
};

const f32 wall2_vtx_uv[120] = {
    0, 0, 0,  0, 1,   0.48f, 0, 0,  0.25f, 1,        0.48f, 1.08f, 0,  0.25f, 0,
    0, 0, 0,  0, 1,   0.48f, 1.08f, 0,  0.25f, 0,    0, 1.08f, 0,  0, 0,

    0.48f, 0, 0,  0.25f, 1,   0.93f, 0, -0.2f,  0.5f, 1,       0.93f, 1.08f, -0.2f,  0.5f, 0,
    0.48f, 0, 0,  0.25f, 1,   0.93f, 1.08f, -0.2f,  0.5f, 0,   0.48f, 1.08f, 0,  0.25f, 0,
    0.93f, 0, -0.2f,  0.5f, 1,   1.27f, 0, -0.7f,  0.75f, 1,       1.27f, 1.08f, -0.7f,  0.75f, 0,
    0.93f, 0, -0.2f,  0.5f, 1,   1.27f, 1.08f, -0.7f,  0.75f, 0,   0.93f, 1.08f, -0.2f,  0.5f, 0,
    1.27f, 0, -0.7f,  0.75f, 1,   1.52f, 0, -1.5f,  1, 1,       1.52f, 1.08f, -1.5f,  1, 0,
    1.27f, 0, -0.7f,  0.75f, 1,   1.52f, 1.08f, -1.5f,  1, 0,   1.27f, 1.08f, -0.7f,  0.75f, 0,
};

const mat4_t wall2_transform = {
    1,  0,  0, 0,
    0,  1,  0, 0,
    0,  0,  1, 0,
    -7.0f, 0, 0.0f, 1
};

//

static const vec3f_t player_vtx[12] = {
    {0, 0, 0}, {0.06f, 0, 0}, {0.06f, 0.5f, 0},
    {0, 0, 0}, {0.06f, 0.5f, 0}, {0, 0.5f, 0},
    {0, 0, 0}, {0.06f, 0.5f, 0}, {0.06f, 0, 0},
    {0, 0, 0}, {0, 0.5f, 0}, {0.06f, 0.5f, 0}
};

static obj3D_t player_obj = {
    .numvtx = sizeof(player_vtx) / sizeof(vec3f_t),
    .vtxbuf = player_vtx,
    .flags = OBJECT_NEED_REBUILD | OBJECT_VISIBLE | OBJECT_RENDER,
    .pos = {0, 0, 2.0f}
};

// array
obj3D_t objects3D[6] = {{
        .numvtx = sizeof(terrain_vtx) / sizeof(vec3f_t),
        .vtxbuf = terrain_vtx,
        .flags = OBJECT_NEED_REBUILD | OBJECT_VISIBLE | OBJECT_RENDER
    }, {
        .numvtx = sizeof(wall_vtx) / sizeof(vec3f_t),
        .vtxbuf = wall_vtx,
        .flags = OBJECT_NEED_REBUILD | OBJECT_VISIBLE | OBJECT_RENDER,
        .pos = {-4.0f, 0, -4.0f},
        .tag = TAG_WALL
    }, {
        .numvtx = sizeof(pyramid_vtx) / sizeof(vec3f_t),
        .vtxbuf = pyramid_vtx,
        .flags = OBJECT_NEED_REBUILD | OBJECT_VISIBLE | OBJECT_RENDER
    }, {
        .numvtx = sizeof(player_vtx) / sizeof(vec3f_t),
        .vtxbuf = player_vtx,
        .flags = OBJECT_NEED_REBUILD | OBJECT_VISIBLE | OBJECT_RENDER,
        .pos = {0, 0, 2.0f},
        .tag = TAG_PLAYER
    }, {
        .flags = OBJECT_NEED_REBUILD | OBJECT_VISIBLE | OBJECT_RENDER | OBJECT_INCOMPLETE,
        .pos = {3.0f, 0.25f, -6.0f},
        .tag = TAG_JET
    }, {
        .numvtx = sizeof(wall2_vtx) / sizeof(vec3f_t),
        .vtxbuf = wall2_vtx,
        .flags = OBJECT_NEED_REBUILD | OBJECT_VISIBLE | OBJECT_RENDER,
        .pos = {-7.0f, 0, 0},
        .tag = TAG_WALL2
    }
};

obj3D_t objects3D2[5] = {{
        .numvtx = sizeof(terrain_vtx) / sizeof(vec3f_t),
        .vtxbuf = terrain_vtx,
        .flags = OBJECT_NEED_REBUILD | OBJECT_VISIBLE | OBJECT_RENDER
    }, {
        .numvtx = sizeof(wall_vtx) / sizeof(vec3f_t),
        .vtxbuf = wall_vtx,
        .flags = OBJECT_NEED_REBUILD | OBJECT_VISIBLE | OBJECT_RENDER,
        .pos = {-4.0f, 0, -4.0f},
        .tag = TAG_WALL
    }, {
        .numvtx = sizeof(pyramid_vtx) / sizeof(vec3f_t),
        .vtxbuf = pyramid_vtx,
        .flags = OBJECT_NEED_REBUILD | OBJECT_VISIBLE | OBJECT_RENDER,
        .pos = {7.0f, 0.0f, 0.0f}
    }, {
        .flags = OBJECT_NEED_REBUILD | OBJECT_VISIBLE | OBJECT_RENDER | OBJECT_INCOMPLETE | OBJECT_RENDER_DEBUG,
        .pos = {2.0f, 0.2f, 4.0f},
        .tag = TAG_JET
    }, {
        .numvtx = sizeof(wall2_vtx) / sizeof(vec3f_t),
        .vtxbuf = wall2_vtx,
        .flags = OBJECT_NEED_REBUILD | OBJECT_VISIBLE | OBJECT_RENDER,
        .pos = {-7.0f, 0, 0},
        .tag = TAG_WALL2
    }
};

obj3D_t* getObjectByTag3D(tag objtag, obj3D_t* objects, u32 len)
{
    const obj3D_t* bound = objects + len;

    for (obj3D_t* i = objects; i < bound; i++) {
        if (i->tag == objtag)
            return i;
    }

    SDL_Log("[ERROR] Could not find object with tag %ld", objtag);

    return NULL;
}

static inline void winding(vec3f_t* triangle, vec3f_t normal)
{
    vec3f_t a = {triangle[1].x - triangle[0].x, triangle[1].y - triangle[0].y, triangle[1].z - triangle[0].z};
    vec3f_t b = {triangle[2].x - triangle[0].x, triangle[2].y - triangle[0].y, triangle[2].z - triangle[0].z};

    vec3f_t n = v3Cross(a, b);

    if ((n.x > 0.0f && normal.x < 0.0f) || (n.x < 0.0f && normal.x > 0.0f))
        goto swap;

    if ((n.y > 0.0f && normal.y < 0.0f) || (n.y < 0.0f && normal.y > 0.0f))
        goto swap;

    if ((n.z > 0.0f && normal.z < 0.0f) || (n.z < 0.0f && normal.z > 0.0f))
        goto swap;

    return;

    swap:

    n = triangle[1];
    triangle[1] = triangle[2];
    triangle[2] = n;
}

vec3f_t* parseStl(const char* filename, u32* numvtx, u32 color)
{
    rAssert(filename);
    rAssert(numvtx);

    char buf[65535];

    static context_t* context;

    if (!context)
        context = getContext();

    rAssert(context->path);

    snprintf(buf, 512, "%s..\\resources\\models\\%s.stl", context->path, filename);

    FILE* fd = fopen(buf, "rb");

    if (!fd || ferror(fd)) {
        SDL_Log("[ERROR] Failed to parse STL: file open error");
        fclose(fd);
        return NULL;
    }

    size_t len = fread(buf, 1, 65535, fd);

    if (!feof(fd)) {
        SDL_Log("[ERROR] Failed to parse STL: file too big");
        fclose(fd);
        return NULL;
    }

    fclose(fd);

    if (len < 100) {
        SDL_Log("[ERROR] Failed to parse STL: file read error");
        return NULL;
    }

    u32 num = *((u32*) (buf + 80));

    rAssert(num);
    rAssert(num < 4096);

    *numvtx = num * 3;

    vec3f_t* ret = (vec3f_t*) memAlloc(num * 3 * sizeof(vec3f_t));
    vec3f_t* tmp = ret;

    for (const char* i = buf + 84; i < buf + len && num; i += 50, tmp += 3, num--) {
        rAssert(i + 48 < buf + len);

        vec3f_t n;

        n.x = *((const f32*) i);
        n.y = *((const f32*) (i + 4));
        n.z = *((const f32*) (i + 8));

        tmp[0].x = *((const f32*) (i + 12));
        tmp[0].y = *((const f32*) (i + 16));
        tmp[0].z = *((const f32*) (i + 20));

        tmp[2].x = *((const f32*) (i + 24));
        tmp[2].y = *((const f32*) (i + 28));
        tmp[2].z = *((const f32*) (i + 32));

        tmp[1].x = *((const f32*) (i + 36));
        tmp[1].y = *((const f32*) (i + 40));
        tmp[1].z = *((const f32*) (i + 44));

        winding(tmp, n);

        tmp[0].pad = color;

        u8 c = ((u8) (color >> 8)) + 16;

        tmp[1].pad = (c << 24) | (c << 16) | (c << 8) | 0xFF;

        c += 16;

        tmp[2].pad = (c << 24) | (c << 16) | (c << 8) | 0xFF;
    }

    rAssert(!num);

    return ret;
}