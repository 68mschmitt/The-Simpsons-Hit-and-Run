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

// Minimal ref-counted shader stand-in.  BootupContext only needs AddRef/Release
// with delete-on-zero semantics for its shared "simple" shader.
class pddiShader
{
public:
    pddiShader() : mRefCount(0) {}
    virtual ~pddiShader() {}

    void AddRef() { ++mRefCount; }
    void Release()
    {
        if(--mRefCount <= 0)
        {
            delete this;
        }
    }

    int GetRefCount() const { return mRefCount; }

private:
    int mRefCount;
};

#endif // PDDI_PDDI_HPP
