// Minimal PDDI shim for the Linux PoC.
#ifndef PDDI_PDDI_HPP
#define PDDI_PDDI_HPP

struct pddiColour
{
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char alpha;

    pddiColour(unsigned char r = 0, unsigned char g = 0, unsigned char b = 0, unsigned char a = 255)
        : red(r), green(g), blue(b), alpha(a)
    {
    }

    void Set(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255)
    {
        red = r;
        green = g;
        blue = b;
        alpha = a;
    }
};

#endif // PDDI_PDDI_HPP
