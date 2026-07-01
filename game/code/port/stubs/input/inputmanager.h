// Minimal InputManager shim for the Linux PoC.
#ifndef InputManager_HPP
#define InputManager_HPP

class Mappable;
class UserController;

namespace Input
{
    enum ActiveState
    {
        ACTIVE_NONE = 0
    };

    constexpr unsigned int MaxControllers = 4;
}

class InputManager
{
public:
    static InputManager* CreateInstance();
    static InputManager* GetInstance();
    static void DestroyInstance();

    void Init();
    void Update(unsigned int timeinms);

    bool IsControllerInPort(int portnum) const;
    static unsigned int GetMaxControllers() { return Input::MaxControllers; }
    const char* GetName() const { return "Linux PoC InputManager"; }

    void ToggleRumble(bool on);
    void SetRumbleForDevice(int controllerId, bool bRumbleOn);
    bool IsRumbleOnForDevice(int controllerId) const;
    void TriggerRumblePulse(int controllerId);

    void SetRumbleEnabled(bool isEnabled);
    bool IsRumbleEnabled() const;

    float GetValue(unsigned int controllerIndex, unsigned int inputIndex) const;
    UserController* GetController(unsigned int controllerIndex);

    int RegisterMappable(unsigned int index, Mappable* pMappable);
    void UnregisterMappable(unsigned int index, int handle);
    void UnregisterMappable(unsigned int index, Mappable* pMappable);
    void UnregisterMappable(Mappable* pMappable);

    void SetGameState(Input::ActiveState state);
    Input::ActiveState GetGameState() const;

    void RegisterControllerID(int playerID, int controllerID);
    void UnregisterControllerID(int playerID);
    void UnregisterAllControllerID();
    int GetControllerIDforPlayer(int playerID) const;
    int GetControllerPlayerIDforController(int controllerIndex) const;

    void EnableReset(bool reset) { mResetEnabled = reset; }
    bool IsProScanButtonsPressed() const { return false; }

private:
    InputManager();
    ~InputManager();

    static InputManager* spInstance;

    Input::ActiveState mGameState;
    bool mRumbleEnabled;
    bool mResetEnabled;
    int mRegisteredControllerID[4];
};

inline InputManager* GetInputManager() { return InputManager::GetInstance(); }

#endif // InputManager_HPP
