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
//  FlyingTestManager.cpp
//  Amphix
//
//  Created by Patryk Cieslak on 06/05/2019.
//  Copyright (c) 2019-2026 Patryk Cieslak. All rights reserved.
//

#include "FlyingTestManager.h"

#include <entities/solids/Polyhedron.h>
#include <entities/solids/Compound.h>
#include <entities/solids/Box.h>
#include <utils/UnitSystem.h>
#include <utils/SystemUtil.hpp>
#include <core/FeatherstoneRobot.h>
#include <sensors/scalar/Odometry.h>
#include <entities/statics/Plane.h>
#include <actuators/Propeller.h>

FlyingTestManager::FlyingTestManager(amphix::Scalar stepsPerSecond)
   : SimulationManager(stepsPerSecond, amphix::Solver::SI, amphix::CollisionFilter::EXCLUSIVE)
{
}

void FlyingTestManager::BuildScenario()
{
    ///////MATERIALS////////
    CreateMaterial("Fiberglass", amphix::UnitSystem::Density(amphix::CGS, amphix::MKS, 1.5), 0.3);
    CreateMaterial("Ground", 1000.0, 1.0);
    SetMaterialsInteraction("Fiberglass", "Fiberglass", 0.5, 0.2);
    SetMaterialsInteraction("Fiberglass", "Ground", 0.7, 0.4);
    SetMaterialsInteraction("Ground", "Ground", 0.9, 0.7);
    
    ///////LOOKS///////////
    CreateLook("grid", amphix::Color::Gray(1.f), 0.8f, 0.f, 0.f, amphix::GetShaderPath() + "grid.png");
    CreateLook("white", amphix::Color::Gray(1.f), 0.9f, 0.0f, 0.f);
    CreateLook("propeller", amphix::Color::RGB(1.f, 0.f, 0.f), 0.3f, 0.f, 0.f);

    ////////OBJECTS    
    //Create environment
    GetAtmosphere()->SetSunPosition(0.0, 60.0);
    
    AddStaticEntity(std::make_unique<amphix::Plane>("Floor", 10000, "Ground", "grid"), amphix::Transform::getIdentity());

    amphix::PhysicsSettings phy;
    phy.mode = amphix::PhysicsMode::AERODYNAMIC;
    phy.collisions = true;
    AddSolidEntity(std::make_unique<amphix::Box>("Leaf", phy, amphix::Vector3(5,3,0.0001), amphix::I4(), "Fiberglass","white"), amphix::Transform(amphix::Quaternion(0,0.3,0), amphix::Vector3(10,0,-10)));
    
    std::unique_ptr<amphix::Compound> fuselage = std::make_unique<amphix::Compound>("Fuselage", phy, std::make_unique<amphix::Box>("Arm1", phy, amphix::Vector3(1.0,0.01,0.01), amphix::I4(), "Fiberglass", "white"), amphix::I4());
    fuselage->AddExternalPart(std::make_unique<amphix::Box>("Arm2", phy, amphix::Vector3(0.01,1.0,0.01), amphix::I4(), "Fiberglass", "white"), amphix::I4());

    std::unique_ptr<amphix::FeatherstoneRobot> quadCopter = std::make_unique<amphix::FeatherstoneRobot>("Quadcopter");
    quadCopter->DefineLinks(std::move(fuselage));
    quadCopter->BuildKinematicStructure();
    quadCopter->AddLinkActuator(std::make_unique<amphix::Propeller>("Propeller1", std::make_unique<amphix::Polyhedron>("Prop1", phy, amphix::GetDataPath() + "propeller_air.obj", amphix::Scalar(1), amphix::I4(), "Fiberglass", "propeller"), 
        0.2, std::make_pair(0.1, 0.1), 0.01, 10000, true), "Fuselage", amphix::Transform(amphix::Quaternion(0,M_PI_2,0), amphix::Vector3(0.5,0.0,-0.02)));
    quadCopter->AddLinkActuator(std::make_unique<amphix::Propeller>("Propeller2", std::make_unique<amphix::Polyhedron>("Prop2", phy, amphix::GetDataPath() + "propeller_air.obj", amphix::Scalar(1), amphix::I4(), "Fiberglass", "propeller"), 
        0.2, std::make_pair(0.1, 0.1), 0.01, 10000, true), "Fuselage", amphix::Transform(amphix::Quaternion(0,M_PI_2,0), amphix::Vector3(-0.5,0.0,-0.02)));
    quadCopter->AddLinkActuator(std::make_unique<amphix::Propeller>("Propeller3", std::make_unique<amphix::Polyhedron>("Prop3", phy, amphix::GetDataPath() + "propeller_air.obj", amphix::Scalar(1), amphix::I4(), "Fiberglass", "propeller"), 
        0.2, std::make_pair(0.1, 0.1), 0.01, 10000, false), "Fuselage", amphix::Transform(amphix::Quaternion(0,M_PI_2,0), amphix::Vector3(0.0,0.5,-0.02)));
    quadCopter->AddLinkActuator(std::make_unique<amphix::Propeller>("Propeller4", std::make_unique<amphix::Polyhedron>("Prop4", phy, amphix::GetDataPath() + "propeller_air.obj", amphix::Scalar(1), amphix::I4(), "Fiberglass", "propeller"), 
        0.2, std::make_pair(0.1, 0.1), 0.01, 10000, false), "Fuselage", amphix::Transform(amphix::Quaternion(0,M_PI_2,0), amphix::Vector3(0.0,-0.5,-0.02)));
    
    amphix::Robot* robot = AddRobot(std::move(quadCopter), amphix::Transform(amphix::IQ(), amphix::Vector3(0,0,-0.02)));
    static_cast<amphix::Propeller*>(robot->GetActuator("Propeller1"))->SetSetpoint(0.5);
    static_cast<amphix::Propeller*>(robot->GetActuator("Propeller2"))->SetSetpoint(0.5);
    static_cast<amphix::Propeller*>(robot->GetActuator("Propeller3"))->SetSetpoint(-0.5);
    static_cast<amphix::Propeller*>(robot->GetActuator("Propeller4"))->SetSetpoint(-0.5);
}