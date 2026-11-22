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
//  FloatingTestManager.cpp
//  Amphix
//
//  Created by Patryk Cieslak on 02/05/2019.
//  Copyright (c) 2019-2026 Patryk Cieslak. All rights reserved.
//

#include "FloatingTestManager.h"

#include <entities/solids/Polyhedron.h>
#include <actuators/Thruster.h>
#include <utils/UnitSystem.h>
#include <utils/SystemUtil.hpp>
#include <core/FeatherstoneRobot.h>
#include <entities/FeatherstoneEntity.h>
#include <sensors/scalar/Odometry.h>
#include <sensors/Sample.h>

FloatingTestManager::FloatingTestManager(amphix::Scalar stepsPerSecond)
   : SimulationManager(stepsPerSecond, amphix::Solver::SI, amphix::CollisionFilter::EXCLUSIVE)
{
}

void FloatingTestManager::BuildScenario()
{
    ///////MATERIALS////////
    CreateMaterial("Fiberglass", amphix::UnitSystem::Density(amphix::CGS, amphix::MKS, 1.5), 0.3);
    SetMaterialsInteraction("Fiberglass", "Fiberglass", 0.5, 0.2);
    
    ///////LOOKS///////////
    CreateLook("white", amphix::Color::Gray(1.f), 0.9f, 0.0f, 0.f);
    CreateLook("propeller", amphix::Color::Gray(1.f), 0.3f, 0.f, 0.f, amphix::GetDataPath() + "propeller_tex.png");

    ////////OBJECTS    
    //Create environment
    EnableOcean(0.0);
    GetOcean()->SetWaterType(0.2);
    GetAtmosphere()->SetSunPosition(0.0, 60.0);
    
    //Hull
    amphix::PhysicsSettings phy;
    phy.mode = amphix::PhysicsMode::FLOATING;
    phy.collisions = true;
    phy.buoyancy = true;
    std::unique_ptr<amphix::Polyhedron> hull = std::make_unique<amphix::Polyhedron>("Hull", phy, amphix::GetDataPath() + "boat_gra.obj", amphix::Scalar(1), amphix::I4(), 
        amphix::GetDataPath() + "boat.obj", amphix::Scalar(1), amphix::I4(), "Fiberglass", "white", amphix::Scalar(0.09));
    hull->ScalePhysicalPropertiesToArbitraryMass(150.0);

    //Propeller
    phy.mode = amphix::PhysicsMode::SUBMERGED;
    phy.buoyancy = false;

    std::unique_ptr<amphix::Polyhedron> propeller = std::make_unique<amphix::Polyhedron>("Propeller", phy, amphix::GetDataPath() + "propeller.obj", amphix::Scalar(1), amphix::I4(), "Fiberglass", "propeller");
    std::unique_ptr<amphix::MechanicalPI> rotorDynamics =  std::make_unique<amphix::MechanicalPI>(1.0, 10.0, 5.0, 5.0);
    std::unique_ptr<amphix::FDThrust> thrustModel = std::make_unique<amphix::FDThrust>(0.18, 0.48, 0.48, 0.05, true, GetOcean()->GetLiquid().density);
    std::unique_ptr<amphix::Thruster> thrust = std::make_unique<amphix::Thruster>("Thruster", std::move(propeller), std::move(rotorDynamics), std::move(thrustModel), 
        0.18, true, 105.0, false, true);

    //Sensors   
    std::unique_ptr<amphix::Odometry> odom = std::make_unique<amphix::Odometry>("Odom");
    
    //Boat
    std::unique_ptr<amphix::FeatherstoneRobot> boat = std::make_unique<amphix::FeatherstoneRobot>("Boat");
    boat->DefineLinks(std::move(hull));
    boat->BuildKinematicStructure();
    amphix::LinkActuator* th = boat->AddLinkActuator(std::move(thrust), "Hull", amphix::Transform(amphix::Quaternion(0,0,0), amphix::Vector3(-0.8,0.02,0.3)));
    boat->AddLinkSensor(std::move(odom), "Hull", amphix::I4());
    AddRobot(std::move(boat), amphix::Transform(amphix::IQ(), amphix::Vector3(0,0,0)));
    
    static_cast<amphix::Thruster*>(th)->SetSetpoint(1.0);
}