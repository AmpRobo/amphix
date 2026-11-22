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
//  ConsoleTestManager.cpp
//  Amphix
//
//  Created by Patryk Cieslak on 18/09/2018.
//  Copyright(c) 2018-2026 Patryk Cieslak. All rights reserved.
//

#include "ConsoleTestManager.h"

#include <entities/statics/Plane.h>
#include <entities/solids/Sphere.h>
#include <core/FeatherstoneRobot.h>
#include <utils/UnitSystem.h>
#include <utils/SystemUtil.hpp>
#include <core/ScenarioParser.h>
#include <core/Console.h>

ConsoleTestManager::ConsoleTestManager(amphix::Scalar stepsPerSecond) 
    : SimulationManager(stepsPerSecond, amphix::Solver::SI, amphix::CollisionFilter::EXCLUSIVE)
{
}

void ConsoleTestManager::BuildScenario()
{
#ifdef PARSED_SCENARIO
    amphix::ScenarioParser parser(this);
    bool success = parser.Parse(amphix::GetDataPath() + "console_test.scn");
    if(success)
        cInfo("Scenario description parsed successfully.");
    else
    {
        cError("Errors detected when parsing scenario description!");
        auto log = parser.getLog();
        for(size_t i=0; i<log.size(); ++i)
        {
            switch(log[i].type)
            {
                case amphix::MessageType::INFO:
                    cInfo(log[i].text.c_str());
                    break;
                
                case amphix::MessageType::ERROR:
                    cError(log[i].text.c_str());
                    break;

                case amphix::MessageType::WARNING:
                    cWarning(log[i].text.c_str());
                    break;

                case amphix::MessageType::CRITICAL:
                    cCritical(log[i].text.c_str());
                    break;
            }
        }
    }
#else
    //Create materials
    CreateMaterial("Rock", amphix::UnitSystem::Density(amphix::CGS, amphix::MKS, 3.0), 0.8);
    SetMaterialsInteraction("Rock", "Rock", 0.9, 0.7);
    
    //Build scene	
    AddStaticEntity(std::make_unique<amphix::Plane>("Bottom", 1000.0, "Rock"), amphix::Transform(amphix::IQ(), amphix::Vector3(0,0,0)));
    
    amphix::PhysicsSettings phy;
    phy.mode = amphix::PhysicsMode::SURFACE;
    phy.collisions = true;
    
    std::vector<std::unique_ptr<amphix::SolidEntity>> links;
    links.push_back(std::make_unique<amphix::Sphere>("Sphere2", phy, 0.1, amphix::I4(), "Rock", ""));
    
    std::unique_ptr<amphix::FeatherstoneRobot> robot = std::make_unique<amphix::FeatherstoneRobot>("Robot", false);
    robot->DefineLinks(std::make_unique<amphix::Sphere>("Sphere1", phy, 0.1, amphix::I4(), "Rock", ""), std::move(links));
    robot->DefineRevoluteJoint("Joint1", 
                               "Sphere1", 
                               "Sphere2", 
                               amphix::Transform(amphix::IQ(), amphix::Vector3(0.5,0,0.0)), 
                               amphix::Vector3(0.0,1.0,0.0), 
                               std::make_pair<amphix::Scalar,amphix::Scalar>(-1.0,1.0));
	robot->BuildKinematicStructure();
    AddRobot(std::move(robot), amphix::I4());
#endif
}

void ConsoleTestManager::SimulationStepCompleted(amphix::Scalar timeStep)
{
    cInfo("Simulation time: %1.3lf", GetSimulationTime());
}
