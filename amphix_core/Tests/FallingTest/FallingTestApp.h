/*    
    This file is a part of Amphix.

    Amphix is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Amphix is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

//
//  FallingTestApp.h
//  Amphix
//
//  Created by Patryk Cieslak on 03/03/2014.
//  Copyright (c) 2014-2026 Patryk Cieslak. All rights reserved.
//

#pragma once

#include <core/GraphicalSimulationApp.h>
#include "FallingTestManager.h"

class FallingTestApp : public amphix::GraphicalSimulationApp
{
public:
    FallingTestApp(std::string_view dataDirPath, amphix::RenderSettings s, amphix::HelperSettings h, std::unique_ptr<FallingTestManager> sim);
    
    void DoHUD();
};

