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
//  JointsTestApp.cpp
//  Amphix
//
//  Created by Patryk Cieslak on 01/02/2023.
//  Copyright (c) 2023-2026 Patryk Cieslak. All rights reserved.
//

#include "JointsTestApp.h"

#include <actuators/Servo.h>
#include <actuators/SuctionCup.h>
#include <core/Robot.h>
#include <core/Console.h>
#include <graphics/IMGUI.h>

JointsTestApp::JointsTestApp(std::string dataDirPath, amphix::RenderSettings s, amphix::HelperSettings h, std::unique_ptr<JointsTestManager> sim)
    : GraphicalSimulationApp("Joints Test", dataDirPath, s, h, std::move(sim))
{
}

void JointsTestApp::DoHUD()
{
    GraphicalSimulationApp::DoHUD();

    amphix::Uid id;
    id.owner = 10;
    id.item = 0;

    amphix::Servo* srv1 = (amphix::Servo*)GetSimulationManager()->GetActuator("Servo1");
    amphix::Scalar sp = GetGui()->DoSlider(id, 180.f, 10.f, 150.f, amphix::Scalar(-M_PI_2), amphix::Scalar(M_PI_2), srv1->GetDesiredPosition(), "Servo1");
    srv1->SetDesiredPosition(sp);

    id.item = 1;
    amphix::Servo* srv2 = (amphix::Servo*)GetSimulationManager()->GetActuator("Servo2");
    sp = GetGui()->DoSlider(id, 180.f, 60.f, 150.f, amphix::Scalar(-M_PI_2), amphix::Scalar(M_PI_2), srv2->GetDesiredPosition(), "Servo2");
    srv2->SetDesiredPosition(sp);

    id.item = 2;
    amphix::Servo* srv3 = (amphix::Servo*)GetSimulationManager()->GetActuator("Servo3");
    sp = GetGui()->DoSlider(id, 180.f, 110.f, 150.f, amphix::Scalar(-M_PI_2), amphix::Scalar(M_PI_2), srv3->GetDesiredPosition(), "Servo3");
    srv3->SetDesiredPosition(sp);

    // id.item = 3;
    // amphix::SuctionCup* suction = (amphix::SuctionCup*)GetSimulationManager()->GetActuator("Suction");
    // suction->SetPump(GetGui()->DoCheckBox(id, 180.f, 160.f, 150.f, suction->GetPump(), "Vacuum Pump"));
}
