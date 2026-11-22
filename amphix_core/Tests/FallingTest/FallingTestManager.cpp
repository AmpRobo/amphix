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
//  FallingTestManager.cpp
//  Amphix
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright (c) 2014-2026 Patryk Cieslak. All rights reserved.
//

#include "FallingTestManager.h"

#include "FallingTestApp.h"
#include <core/Robot.h>
#include <utils/SystemUtil.hpp>
#include <utils/UnitSystem.h>
#include <entities/statics/Plane.h>
#include <entities/statics/Obstacle.h>
#include <entities/solids/Polyhedron.h>
#include <entities/solids/Box.h>
#include <entities/solids/Sphere.h>
#include <entities/solids/Torus.h>
#include <entities/solids/Cylinder.h>
#include <graphics/OpenGLContent.h>
#include <sensors/scalar/IMU.h>
#include <sensors/scalar/RotaryEncoder.h>
#include <sensors/scalar/Odometry.h>
#include <actuators/Light.h>

FallingTestManager::FallingTestManager(amphix::Scalar stepsPerSecond) 
    : SimulationManager(stepsPerSecond, amphix::Solver::SI, amphix::CollisionFilter::EXCLUSIVE)
{
}

void FallingTestManager::BuildScenario()
{
    SetIcSolverParams(false);

    ///////MATERIALS////////
    CreateMaterial("Ground", 1000.0, 1.0);
    CreateMaterial("Steel", amphix::UnitSystem::Density(amphix::CGS, amphix::MKS, 1.0), 0.1);
    SetMaterialsInteraction("Ground", "Ground", 0.5, 0.3);
    SetMaterialsInteraction("Ground", "Steel", 0.5, 0.3);
    SetMaterialsInteraction("Steel", "Steel", 0.5, 0.3);
    
    ///////LOOKS///////////
    CreateLook("Grid", amphix::Color::Gray(1.f), 0.8f, 0.f, 0.0f, amphix::GetShaderPath() + "grid.png");
    CreateLook("Green", amphix::Color::RGB(0.1f, 0.8f, 0.1f), 0.5f, 0.0f);
    
    ////////OBJECTS
    AddStaticEntity(std::make_unique<amphix::Plane>("Floor", 10000.f, "Ground", "Grid"), amphix::Transform::getIdentity());
	
	AddStaticEntity(std::make_unique<amphix::Obstacle>("Dragon", amphix::GetDataPath() + "dragon.obj", 0.2f, amphix::I4(), false, "Steel", "Green"),
        amphix::Transform(amphix::Quaternion(0.0, M_PI/2, 0.0), amphix::V0()));

	AddStaticEntity(std::make_unique<amphix::Obstacle>("Pillar", amphix::Vector3(0.5,0.3,4.0), amphix::I4(), "Steel", "Green"), 
        amphix::Transform(amphix::IQ(), amphix::Vector3(-2.0,0.0,-2.0)));
	
	for(int i=1; i<=10; ++i)
		for(int k=0; k<5; ++k)
		{
			AddStaticEntity(std::make_unique<amphix::Obstacle>("Box", amphix::Vector3(0.45,0.45,sin(i/2.0)+0.2*i), amphix::I4(), "Steel", "Green"), 
                amphix::Transform(amphix::IQ(), amphix::Vector3(0.5*i+5.0,0.5*k,-sin(i/2.0)/2.0-0.2*i/2.0)));
		}
	
	amphix::Light* spot = static_cast<amphix::Light*>(AddActuator(std::make_unique<amphix::Light>("Spot", 0.2, 50.0, amphix::Color::BlackBody(5000.0), 1000.0)));
    spot->AttachToWorld(amphix::Transform(amphix::Quaternion(0,0,M_PI/3.0), amphix::Vector3(0.0,1.0,-1.0)));
}
