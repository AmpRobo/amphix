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
//  JointsTestManager.cpp
//  Amphix
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright (c) 2014-2026 Patryk Cieslak. All rights reserved.
//

#include "JointsTestManager.h"

#include <entities/statics/Plane.h>
#include <entities/solids/Box.h>
#include <entities/solids/Sphere.h>
#include <entities/solids/Cylinder.h>
#include <entities/solids/Polyhedron.h>
#include <graphics/OpenGLPointLight.h>
#include <graphics/OpenGLTrackball.h>
#include <joints/FixedJoint.h>
#include <joints/SphericalJoint.h>
#include <joints/RevoluteJoint.h>
#include <joints/PrismaticJoint.h>
#include <joints/CylindricalJoint.h>
#include <actuators/SuctionCup.h>
#include <utils/UnitSystem.h>
#include <utils/SystemUtil.hpp>
#include <core/GeneralRobot.h>
#include <actuators/Servo.h>
#include <sensors/scalar/GPS.h>
#include <core/NED.h>

JointsTestManager::JointsTestManager(amphix::Scalar stepsPerSecond) 
  : SimulationManager(stepsPerSecond, amphix::Solver::SI, amphix::CollisionFilter::EXCLUSIVE)
{
}

void JointsTestManager::BuildScenario()
{
    ///////MATERIALS////////
    CreateMaterial("Steel", amphix::UnitSystem::Density(amphix::UnitSystems::CGS, amphix::UnitSystems::MKS, 7.8), 0.5);
    CreateMaterial("Plastic", amphix::UnitSystem::Density(amphix::UnitSystems::CGS, amphix::UnitSystems::MKS, 1.5), 0.2);
    SetMaterialsInteraction("Steel", "Plastic", 0.8, 0.2);
    SetMaterialsInteraction("Steel", "Steel", 0.5, 0.1);
    SetMaterialsInteraction("Plastic", "Plastic", 0.5, 0.2);
    
    ///////LOOKS///////////
    CreateLook("grid", amphix::Color(1.f, 1.f, 1.f), 0.5f, 0.0f, 0.0f, amphix::GetShaderPath() + "grid.png");
    CreateLook("orange", amphix::Color(1.0f,0.6f,0.3f), 0.3f);
    CreateLook("green", amphix::Color(0.5f,1.0f,0.4f), 0.5f);
    
    ////////OBJECTS
    SetSolverParams(0.25, 0.5, 0.25, 0.25, 0.0, -1.0, -1.0);
    GetAtmosphere()->SetSunPosition(0.0, 70.0);
    static_cast<amphix::GraphicalSimulationApp*>(amphix::SimulationApp::GetApp())->GetTrackball()->MoveCenter(glm::vec3(1.f,3.f,0.f));
    GetNed()->Init(-10.0, -10.0, 0.0);
    
    AddStaticEntity(std::make_unique<amphix::Plane>("Floor", 1000.f, "Steel", "grid"), amphix::I4());
    
    amphix::PhysicsSettings phy;
    phy.mode = amphix::PhysicsMode::SURFACE;
    phy.collisions = true;

    //----Fixed Joint----
    amphix::SolidEntity* box = AddSolidEntity(
        std::make_unique<amphix::Box>("Box", phy, amphix::Vector3(0.1,0.1,0.1), amphix::I4(), "Plastic","green"), 
        amphix::Transform(amphix::IQ(), amphix::Vector3(-2.0,0.0,-3.0))
    );
    
    amphix::SolidEntity* sph = AddSolidEntity(
        std::make_unique<amphix::Sphere>("Sph1", phy, amphix::Scalar(0.2), amphix::I4(), "Steel", "orange"), 
        amphix::Transform(amphix::IQ(), amphix::Vector3(-2.0,-0.5,-3.0))
    );
    AddJoint(std::make_unique<amphix::FixedJoint>("Fix", box, sph));
    
    //----Revolute Joint----
    box = AddSolidEntity(
        std::make_unique<amphix::Box>("Box1", phy, amphix::Vector3(0.1,0.1,0.1), amphix::I4(), "Plastic", "green"), 
        amphix::Transform(amphix::IQ(), amphix::Vector3(-2.5,0.0,-1.0))
    );
    
    amphix::SolidEntity* box2 = AddSolidEntity(
        std::make_unique<amphix::Box>("Box2", phy, amphix::Vector3(0.1,0.1,0.1), amphix::I4(), "Plastic", "orange"), 
        amphix::Transform(amphix::IQ(), amphix::Vector3(-2.5,0.2,-1.0))
    );
    AddJoint(std::make_unique<amphix::RevoluteJoint>("Revolute", box, box2, amphix::Vector3(-2.5,0.1,-0.9), amphix::Vector3(0,1,0), false));
    
    sph = AddSolidEntity(
        std::make_unique<amphix::Sphere>("Sph2", phy, amphix::Scalar(0.2), amphix::I4(), "Plastic", "green"),
        amphix::Transform(amphix::IQ(), amphix::Vector3(0.0,0.0,-3.0))
    );
    AddJoint(std::make_unique<amphix::RevoluteJoint>("RevoluteFix", sph, amphix::Vector3(0.0,1.0,-3.0), amphix::Vector3(1.0,0.0,0.0)));
    
    //----Spherical Joint----
    sph = AddSolidEntity(
        std::make_unique<amphix::Sphere>("Sph3", phy, amphix::Scalar(0.2), amphix::I4(), "Plastic", "green"),
        amphix::Transform(amphix::IQ(), amphix::Vector3(0.0, -2.0, -1.0))
    );
    amphix::SolidEntity* sph2 = AddSolidEntity(
        std::make_unique<amphix::Sphere>("Sph4", phy, amphix::Scalar(0.15), amphix::I4(), "Plastic", "orange"),
        amphix::Transform(amphix::IQ(), amphix::Vector3(0.0, -2.2, -0.4))
    );
    AddJoint(std::make_unique<amphix::SphericalJoint>("Spherical", sph, sph2, amphix::Vector3(0.0, -2.0, -0.6)));
    
    //----Prismatic Joint----
    box = AddSolidEntity(
        std::make_unique<amphix::Box>("Box4", phy, amphix::Vector3(0.1,0.1,0.1), amphix::I4(), "Plastic", "green"),
        amphix::Transform(amphix::IQ(), amphix::Vector3(2.0,0.0,-0.051))
    );
    
    box2 = AddSolidEntity(
        std::make_unique<amphix::Box>("Box5", phy, amphix::Vector3(0.1,0.1,0.1), amphix::I4(), "Plastic", "orange"),
        amphix::Transform(amphix::IQ(), amphix::Vector3(2.0,0.0,-0.5))
    );
    AddJoint(std::make_unique<amphix::PrismaticJoint>("Prismatic", box, box2, amphix::Vector3(0.5,0,-1.0)));
    
    // //----Cylindrical Joint----
    box = AddSolidEntity(
        std::make_unique<amphix::Box>("Box6", phy, amphix::Vector3(0.1,0.1,0.1), amphix::I4(), "Plastic", "green"),
        amphix::Transform(amphix::IQ(), amphix::Vector3(-1.0,0.0,-0.051))
    );
    
    box2 = AddSolidEntity(
        std::make_unique<amphix::Box>("Box7", phy, amphix::Vector3(0.1,0.1,0.1), amphix::I4(), "Plastic", "orange"),
        amphix::Transform(amphix::IQ(), amphix::Vector3(-1.0,0.0,-0.5))
    );
    
    AddJoint(std::make_unique<amphix::CylindricalJoint>("Cylindrical", box, box2, amphix::Vector3(-1.0, 0.050, -0.25), amphix::Vector3(0,0,1)));

    //----General robot
    std::unique_ptr<amphix::GeneralRobot> robot = std::make_unique<amphix::GeneralRobot>("Manipulator", true);
    
    // Base link
    std::unique_ptr<amphix::Box> base = std::make_unique<amphix::Box>("Base", phy, amphix::Vector3(0.1,0.1,0.1), amphix::Transform(amphix::IQ(), amphix::Vector3(0.0, 0, 0.0)), "Plastic","green");
    // Arm
    std::unique_ptr<amphix::Box> arm1 = std::make_unique<amphix::Box>("Arm1", phy, amphix::Vector3(0.1,0.1,0.5), amphix::Transform(amphix::IQ(), amphix::Vector3(0.0, 0, -0.25)), "Plastic","green");
    std::unique_ptr<amphix::Box> arm2 = std::make_unique<amphix::Box>("Arm2", phy, amphix::Vector3(0.8,0.1,0.1), amphix::Transform(amphix::IQ(), amphix::Vector3(0.4, 0, -0.5)), "Plastic","green");
    std::unique_ptr<amphix::Box> arm3 = std::make_unique<amphix::Box>("Arm3", phy, amphix::Vector3(0.1,0.1,0.5), amphix::Transform(amphix::IQ(), amphix::Vector3(0.8, 0, -0.25)), "Plastic","green");

    std::vector<std::unique_ptr<amphix::SolidEntity>> links;
    links.push_back(std::move(arm1));
    links.push_back(std::move(arm2));
    links.push_back(std::move(arm3));
    robot->DefineLinks(std::move(base), std::move(links));
    
    robot->DefineRevoluteJoint("joint1", "Base", "Arm1", amphix::Transform(amphix::IQ(), amphix::Vector3(0.0, 0.0, 0.0)), amphix::Vector3(0, 0, 1.0));
    robot->DefineRevoluteJoint("joint2", "Arm1", "Arm2", amphix::Transform(amphix::IQ(), amphix::Vector3(0, 0.0, -0.5)), amphix::Vector3(0, 1.0, 0.0));
    robot->DefineRevoluteJoint("joint3", "Arm2", "Arm3", amphix::Transform(amphix::IQ(), amphix::Vector3(0.8, 0.0, -0.5)), amphix::Vector3(0, 1.0, 0.0));
    
    robot->BuildKinematicStructure();

    std::unique_ptr<amphix::Servo> srv1 = std::make_unique<amphix::Servo>("Servo1", 1.0, 1.0, 1000.0);
    srv1->SetControlMode(amphix::ServoControlMode::POSITION);
    robot->AddJointActuator(std::move(srv1), "joint1");

    std::unique_ptr<amphix::Servo> srv2 = std::make_unique<amphix::Servo>("Servo2", 1.0, 1.0, 1000.0);
    srv2->SetControlMode(amphix::ServoControlMode::POSITION);
    robot->AddJointActuator(std::move(srv2), "joint2");

    std::unique_ptr<amphix::Servo> srv3 = std::make_unique<amphix::Servo>("Servo3", 1.0, 1.0, 1000.0);
    srv3->SetControlMode(amphix::ServoControlMode::POSITION);
    robot->AddJointActuator(std::move(srv3), "joint3");

    robot->AddLinkActuator(std::make_unique<amphix::SuctionCup>("Suction"), "Arm3", amphix::Transform(amphix::IQ(), amphix::Vector3(0.0, 0.0, 0.0)));

    AddRobot(std::move(robot), amphix::Transform(amphix::IQ(), amphix::Vector3(0.0,0.0,-0.75)));

    std::unique_ptr<amphix::Box> tallbox = std::make_unique<amphix::Box>("BoxBig", phy, amphix::Vector3(0.2,0.2,0.6), amphix::I4(), "Plastic","green");
    tallbox->ScalePhysicalPropertiesToArbitraryMass(1.0);
    AddSolidEntity(std::move(tallbox), amphix::Transform(amphix::IQ(), amphix::Vector3(0.7, 0.0, -0.3) ));
}
