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
//  amphix_simulator_nogpu.cpp
//  amphix_ros_wrapper
//
//  Created by Patryk Cieslak on 05/10/23.
//  Copyright (c) 2023-2026 Patryk Cieslak. All rights reserved.
//

#include "rclcpp/rclcpp.hpp"
#include <AmphixCore/utils/SystemUtil.hpp>
#include "amphix_ros_wrapper/ROS2SimulationManager.h"
#include "amphix_ros_wrapper/ROS2ConsoleSimulationApp.h"

using namespace std::chrono_literals;

class AmphixConsoleNode : public rclcpp::Node
{
public:
    AmphixConsoleNode(const std::string& scenarioPath,
                           const std::string& dataPath,
                           amphix::Scalar rate) 
                           : Node("amphix_simulator_nogpu")
    {   
        app_ = std::make_unique<amphix::ROS2ConsoleSimulationApp>("Amphix Simulator", dataPath,
            std::make_unique<amphix::ROS2SimulationManager>(rate, scenarioPath,
                std::shared_ptr<rclcpp::Node>(this, [](rclcpp::Node*) {})));
        loaded_ = app_->Startup();
        if (!loaded_)
        {
            RCLCPP_FATAL(get_logger(), "Scenario failed to load, shutting down.");
        }
    };

    void Shutdown()
    {
        app_->Shutdown();
    };

    bool IsLoaded() const
    {
        return loaded_;
    }

private:
    bool loaded_ = false;
    std::unique_ptr<amphix::ROS2ConsoleSimulationApp> app_;
};

int main(int argc, char **argv)
{
	rclcpp::init(argc, argv);
    
    //Check number of command line arguments
	if(argc < 4)
	{
		std::cout << "Not enough command-line arguments provided!" << std::endl;
		exit(-1);
	}

    //Parse the arguments
    std::string dataPath = std::string(argv[1]) + "/";
    std::string scenarioPath(argv[2]);
    amphix::Scalar rate = atof(argv[3]);

    // Start simulation
    std::shared_ptr<AmphixConsoleNode> node(new AmphixConsoleNode(scenarioPath, dataPath, rate));
    if (node->IsLoaded())
        rclcpp::spin(node);
    node->Shutdown();
    rclcpp::shutdown();
    return node->IsLoaded() ? 0 : 1;
}
