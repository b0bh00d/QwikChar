#include "mainwindow.h"

LRESULT CALLBACK LowLevelKeyboardProc( int nCode, WPARAM wParam, LPARAM lParam )
{
    if(nCode >= 0)
    {
        if(!main_window->is_showing())
        {
            KBDLLHOOKSTRUCT *pKeyboard = reinterpret_cast<KBDLLHOOKSTRUCT *>(lParam);

            switch(wParam)
            {
                case WM_SYSKEYDOWN:
                    main_window->set_modifier(pKeyboard->vkCode);
                    break;

                case WM_KEYDOWN:
                    if(main_window->set_modifier(pKeyboard->vkCode))
                    {
                        if(main_window->should_activate(pKeyboard->vkCode))
                        {
                            main_window->process_show();
                            return 1;
                        }
                    }
                    break;

                case WM_SYSKEYUP:
                case WM_KEYUP: // When the key has been pressed and released
                    main_window->reset_modifier(pKeyboard->vkCode);
                    break;
            }
        }
    }

    return CallNextHookEx( nullptr, nCode, wParam, lParam );
}

void MainWindow::hook_global_keyboard_event()
{
    if(keyboard_hook_handle)
        return;

    HINSTANCE appInstance = GetModuleHandle(nullptr); // Set a global Windows Hook to capture keystrokes.
    keyboard_hook_handle = SetWindowsHookExW( WH_KEYBOARD_LL, LowLevelKeyboardProc, appInstance, 0 );
}

void MainWindow::unhook_global_keyboard_event()
{
    if(!keyboard_hook_handle)
        return;

    UnhookWindowsHookEx(keyboard_hook_handle);
    keyboard_hook_handle = nullptr;
}
