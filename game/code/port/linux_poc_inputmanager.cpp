//=============================================================================
// Linux PoC InputManager no-op implementation.
//=============================================================================

#include <input/inputmanager.h>

#include <raddebug.hpp>

InputManager* InputManager::spInstance = nullptr;

InputManager* InputManager::CreateInstance()
{
    rAssertMsg(spInstance == nullptr, "Trying to create more than one InputManager instance");
    spInstance = new InputManager;
    return spInstance;
}

InputManager* InputManager::GetInstance()
{
    return spInstance;
}

void InputManager::DestroyInstance()
{
    delete spInstance;
    spInstance = nullptr;
}

void InputManager::Init()
{
    rReleasePrintf("InputManager initialized (Linux PoC stub)\n");
}

void InputManager::Update(unsigned int timeinms)
{
    (void)timeinms;
}

bool InputManager::IsControllerInPort(int portnum) const
{
    return portnum >= 0 && portnum < static_cast<int>(Input::MaxControllers);
}

void InputManager::ToggleRumble(bool on)
{
    mRumbleEnabled = on;
}

void InputManager::SetRumbleForDevice(int controllerId, bool bRumbleOn)
{
    (void)controllerId;
    mRumbleEnabled = bRumbleOn;
}

bool InputManager::IsRumbleOnForDevice(int controllerId) const
{
    (void)controllerId;
    return mRumbleEnabled;
}

void InputManager::TriggerRumblePulse(int controllerId)
{
    (void)controllerId;
}

void InputManager::SetRumbleEnabled(bool isEnabled)
{
    mRumbleEnabled = isEnabled;
}

bool InputManager::IsRumbleEnabled() const
{
    return mRumbleEnabled;
}

float InputManager::GetValue(unsigned int controllerIndex, unsigned int inputIndex) const
{
    (void)controllerIndex;
    (void)inputIndex;
    return 0.0f;
}

UserController* InputManager::GetController(unsigned int controllerIndex)
{
    (void)controllerIndex;
    return nullptr;
}

int InputManager::RegisterMappable(unsigned int index, Mappable* pMappable)
{
    (void)index;
    (void)pMappable;
    return 0;
}

void InputManager::UnregisterMappable(unsigned int index, int handle)
{
    (void)index;
    (void)handle;
}

void InputManager::UnregisterMappable(unsigned int index, Mappable* pMappable)
{
    (void)index;
    (void)pMappable;
}

void InputManager::UnregisterMappable(Mappable* pMappable)
{
    (void)pMappable;
}

void InputManager::SetGameState(Input::ActiveState state)
{
    mGameState = state;
}

Input::ActiveState InputManager::GetGameState() const
{
    return mGameState;
}

void InputManager::RegisterControllerID(int playerID, int controllerID)
{
    if(playerID >= 0 && playerID < 4)
    {
        mRegisteredControllerID[playerID] = controllerID;
    }
}

void InputManager::UnregisterControllerID(int playerID)
{
    if(playerID >= 0 && playerID < 4)
    {
        mRegisteredControllerID[playerID] = -1;
    }
}

void InputManager::UnregisterAllControllerID()
{
    for(int& id : mRegisteredControllerID)
    {
        id = -1;
    }
}

int InputManager::GetControllerIDforPlayer(int playerID) const
{
    if(playerID >= 0 && playerID < 4)
    {
        return mRegisteredControllerID[playerID];
    }

    return -1;
}

int InputManager::GetControllerPlayerIDforController(int controllerIndex) const
{
    for(int i = 0; i < 4; ++i)
    {
        if(mRegisteredControllerID[i] == controllerIndex)
        {
            return i;
        }
    }

    return -1;
}

InputManager::InputManager()
    : mGameState(Input::ACTIVE_NONE),
      mRumbleEnabled(false),
      mResetEnabled(false),
      mRegisteredControllerID{-1, -1, -1, -1}
{
}

InputManager::~InputManager()
{
    rReleasePrintf("InputManager shut down (Linux PoC stub)\n");
}
