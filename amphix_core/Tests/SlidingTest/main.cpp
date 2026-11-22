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
//  main.cpp
//  SlidingTest
//
//  Created by Patryk Cieslak on 02/03/2014.
//  Copyright (c) 2018-2026 Patryk Cieslak. All rights reserved.
//

#include "SlidingTestApp.h"
#include "SlidingTestManager.h"

int main(int argc, const char * argv[])
{
    amphix::RenderSettings s;
    s.windowW = 1200;
    s.windowH = 900;
    s.aa = amphix::RenderQuality::HIGH;
    s.shadows = amphix::RenderQuality::HIGH;
    s.ao = amphix::RenderQuality::HIGH;
    s.atmosphere = amphix::RenderQuality::HIGH;
    s.ocean = amphix::RenderQuality::DISABLED;
    s.verticalSync = true;
    
    amphix::HelperSettings h;
    h.showFluidDynamics = false;
    h.showCoordSys = false;
    h.showBulletDebugInfo = false;
    h.showSensors = false;
    h.showActuators = false;
    h.showForces = false;
    h.showJoints = false;
    
    SlidingTestApp app(std::string(DATA_DIR_PATH), s, h, std::make_unique<SlidingTestManager>(500.0));
    app.Run();
    
    return 0;
}

