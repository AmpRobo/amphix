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
//  SlidingTestManager.cpp
//  Amphix
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright (c) 2014-2026 Patryk Cieslak. All rights reserved.
//

#include "SlidingTestManager.h"

#include "SlidingTestApp.h"
#include <entities/statics/Plane.h>
#include <entities/statics/Obstacle.h>
#include <entities/solids/Box.h>
#include <actuators/Light.h>
#include <sensors/scalar/Odometry.h>
#include <utils/SystemUtil.hpp>

SlidingTestManager::SlidingTestManager(amphix::Scalar stepsPerSecond) 
  : SimulationManager(stepsPerSecond, amphix::Solver::SI, amphix::CollisionFilter::EXCLUSIVE)
{
}

void SlidingTestManager::BuildScenario()
{
    ///////MATERIALS////////
    CreateMaterial("Ground", 1000.0, 1.0);
    CreateMaterial("Steel", 1000.0, 0.0);
    SetMaterialsInteraction("Ground", "Ground", 0.0, 0.0);
    SetMaterialsInteraction("Ground", "Steel", 0.25, 0.2);
    SetMaterialsInteraction("Steel", "Steel", 0.25, 0.2);
    
    ///////LOOKS///////////
    CreateLook("grid", amphix::Color::RGB(1.f, 1.f, 1.f), 0.f, 0.1f, 0.f, amphix::GetShaderPath() + "grid.png");
    CreateLook("green", amphix::Color::RGB(0.3f, 1.0f, 0.2f), 0.2f, 0.f);
    
    ////////OBJECTS
    amphix::Scalar angle = M_PI/180.0 * 14.1;
    
    AddStaticEntity(std::make_unique<amphix::Plane>("Floor", amphix::Scalar(10000), "Ground", "grid"), amphix::I4());
  
    AddStaticEntity(std::make_unique<amphix::Obstacle>("Ramp", amphix::Vector3(10,2,0.1), amphix::I4(), "Ground", "grid", 2), 
        amphix::Transform(amphix::Quaternion(0, angle, 0), amphix::Vector3(0,0,-1.0)));

    amphix::PhysicsSettings phy;
    phy.mode = amphix::PhysicsMode::SURFACE;
    phy.collisions = true;  
    amphix::SolidEntity* box = AddSolidEntity(std::make_unique<amphix::Box>("Box", phy, amphix::Vector3(0.1,0.1,0.1), amphix::I4(), "Steel", "green"),
        amphix::Transform(amphix::Quaternion(0, angle, 0), amphix::Vector3(2.5, 0, -1.72)));
    
    std::unique_ptr<amphix::Odometry> traj = std::make_unique<amphix::Odometry>("Odometry", -1, 1000);
    traj->AttachToSolid(box, amphix::I4());
    traj->SetRenderable(true);
    AddSensor(std::move(traj));
    
    //////CAMERA & LIGHT//////
    std::unique_ptr<amphix::Light> omni = std::make_unique<amphix::Light>("Omni", amphix::Scalar(0.1), amphix::Color::BlackBody(4000), 1000000);
    omni->AttachToSolid(box, amphix::Transform(amphix::IQ(), amphix::Vector3(0,0,-1)));
    AddActuator(std::move(omni));
}