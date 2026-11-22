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
//  ROS2GraphicalSimulationApp.h
//  amphix_ros_wrapper
//
//  Created by Patryk Cieslak on 02/10/23.
//  Copyright (c) 2023-2026 Patryk Cieslak. All rights reserved.
//

#pragma once

#include <AmphixCore/core/GraphicalSimulationApp.h>

namespace amphix
{
    class ROS2SimulationManager;

    class ROS2GraphicalSimulationApp : public GraphicalSimulationApp
    {
    public:
        ROS2GraphicalSimulationApp(std::string title, std::string dataPath, RenderSettings s, HelperSettings h, std::unique_ptr<ROS2SimulationManager> sim);
        //! Builds the scenario and starts the simulation; returns false if the scenario failed to load.
        bool Startup();
        void Tick();
        void Shutdown();

    private:
        bool cleanedUp_;
    };
}
