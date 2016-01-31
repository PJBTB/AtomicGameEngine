
#ifdef ATOMIC_PLATFORM_WINDOWS

#include <include/cef_client.h>

#include <ThirdParty/SDL/include/SDL.h>
#include <ThirdParty/SDL/include/SDL_syswm.h>

#include <ThirdParty/SDL/src/events/scancodes_windows.h>

#include <Atomic/Core/Variant.h>
#include <Atomic/Input/InputEvents.h>
#include <Atomic/Input/Input.h>
#include <Atomic/IO/Log.h>

#include "WebKeyboard.h"

namespace Atomic
{

static bool SDLScanCodeToWindowsScanCode(SDL_Scancode code, LPARAM& lParam, WPARAM& wParam )
{

    wParam = 0;
    lParam = 0;

    int numCodes = sizeof(windows_scancode_table)/sizeof(SDL_Scancode);

    int windowsScanCode = -1;
    for (int i  = 0; i < numCodes; i++)
    {
        if (windows_scancode_table[i] == code)
        {
            windowsScanCode = i;
            break;
        }
    }

    if (windowsScanCode != -1)
    {
        wParam = MapVirtualKey(windowsScanCode, MAPVK_VSC_TO_VK);
        lParam = windowsScanCode << 16;
    }

    switch (code)
    {
    case SDL_SCANCODE_RIGHT:
        wParam = VK_RIGHT;
        break;
    case SDL_SCANCODE_LEFT:
        wParam = VK_LEFT;
        break;
    case SDL_SCANCODE_UP:
        wParam = VK_UP;
        break;
    case SDL_SCANCODE_DOWN:
        wParam = VK_DOWN;
        break;
    case SDL_SCANCODE_DELETE:
        wParam = VK_DELETE;
        break;
    case SDL_SCANCODE_BACKSPACE:
        wParam = VK_BACK;
        break;

    }

    return wParam != 0 || lParam != 0;

}

bool ConvertKeyEvent(Input* input, const StringHash eventType, VariantMap& eventData, CefKeyEvent& keyEvent)
{
    if (eventType != "KeyDown" && eventType != "KeyUp")
    {
        LOGERROR("ConvertKeyEvent - Unknown event type");
        return false;
    }

    WebKeyEvent wk(eventType, eventData);

    if (wk.scanCode == SDL_SCANCODE_RETURN)
    {
        if (!wk.keyDown)
            return false;

        keyEvent.windows_key_code = VK_RETURN;
        keyEvent.native_key_code = (int) 0;
        keyEvent.type = KEYEVENT_RAWKEYDOWN;
        return true;
    }

    LPARAM lParam;
    WPARAM wParam;

    keyEvent.modifiers = EVENTFLAG_NONE;

    if (wk.qual & QUAL_SHIFT)
        keyEvent.modifiers |= EVENTFLAG_SHIFT_DOWN;
    if (wk.qual & QUAL_ALT)
        keyEvent.modifiers |= EVENTFLAG_ALT_DOWN;
    if (wk.qual & QUAL_CTRL)
        keyEvent.modifiers |= EVENTFLAG_CONTROL_DOWN;


    if (SDLScanCodeToWindowsScanCode((SDL_Scancode) wk.scanCode, lParam, wParam))
    {
        keyEvent.windows_key_code = (int) wParam;
        keyEvent.native_key_code = (int) lParam;
        keyEvent.type = wk.keyDown ? KEYEVENT_RAWKEYDOWN : KEYEVENT_KEYUP;
    }

    return true;
}

bool ConvertTextInputEvent(StringHash eventType, VariantMap& eventData, CefKeyEvent& keyEvent)
{
    if (eventType != "TextInput")
    {
        LOGERROR("ConvertTextInputEvent - Unknown event type");
        return false;
    }

    String text = eventData[TextInput::P_TEXT].GetString();

    SDL_Keycode keyCode = SDL_GetKeyFromName(text.CString());

    if (SDL_strlen(text.CString()) == 1)
    {
        if (text[0] >= 'A' && text[0] <= 'Z')
        {
            keyCode -= 32;
        }
    }

    keyEvent.windows_key_code = (int) keyCode;
    keyEvent.native_key_code = 0;// (int) lParam;
    keyEvent.type = KEYEVENT_CHAR;

    return true;
}

}

#endif
