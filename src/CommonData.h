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

#pragma once

#include "Framework/PluginFramework.h"
#include "Framework/ConfigFramework.h"
#include "Framework/UtilityFramework.h"

struct SyntaxInfo;

inline struct CommonData {

    config<bool> enabled              = { "Enabled"             , false };
    config<bool> matchIndentation     = { "MatchIndentation"    , true  };
    config<bool> trimEmptyLines       = { "TrimEmptyLines"      , false };
    config<bool> removeTrailingSpace  = { "RemoveTrailingSpace" , false };
    config<bool> retainFollowingSpace = { "RetainFollowingSpace", false };
    config<bool> pythonColon          = { "PythonColonIndent"   , true  };
    config<bool> cBraces              = { "CBracesIndent"       , true  };
    config<bool> cAddEmpty            = { "CBracesAddEmpty"     , true  };
    config<bool> cContinuation        = { "CContinuationIndent" , false };

    const SyntaxInfo* syntaxInfo = 0;  // points to syntax information for the current buffer, or 0 if lexer says not to auto-indent

} data;
