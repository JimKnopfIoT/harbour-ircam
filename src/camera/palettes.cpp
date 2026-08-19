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
      << QStringLiteral("White hot") << QStringLiteral("Black hot") << QStringLiteral("Grey + red");
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
