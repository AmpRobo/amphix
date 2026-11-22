/*    
    This file is a part of amphix_ros_wrapper.

    amphix_ros_wrapper is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    amphix_ros_wrapper is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

//
//  ROS2ConsoleSimulationApp.cpp
//  amphix_ros_wrapper
//
//  Created by Patryk Cieslak on 02/10/23.
//  Copyright (c) 2023-2026 Patryk Cieslak. All rights reserved.
//

#include "amphix_ros_wrapper/ROS2SimulationManager.h"
#include "amphix_ros_wrapper/ROS2ConsoleSimulationApp.h"

namespace amphix
{

ROS2ConsoleSimulationApp::ROS2ConsoleSimulationApp(std::string_view title, std::string_view dataPath, std::unique_ptr<ROS2SimulationManager> sim)
    : ConsoleSimulationApp(title, dataPath, std::move(sim))
{
}

bool ROS2ConsoleSimulationApp::Startup()
{
    Init();
    if (!static_cast<ROS2SimulationManager*>(GetSimulationManager())->IsScenarioLoaded())
        return false;
    StartSimulation();
    return true;
}

void ROS2ConsoleSimulationApp::Shutdown()
{
    // Join the simulation thread before tearing down (see ROS2GraphicalSimulationApp::Shutdown)
    if (state_ == SimulationState::RUNNING || state_ == SimulationState::FINISHED)
        StopSimulation();
    CleanUp();
}

}