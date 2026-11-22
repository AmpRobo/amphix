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
//  main.cpp
//  LearningTest
//
//  Created by Patryk Cieslak on 20/05/2025.
//  Copyright (c) 2025-2026 Patryk Cieslak. All rights reserved.
//

#include <core/GraphicalSimulationApp.h>
#include <sensors/scalar/RotaryEncoder.h>
#include <actuators/Motor.h>
#include <sensors/Sample.h>

#include "LearningTestManager.h"

struct LearningThreadData
{
    amphix::SimulationApp& sim;
};

int learning(void* data)
{
    amphix::SimulationApp& simApp = static_cast<LearningThreadData*>(data)->sim;
    amphix::SimulationManager* simManager = simApp.GetSimulationManager();

    // Wait for app to be ready
    while(simApp.GetState() == amphix::SimulationState::NOT_READY)
    {
        SDL_Delay(10);
    }

    // Start the simulation (includes building the scenario)
    simApp.StartSimulation();
    
    // Repeatedly step the simulation
    while(simApp.GetState() != amphix::SimulationState::FINISHED)
    {
        // Step simulation
        simApp.StepSimulation();

        // Get the observations
        amphix::Scalar angle1 = static_cast<amphix::RotaryEncoder*>(simManager->GetSensor("Encoder1"))->GetLastSample().GetValue(0);
        amphix::Scalar angle2 = static_cast<amphix::RotaryEncoder*>(simManager->GetSensor("Encoder2"))->GetLastSample().GetValue(0);

        // Compute command
        amphix::Scalar command = btCos(angle1 * angle2) * 15;

        // Apply actuator commands
        static_cast<amphix::Motor*>(simManager->GetActuator("Motor"))->SetTorque(command);
    }
    
    return 0;
}

int main(int argc, const char * argv[])
{
    // Define the simulation settings
    amphix::RenderSettings s;
    s.windowW = 1200;
    s.windowH = 900;
    s.aa = amphix::RenderQuality::HIGH;
    s.shadows = amphix::RenderQuality::HIGH;
    s.ao = amphix::RenderQuality::HIGH;
    s.atmosphere = amphix::RenderQuality::HIGH;
    s.ocean = amphix::RenderQuality::DISABLED;
    s.verticalSync = false;
    
    amphix::HelperSettings h;
    h.showFluidDynamics = false;
    h.showCoordSys = false;
    h.showBulletDebugInfo = false;
    h.showSensors = false;
    h.showActuators = false;
    h.showForces = false;
    
    // Create the simulation manager and application
    // Here you define the sampling frequency of the simulation;
    // higher frequency means more accurate simulation but taking more time to compute...
    amphix::GraphicalSimulationApp app("LearningTest", std::string(DATA_DIR_PATH), s, h, std::make_unique<LearningTestManager>(1000.0));
    
    // Start the learning thread
    LearningThreadData data {app};
    SDL_Thread* learningThread = SDL_CreateThread(learning, "learningThread", &data);

    // Start the main application loop (no automatic start and no automatic step!)
    // Here you define the time step that will be used to advance the simulation.
    app.Run(false, false, amphix::Scalar(0.1));

    // Clean-up
    int status {0};
    SDL_WaitThread(learningThread, &status);
    return status;
}
