// Minimal CardGallery shim for the Linux PoC.
#ifndef CARDGALLERY_H
#define CARDGALLERY_H

class CardGallery
{
public:
    static CardGallery* GetInstance();
    static void DestroyInstance();

    void Init();

private:
    CardGallery();
    ~CardGallery();

    static CardGallery* spInstance;
};

inline CardGallery* GetCardGallery() { return CardGallery::GetInstance(); }

#endif // CARDGALLERY_H
