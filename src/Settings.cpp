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
#include "resource.h"
#include "Shlwapi.h"

namespace {

INT_PTR CALLBACK settingsDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM) {

    switch (uMsg) {

    case WM_DESTROY:
        return TRUE;

    case WM_INITDIALOG:
        config_rect::show(hwndDlg);  // centers dialog on owner client area, without saving position
        data.matchIndentation    .put(hwndDlg, IDC_SETTINGS_MATCH        );
        data.trimEmptyLines      .put(hwndDlg, IDC_SETTINGS_TRIMEMPTY    );
        data.removeTrailingSpace .put(hwndDlg, IDC_SETTINGS_TRAILING     );
        data.retainFollowingSpace.put(hwndDlg, IDC_SETTINGS_RETAIN       );
        data.pythonColon         .put(hwndDlg, IDC_SETTINGS_PYTHON_COLON );
        data.cBraces             .put(hwndDlg, IDC_SETTINGS_C_BRACES     );
        data.cAddEmpty           .put(hwndDlg, IDC_SETTINGS_C_ADDEMPTY   );
        data.cContinuation       .put(hwndDlg, IDC_SETTINGS_C_INCOMPLETE );
        npp(NPPM_DARKMODESUBCLASSANDTHEME, NPP::NppDarkMode::dmfInit, hwndDlg);  // Include to support dark mode
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDCANCEL:
            EndDialog(hwndDlg, 1);
            return TRUE;
        case IDOK:
            data.matchIndentation    .get(hwndDlg, IDC_SETTINGS_MATCH        );
            data.trimEmptyLines      .get(hwndDlg, IDC_SETTINGS_TRIMEMPTY    );
            data.removeTrailingSpace .get(hwndDlg, IDC_SETTINGS_TRAILING     );
            data.retainFollowingSpace.get(hwndDlg, IDC_SETTINGS_RETAIN       );
            data.pythonColon         .get(hwndDlg, IDC_SETTINGS_PYTHON_COLON );
            data.cBraces             .get(hwndDlg, IDC_SETTINGS_C_BRACES     );
            data.cAddEmpty           .get(hwndDlg, IDC_SETTINGS_C_ADDEMPTY   );
            data.cContinuation       .get(hwndDlg, IDC_SETTINGS_C_INCOMPLETE );
            EndDialog(hwndDlg, 0);
            return TRUE;
        }
        return FALSE;

    }
    return FALSE;
}

}

void showSettingsDialog() {
    DialogBox(plugin.dllInstance, MAKEINTRESOURCE(IDD_SETTINGS), plugin.nppData._nppHandle, settingsDialogProc);
}
