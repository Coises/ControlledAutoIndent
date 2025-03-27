// This file is part of ControlledAutoIndent.
// Copyright 2025 by Randall Joseph Fellmy <software@coises.com>, <http://www.coises.com/software/>

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "CommonData.h"
using namespace NPP;


// Routines that load and save the configuration file

void loadConfiguration();
void saveConfiguration();

// Routines that process Scintilla notifications

void scnCharAdded(const Scintilla::NotificationData*);

// Routines that process Notepad++ notifications

void bufferActivated();
void languageChanged(const NMHDR*);

// Routines that process menu commands

void showAboutDialog();
void showSettingsDialog();
void toggleEnable();


// Define menu commands:
//     text to appear on menu (ignored for a menu separator line)
//     address of a void, zero-argument function that processes the command (0 for a menu separator line)
//         recommended: use plugin.cmd, per examples, to wrap the function, setting Scintilla pointers and bypassing notifications
//     ignored on call; on return, Notepad++ will fill this in with the menu command ID it assigns
//     whether to show a checkmark beside this item on initial display of the menu
//     0 or default shortcut key (menu accelerator) specified as the address of an NPP::ShortcutKey structure
//
// If you will reference any menu items elsewhere, define mnemonic references for them following the menu definition,
// where it's easy to see and update them if you change the menu; then you can use:
//    extern NPP::FuncItem menuDefinition[];
//    extern int mnemonic;
// in other source files; use the mnemonic to get the ordinal position in the original menu, and:
//    menuDefinition[mnemonic]._cmdID
// to get the menu item identifier assigned by Notepad++.


FuncItem menuDefinition[] = {
    { L"Enabled"       , []() {plugin.cmd(toggleEnable      );}, 0, false, 0},
    { L"Settings..."   , []() {plugin.cmd(showSettingsDialog);}, 0, false, 0},
    { L"Help/About..." , []() {plugin.cmd(showAboutDialog   );}, 0, false, 0}
};

int menuItem_ToggleEnable = 0;


// Tell Notepad++ the plugin name

extern "C" __declspec(dllexport) const wchar_t* getName() {
    return L"Controlled Auto-indent";
}


// Tell Notepad++ about the plugin menu

extern "C" __declspec(dllexport) FuncItem * getFuncsArray(int *n) {
    loadConfiguration();
    menuDefinition[menuItem_ToggleEnable]._init2Check = data.enabled;
    *n = sizeof(menuDefinition) / sizeof(FuncItem);
    return reinterpret_cast<FuncItem*>(&menuDefinition);
}


// Notification processing: each notification desired must be sent to a function that will handle it.

extern "C" __declspec(dllexport) void beNotified(SCNotification *np) {

    if (plugin.bypassNotifications) return;
    plugin.bypassNotifications = true;
    auto*& nmhdr = reinterpret_cast<NMHDR*&>(np);

    // Example Notepad++ notifications; you can add others as needed.
    // Note that most of the notifications listed below have some connection to plugin framework code;
    // it's best to leave those and just remove any function calls you don't use.

    if (nmhdr->hwndFrom == plugin.nppData._nppHandle) {
      
        switch (nmhdr->code) {

        case NPPN_BEFORESHUTDOWN:
            plugin.startupOrShutdown = true;
            break;

        case NPPN_BUFFERACTIVATED:
            if (!plugin.startupOrShutdown && !plugin.fileIsOpening) {
                plugin.getScintillaPointers();
                bufferActivated();
            }
            break;

        case NPPN_CANCELSHUTDOWN:
            plugin.startupOrShutdown = false;
            break;

        case NPPN_FILEBEFOREOPEN:
            plugin.fileIsOpening = true;
            break;

        case NPPN_FILEOPENED:
            plugin.fileIsOpening = false;
            break;

        case NPPN_LANGCHANGED:
            plugin.getScintillaPointers();
            languageChanged(nmhdr);
            break;

        case NPPN_READY:
            plugin.startupOrShutdown = false;
            plugin.getScintillaPointers();
            bufferActivated();
            break;

        case NPPN_SHUTDOWN:
            saveConfiguration();
            break;

        }

    }

    else if ( data.enabled
        && (nmhdr->hwndFrom == plugin.nppData._scintillaMainHandle || nmhdr->hwndFrom == plugin.nppData._scintillaSecondHandle) ) {

        auto*& scnp = reinterpret_cast<Scintilla::NotificationData*&>(np);
        switch (scnp->nmhdr.code) {

        case Scintilla::Notification::CharAdded:
            plugin.getScintillaPointers(scnp);
            scnCharAdded(scnp);
            break;

        default:;
        }

    }

    plugin.bypassNotifications = false;

}


// This is rarely used, but a few Notepad++ commands call this routine as part of their processing

extern "C" __declspec(dllexport) LRESULT messageProc(UINT, WPARAM, LPARAM) {return TRUE;}
