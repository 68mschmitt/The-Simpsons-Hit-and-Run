// Minimal Pure3D type shim for the Linux PoC.
#ifndef P3D_P3DTYPES_HPP
#define P3D_P3DTYPES_HPP

struct tColour
{
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char alpha;

    constexpr tColour(unsigned char r = 0,
                      unsigned char g = 0,
                      unsigned char b = 0,
                      unsigned char a = 255)
        : red(r), green(g), blue(b), alpha(a)
    {
    }
};

#endif // P3D_P3DTYPES_HPP
