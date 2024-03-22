#include "minecraft/src-client/common/client/renderer/game/LevelRendererPlayer.h"
#include "amethyst/runtime/events/EventManager.h"
#include "amethyst/runtime/input/InputManager.h"
#include "amethyst/runtime/AmethystContext.h"
#include "amethyst/runtime/HookManager.h"
#include "amethyst/Log.h"

#include "ConfigManager.h"

#define ModFunction extern "C" __declspec(dllexport)

ConfigManager configManager;
SafetyHookInline getFov;
HookManager hookManager;
float initialFov;
bool releasing;
bool pressed;
float fov;


static float LevelRendererPlayer_getFov(LevelRendererPlayer* self, float a, bool applyEffects) {
    if(pressed || releasing) return fov;
    else {
        initialFov = getFov.thiscall<float>(self, a, applyEffects);
        return fov = initialFov;
    }
}

inline void gradualPress() {
    static float targetFov = configManager.getTargetFov();
    static float zoomRate = configManager.getZoomRate();

    if(fov <= targetFov) fov = targetFov;
    else fov -= zoomRate;
}

inline void gradualRelease() {
    static float zoomRate = configManager.getZoomRate();

    if(fov >= initialFov) {
        releasing = false;
        fov = initialFov;
    } else fov += zoomRate;
}

void OnUpdate() {
    static std::string type = configManager.getZoomType();
    static float targetFov = configManager.getTargetFov();
    
    if(type == "gradual") {
        if(pressed) gradualPress();
        else if(releasing) gradualRelease();
    } else if(type == "instant") {
        if(pressed) fov = targetFov;
        else if(releasing) fov = initialFov;
    }
}

SafetyHookInline _getSensitivity;

float getSensitivity(void* a1, unsigned int a2) {
    float value = _getSensitivity.call<float>(a1, a2);
    Log::Info("sens: {}", value);
    return value;
}


ModFunction void Initialize(AmethystContext* context) {
    context->mHookManager.RegisterFunction(&LevelRendererPlayer::getFov, "48 8B C4 48 89 58 ? 48 89 70 ? 57 48 81 EC ? ? ? ? 0F 29 70 ? 0F 29 78 ? 44 0F 29 40 ? 44 0F 29 48 ? 48 8B 05");
    context->mHookManager.CreateHook(&LevelRendererPlayer::getFov, getFov, &LevelRendererPlayer_getFov);

    
    context->mHookManager.CreateHookAbsolute(_getSensitivity, SigScan("74 ? E8 ? ? ? ? 48 8B 5C 24 ? 48 83 C4 ? 5F C3 49 8B C8 E8 ? ? ? ? 48 8B 5C 24 ? 48 83 C4 ? 5F C3 E8 ? ? ? ? CC E8 ? ? ? ? CC CC CC CC CC CC CC CC CC CC 40 53"), getSensitivity);

    context->mInputManager.RegisterNewInput("zoom", 0x56, true);
    
    //context->mInputManager.AddButtonDownHandler("zoom", [](FocusImpact f, ClientInstance c) {
    //    releasing = false;
    //    pressed = true;
    //});

    //context->mInputManager.AddButtonUpHandler("zoom", [](FocusImpact f, ClientInstance c) {
    //    static std::string type = configManager.getZoomType();

    //    if(type == "gradual") releasing = true;
    //    pressed = false;
    //});
    
    
    context->mEventManager.update.AddListener(OnUpdate);

    Log::Info("[VidereLonge] Mod successfully initialized!");
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) { return TRUE; }