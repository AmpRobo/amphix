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
//  amphix_simulator.cpp
//  amphix_ros_wrapper
//
//  Created by Patryk Cieslak on 02/10/23.
//  Copyright (c) 2023-2026 Patryk Cieslak. All rights reserved.
//

#include "rclcpp/rclcpp.hpp"
#include <AmphixCore/utils/SystemUtil.hpp>
#include "amphix_ros_wrapper/ROS2SimulationManager.h"
#include "amphix_ros_wrapper/ROS2GraphicalSimulationApp.h"

using namespace std::chrono_literals;

class AmphixNode : public rclcpp::Node
{
public:
    AmphixNode(const std::string& scenarioPath,
                           const std::string& dataPath, 
                           const amphix::RenderSettings& s, 
                           const amphix::HelperSettings& h,
                           amphix::Scalar rate) 
                           : Node("amphix_simulator")
    {   
        // The app owns the manager, so its node handle must not own this node.
        app_ = std::make_unique<amphix::ROS2GraphicalSimulationApp>("Amphix Simulator", dataPath, s, h,
            std::make_unique<amphix::ROS2SimulationManager>(rate, scenarioPath,
                std::shared_ptr<rclcpp::Node>(this, [](rclcpp::Node*) {})));
        loaded_ = app_->Startup();
        if (!loaded_)
        {
            RCLCPP_FATAL(get_logger(), "Scenario failed to load, shutting down.");
            return;
        }
        tickTimer_ = this->create_wall_timer(16667us, std::bind(&amphix::ROS2GraphicalSimulationApp::Tick, app_.get()));
    };

    void Shutdown()
    {
        tickTimer_.reset();
        if (app_)
            app_->Shutdown();
    }

    bool IsLoaded() const
    {
        return loaded_;
    }

private:
    bool loaded_ = false;
    std::unique_ptr<amphix::ROS2GraphicalSimulationApp> app_;
    rclcpp::TimerBase::SharedPtr tickTimer_;
};

int main(int argc, char **argv)
{
	rclcpp::init(argc, argv);
    
    //Check number of command line arguments
	if(argc < 7)
	{
		std::cout << "Not enough command-line arguments provided!" << std::endl;
		exit(-1);
	}

    //Parse the arguments
    std::string dataPath = std::string(argv[1]) + "/";
    std::string scenarioPath(argv[2]);
    amphix::Scalar rate = atof(argv[3]);

	amphix::RenderSettings s;
    s.windowW = atoi(argv[4]);
    s.windowH = atoi(argv[5]);

    std::string quality(argv[6]);
    if(quality == "low")
    {
        s.shadows = amphix::RenderQuality::LOW;
        s.ao = amphix::RenderQuality::DISABLED;
        s.atmosphere = amphix::RenderQuality::LOW;
        s.ocean = amphix::RenderQuality::LOW;
        s.aa = amphix::RenderQuality::LOW;
        s.ssr = amphix::RenderQuality::DISABLED;
    }
    else if(quality == "high")
    {
        s.shadows = amphix::RenderQuality::HIGH;
        s.ao = amphix::RenderQuality::HIGH;
        s.atmosphere = amphix::RenderQuality::HIGH;
        s.ocean = amphix::RenderQuality::HIGH;
        s.aa = amphix::RenderQuality::HIGH;
        s.ssr = amphix::RenderQuality::HIGH;
    }
    else // "medium"
    {
        s.shadows = amphix::RenderQuality::MEDIUM;
        s.ao = amphix::RenderQuality::MEDIUM;
        s.atmosphere = amphix::RenderQuality::MEDIUM;
        s.ocean = amphix::RenderQuality::MEDIUM;
        s.aa = amphix::RenderQuality::MEDIUM;
        s.ssr = amphix::RenderQuality::MEDIUM;
    }
    // Disable vertical sync to allow for higher frame rates
    s.verticalSync = false;

    // Initialize GUI settings
    amphix::HelperSettings h;
    h.showFluidDynamics = false;
    h.showCoordSys = false;
    h.showBulletDebugInfo = false;
    h.showSensors = false;
    h.showActuators = false;
    h.showForces = false;
	
    // Start simulation
    std::shared_ptr<AmphixNode> node(new AmphixNode(scenarioPath, dataPath, s, h, rate));
    if (node->IsLoaded())
        rclcpp::spin(node);
    node->Shutdown();
    rclcpp::shutdown();
    return node->IsLoaded() ? 0 : 1;
}
