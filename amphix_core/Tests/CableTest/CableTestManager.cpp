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
//  CableTestManager.cpp
//  Amphix
//
//  Created by Patryk Cieslak on 12/11/2025.
//  Copyright (c) 2025-2026 Patryk Cieslak. All rights reserved.
//

#include "CableTestManager.h"

#include <iostream>
#include <iomanip>
#include <utils/SystemUtil.hpp>
#include <utils/UnitSystem.h>
#include <entities/statics/Plane.h>
#include <entities/solids/Box.h>
#include <entities/solids/Sphere.h>
#include <entities/CableEntity.h>
#include <actuators/Light.h>
#include <graphics/OpenGLContent.h>
#include <entities/forcefields/Uniform.h>
#include <entities/solids/Cylinder.h>
#include <entities/statics/Obstacle.h>
#include <actuators/Servo.h>
#include <actuators/Motor.h>
#include <joints/RevoluteJoint.h>
#include <joints/FixedJoint.h>
#include <core/FeatherstoneRobot.h>

CableTestManager::CableTestManager(amphix::Scalar stepsPerSecond) 
    : SimulationManager(stepsPerSecond, amphix::Solver::SI, amphix::CollisionFilter::EXCLUSIVE)
{
}

void CableTestManager::BuildScenario()
{
    SetIcSolverParams(false);

    ///////MATERIALS////////
    CreateMaterial("Ground", 1000.0, 1.0);
    CreateMaterial("Steel", amphix::UnitSystem::Density(amphix::CGS, amphix::MKS, 9.81), 0.1);
    CreateMaterial("Wood", amphix::UnitSystem::Density(amphix::CGS, amphix::MKS, 0.8), 0.1);
    SetMaterialsInteraction("Ground", "Ground", 1.0, 1.0);
    SetMaterialsInteraction("Ground", "Steel", 1.0, 1.0);
    SetMaterialsInteraction("Ground", "Wood", 1.0, 1.0);
    SetMaterialsInteraction("Steel", "Steel", 1.0, 1.0);
    SetMaterialsInteraction("Steel", "Wood", 1.0, 1.0);
    SetMaterialsInteraction("Wood", "Wood", 1.0, 1.0);
    
    ///////LOOKS///////////
    CreateLook("Grid", amphix::Color::Gray(1.f), 0.8f, 0.f, 0.0f, amphix::GetShaderPath() + "grid.png");
    CreateLook("Green", amphix::Color::RGB(0.1f, 0.8f, 0.1f), 0.5f, 0.0f);
    CreateLook("Red", amphix::Color::RGB(1.0f, 0.3f, 0.3f), 0.5f, 0.0f);
    CreateLook("Rope", amphix::Color::Gray(1.f), 1.f, 0.f, 0.f, amphix::GetDataPath() + "rope_color.jpg", amphix::GetDataPath() + "rope_normal.png");

    EnableOcean();
    GetOcean()->SetWaterType(0.2);
    GetOcean()->AddVelocityField(std::make_unique<amphix::Uniform>(amphix::Vector3(0.0,0.0,0.0)));
    GetOcean()->EnableVelocityFields();
    
    ////////OBJECTS
    AddStaticEntity(std::make_unique<amphix::Plane>("Floor", 10000, "Ground", "Grid"), amphix::Transform(amphix::IQ(), amphix::Vector3(0.0, 0.0, 10.0)));

    amphix::PhysicsSettings phy;
    phy.mode = amphix::PhysicsMode::SUBMERGED;
    phy.collisions = true;

    // Box attached to a fixed cable
    amphix::SolidEntity* box = AddSolidEntity(std::make_unique<amphix::Box>("Box", phy, amphix::Vector3(0.1, 0.1, 0.1), amphix::I4(), "Steel", "Red"), 
        amphix::Transform(amphix::IQ(), amphix::Vector3(0.0, 0.0, 2.0)));

    std::unique_ptr<amphix::CableEntity> cable = std::make_unique<amphix::CableEntity>("Cable1", phy, amphix::Vector3(2.0, 0, -2), amphix::Vector3(0, 0, 1.5), 100, 0.01, "Steel", "Red", 0.0);
    cable->AttachToWorld(amphix::CableEnds::FIRST);
    cable->AttachToSolid(amphix::CableEnds::SECOND, box);
    AddEntity(std::move(cable));

    // Winch with cable
    std::unique_ptr<amphix::Box> box2 = std::make_unique<amphix::Box>("WinchBase", phy, amphix::Vector3(0.1, 0.1, 0.1), amphix::I4(), "Steel", "Green");
    std::vector<std::unique_ptr<amphix::SolidEntity>> links;
    links.push_back(std::make_unique<amphix::Cylinder>("WinchCylinder", phy, 0.2, 2.0, amphix::Transform(amphix::Quaternion(0.0, 0.0, M_PI_2), amphix::Vector3(0.0, 0.0, 0.0)), "Steel", "Green"));
    
    std::unique_ptr<amphix::FeatherstoneRobot> winch = std::make_unique<amphix::FeatherstoneRobot>("Winch", true);
    winch->DefineLinks(std::move(box2), std::move(links), false);
    winch->DefineRevoluteJoint("Joint1", "WinchBase", "WinchCylinder", amphix::Transform(amphix::IQ(), amphix::Vector3(0.0, 1.0, 0.0)), amphix::Vector3(0.0, 1.0, 0.0));
    winch->BuildKinematicStructure();
    
    std::unique_ptr<amphix::Servo> servo = std::make_unique<amphix::Servo>("Servo", 1.0, 1.0, 100.0);
    servo->SetControlMode(amphix::ServoControlMode::VELOCITY);
    winch->AddJointActuator(std::move(servo), "Joint1");
    
    amphix::FeatherstoneRobot* winchPtr = static_cast<amphix::FeatherstoneRobot*>(
        AddRobot(std::move(winch), amphix::Transform(amphix::IQ(), amphix::Vector3(5.0, 0.0, -5.0))));
    
    amphix::SolidEntity* sphere = AddSolidEntity(std::make_unique<amphix::Sphere>("CableAttachSphere", phy, 0.01, amphix::I4(), "Steel", "Green"), 
        amphix::Transform(amphix::IQ(), amphix::Vector3(5.0, 1.0, -4.8)));

    AddJoint(std::make_unique<amphix::FixedJoint>("CableAttachJoint", sphere, winchPtr->GetDynamics(), 0));

    amphix::CableEntity* cable2 = static_cast<amphix::CableEntity*>(
        AddEntity(std::make_unique<amphix::CableEntity>("Cable2", phy, amphix::Vector3(5.0, 1.0, -4.78), amphix::Vector3(5.0, 1.0, -1.0), 100, 0.02f, "Wood", "Rope", 0.f, 80.f))
    );
    cable2->AttachToSolid(amphix::CableEnds::FIRST, sphere);

    AddEntity(std::make_unique<amphix::CableEntity>("Cable3", phy, amphix::Vector3(4.0, -1.5, -2.0), amphix::Vector3(4.0, 1.5, -2.0), 50, 0.05, "Wood", "Rope", 0.0, 80.f));

    AddStaticEntity(std::make_unique<amphix::Obstacle>("Obstacle", 0.25, 2.0, amphix::I4(), "Steel", "Red"), 
        amphix::Transform(amphix::IQ(), amphix::Vector3(7.0, 0.0, 0.0)));
}

void CableTestManager::SimulationStepCompleted(amphix::Scalar timeStep)
{
    amphix::CableEntity* cable = dynamic_cast<amphix::CableEntity*>(GetEntity("Cable1"));
    if (cable != nullptr)
    {
        amphix::Scalar length = cable->GetLength();
        amphix::Scalar restLength = cable->GetRestLength();
        amphix::Scalar stretch = (length - restLength) / restLength;
        std::cout << std::setprecision(3) << "[" << cable->GetName() << "] Length: " << length << " m, Stretch: " << stretch * 100 << " %" << std::endl;
    }
}
