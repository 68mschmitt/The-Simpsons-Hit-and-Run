// Minimal RenderManager shim for the Linux PoC.
#ifndef RENDERMANAGER_H
#define RENDERMANAGER_H

#include <render/enums/renderenums.h>
#include <render/rendermanager/renderlayer.h>

class RenderManager
{
public:
    static RenderManager* GetInstance();
    static void DestroyInstance();

    RenderLayer* mpLayer( RenderEnums::LayerEnum layer );

private:
    RenderManager();
    ~RenderManager();

    static RenderManager* spInstance;
    RenderLayer mLayer;
};

inline RenderManager* GetRenderManager() { return RenderManager::GetInstance(); }

#endif // RENDERMANAGER_H
