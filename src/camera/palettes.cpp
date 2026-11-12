// palettes.cpp — see palettes.h. GPLv3+, © JimKnopfIoT.
#include "palettes.h"

namespace {

struct Stop { double t; int r, g, b; };

// Control points; each list runs t=0..1.
const Stop LAVA[] = {
    {0.00,   0,   0,   4}, {0.14,  40,  11,  84}, {0.29, 101,  21, 110},
    {0.43, 159,  42,  99}, {0.57, 212,  72,  66}, {0.71, 245, 125,  21},
    {0.86, 250, 193,  39}, {1.00, 252, 255, 164},
};
const Stop IRONBOW[] = {
    {0.00,   0,   0,   0}, {0.13,  32,   0,  96}, {0.30, 140,   0, 140},
    {0.50, 220,  48,  64}, {0.70, 255, 140,   0}, {0.85, 255, 220,  48},
    {1.00, 255, 255, 255},
};
const Stop RAINBOW[] = {
    {0.00,   0,   0, 140}, {0.25,   0, 160, 255}, {0.50,   0, 230,   0},
    {0.75, 255, 220,   0}, {1.00, 255,   0,   0},
};
const Stop WHITEHOT[] = { {0.0, 0, 0, 0}, {1.0, 255, 255, 255} };
const Stop BLACKHOT[] = { {0.0, 255, 255, 255}, {1.0, 0, 0, 0} };
const Stop GREYRED[] = {
    {0.00,   0,   0,   0}, {0.80, 235, 235, 235}, {0.87, 255, 255, 255},
    {0.90, 255,  90,  40}, {1.00, 255,   0,   0},
};

// Standard colour maps, sampled from matplotlib's definitions:
// Turbo © Google LLC (Apache-2.0); Magma/Plasma/Viridis by Smith & van der Walt (CC0);
// Hot is the classic black-red-yellow-white ramp. Suggested by the palette list of
// IRCAM Thermal Viewer (OpenRMH-IRCAM).
const Stop TURBO[] = {
    {0.000,  48,  18,  59}, {0.062,  64,  64, 162}, {0.125,  70, 107, 227},
    {0.188,  66, 148, 255}, {0.250,  40, 188, 235}, {0.312,  24, 221, 194},
    {0.375,  50, 242, 152}, {0.438, 109, 254,  98}, {0.500, 164, 252,  60},
    {0.562, 205, 236,  52}, {0.625, 238, 207,  58}, {0.688, 253, 172,  52},
    {0.750, 251, 126,  33}, {0.812, 235,  80,  14}, {0.875, 208,  47,   5},
    {0.938, 169,  22,   1}, {1.000, 122,   4,   3},
};
const Stop MAGMA[] = {
    {0.000,   0,   0,   4}, {0.062,  10,   8,  34}, {0.125,  29,  17,  71},
    {0.188,  54,  16, 107}, {0.250,  81,  18, 124}, {0.312, 106,  28, 129},
    {0.375, 131,  38, 129}, {0.438, 156,  46, 127}, {0.500, 183,  55, 121},
    {0.562, 208,  65, 111}, {0.625, 231,  82,  99}, {0.688, 245, 107,  92},
    {0.750, 252, 137,  97}, {0.812, 254, 167, 114}, {0.875, 254, 196, 136},
    {0.938, 253, 226, 163}, {1.000, 252, 253, 191},
};
const Stop PLASMA[] = {
    {0.000,  13,   8, 135}, {0.062,  49,   5, 151}, {0.125,  76,   2, 161},
    {0.188, 102,   0, 167}, {0.250, 126,   3, 168}, {0.312, 149,  17, 161},
    {0.375, 170,  35, 149}, {0.438, 188,  53, 135}, {0.500, 204,  71, 120},
    {0.562, 218,  90, 106}, {0.625, 230, 108,  92}, {0.688, 240, 128,  78},
    {0.750, 248, 149,  64}, {0.812, 253, 172,  51}, {0.875, 253, 197,  39},
    {0.938, 248, 223,  37}, {1.000, 240, 249,  33},
};
const Stop VIRIDIS[] = {
    {0.000,  68,   1,  84}, {0.062,  72,  24, 106}, {0.125,  71,  45, 123},
    {0.188,  66,  64, 134}, {0.250,  59,  82, 139}, {0.312,  51,  99, 141},
    {0.375,  44, 114, 142}, {0.438,  38, 130, 142}, {0.500,  33, 145, 140},
    {0.562,  31, 160, 136}, {0.625,  40, 174, 128}, {0.688,  63, 188, 115},
    {0.750,  94, 201,  98}, {0.812, 132, 212,  75}, {0.875, 173, 220,  48},
    {0.938, 216, 226,  25}, {1.000, 253, 231,  37},
};
const Stop HOT[] = {
    {0.000,  11,   0,   0}, {0.125,  95,   0,   0}, {0.250, 179,   0,   0},
    {0.375, 255,   8,   0}, {0.500, 255,  92,   0}, {0.625, 255, 176,   0},
    {0.750, 255, 255,   7}, {0.875, 255, 255, 133}, {1.000, 255, 255, 255},
};

struct Table {
    const Stop *stops;
    int count;
    QRgb lut[256];
    bool ready;
};

Table g_tables[palettes::Count] = {
    { LAVA,     int(sizeof(LAVA)     / sizeof(Stop)), {}, false },
    { IRONBOW,  int(sizeof(IRONBOW)  / sizeof(Stop)), {}, false },
    { RAINBOW,  int(sizeof(RAINBOW)  / sizeof(Stop)), {}, false },
    { WHITEHOT, int(sizeof(WHITEHOT) / sizeof(Stop)), {}, false },
    { BLACKHOT, int(sizeof(BLACKHOT) / sizeof(Stop)), {}, false },
    { GREYRED,  int(sizeof(GREYRED)  / sizeof(Stop)), {}, false },
    { TURBO,    int(sizeof(TURBO)    / sizeof(Stop)), {}, false },
    { MAGMA,    int(sizeof(MAGMA)    / sizeof(Stop)), {}, false },
    { PLASMA,   int(sizeof(PLASMA)   / sizeof(Stop)), {}, false },
    { VIRIDIS,  int(sizeof(VIRIDIS)  / sizeof(Stop)), {}, false },
    { HOT,      int(sizeof(HOT)      / sizeof(Stop)), {}, false },
};

void build(Table &tb)
{
    for (int i = 0; i < 256; ++i) {
        const double t = i / 255.0;
        int k = 0;
        while (k < tb.count - 2 && t > tb.stops[k + 1].t)
            ++k;
        const Stop &a = tb.stops[k];
        const Stop &b = tb.stops[k + 1];
        double f = (t - a.t) / (b.t - a.t);
        if (f < 0) f = 0;
        if (f > 1) f = 1;
        tb.lut[i] = qRgb(int(a.r + f * (b.r - a.r)),
                         int(a.g + f * (b.g - a.g)),
                         int(a.b + f * (b.b - a.b)));
    }
    tb.ready = true;
}

int clampId(int id)
{
    if (id < 0 || id >= palettes::Count)
        return palettes::Lava;
    return id;
}

} // namespace

namespace palettes {

QStringList names()
{
    QStringList n;
    n << QStringLiteral("Lava") << QStringLiteral("Ironbow") << QStringLiteral("Rainbow")
      << QStringLiteral("White hot") << QStringLiteral("Black hot") << QStringLiteral("Grey + red")
      << QStringLiteral("Turbo") << QStringLiteral("Magma") << QStringLiteral("Plasma")
      << QStringLiteral("Viridis") << QStringLiteral("Hot");
    return n;
}

const QRgb *lut(int id)
{
    Table &tb = g_tables[clampId(id)];
    if (!tb.ready)
        build(tb);
    return tb.lut;
}

QColor colorAt(int id, double t)
{
    if (t < 0) t = 0;
    if (t > 1) t = 1;
    return QColor(lut(id)[int(t * 255.0 + 0.5)]);
}

} // namespace palettes
