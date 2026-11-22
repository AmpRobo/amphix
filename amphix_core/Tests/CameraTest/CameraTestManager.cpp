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
//  CameraTestManager.cpp
//  Amphix
//
//  Created by Patryk Cieslak on 09/02/2024.
//  Copyright (c) 2024-2026 Patryk Cieslak. All rights reserved.
//

#include "CameraTestManager.h"

#include <utils/UnitSystem.h>
#include <utils/SystemUtil.hpp>
#include <entities/statics/Plane.h>
#include <entities/statics/Obstacle.h>
#include <entities/AnimatedEntity.h>
#include <entities/animation/PWLTrajectory.h>
#include <entities/animation/BSTrajectory.h>
#include <entities/animation/CRTrajectory.h>
#include <sensors/vision/ColorCamera.h>
#include <sensors/vision/DepthCamera.h>
#include <sensors/vision/ThermalCamera.h>
#include <sensors/vision/OpticalFlowCamera.h>
#include <sensors/vision/EventBasedCamera.h>
#include <sensors/vision/SegmentationCamera.h>
#include <iostream>

CameraTestManager::CameraTestManager(amphix::Scalar stepsPerSecond)
   : SimulationManager(stepsPerSecond, amphix::Solver::SI, amphix::CollisionFilter::EXCLUSIVE)
{
}

void CameraTestManager::BuildScenario()
{
    EnableOcean();
    GetAtmosphere()->SetConditions(20.0, 101300.0, 0.7);
    GetOcean()->SetConditions(15.0);

    ///////MATERIALS////////
    CreateMaterial("Ground", 1000.0, 1.0);
    CreateMaterial("Steel", amphix::UnitSystem::Density(amphix::CGS, amphix::MKS, 1.0), 0.1);
    SetMaterialsInteraction("Ground", "Ground", 0.5, 0.3);
    SetMaterialsInteraction("Ground", "Steel", 0.5, 0.3);
    SetMaterialsInteraction("Steel", "Steel", 0.5, 0.3);

    ///////LOOKS///////////
    CreateLook("Grid", amphix::Color::Gray(1.f), 0.8f, 0.f, 0.0f, amphix::GetShaderPath() + "grid.png");
    CreateLook("Green", amphix::Color::RGB(0.1f, 0.1f, 0.1f), 0.2f, 0.0f);
#ifdef TEST_THERMAL_CAMERA
    CreateLook("Boat", amphix::Color::Gray(1.f), 0.9f, 0.f, 0.0f, amphix::GetDataPath() + "aquadelmo_tex.png");
    CreateLook("Skin", amphix::Color::HSV(0.07, 0.516, 0.8), 1.0f, 0.0f, 0.0f, "", "", "", std::make_pair(36.6f, 36.6f));
#else
    CreateLook("Canyon", amphix::Color::Gray(1.f), 0.9f, 0.f, 0.0f, amphix::GetDataPath() + "canyon.png");
#endif

    ////////OBJECTS
    AddStaticEntity(std::make_unique<amphix::Plane>("Floor", 10000.f, "Ground", "Grid"), 
        amphix::Transform(amphix::Quaternion(0.,0.,0.), amphix::Vector3(0.0, 0.0, 100.0)));

#ifdef TEST_THERMAL_CAMERA
	AddStaticEntity(std::make_unique<amphix::Obstacle>("Boat", amphix::GetDataPath() + "aquadelmo.obj", 1.0, amphix::I4(), amphix::GetDataPath() + "aquadelmo_phy.obj", 1.0, amphix::I4(), true, "Steel", "Boat"), 
        amphix::Transform(amphix::Quaternion(1.3,0,0.0), amphix::Vector3(0.0,0.0,0)));
	AddStaticEntity(std::make_unique<amphix::Obstacle>("Human", amphix::GetDataPath() + "human.obj", 0.1, amphix::I4(), true, "Steel", "Skin"),
        amphix::Transform(amphix::Quaternion(1.3,0,0.0), amphix::Vector3(0.0,0.0,0.0)));
#else    
    AddStaticEntity(std::make_unique<amphix::Obstacle>("Canyon", amphix::GetDataPath() + "canyon.obj", 1.0, amphix::I4(), false, "Ground", "Canyon"), 
        amphix::Transform(amphix::IQ(), amphix::Vector3(0.0, 0.0, 10.0)));
#endif
    
    std::unique_ptr<amphix::BSTrajectory> camTraj = std::make_unique<amphix::BSTrajectory>(amphix::PlaybackMode::REPEAT);
#ifdef TEST_THERMAL_CAMERA
    camTraj->AddKeyPoint(amphix::Scalar(0.), amphix::Transform(amphix::Quaternion(0.,0.,0.), amphix::Vector3(7.0, 0.0, -1.0)));
    camTraj->AddKeyPoint(amphix::Scalar(5.), amphix::Transform(amphix::Quaternion(M_PI_2,0.,0.), amphix::Vector3(0.0, 7.0, -1.0)));
    camTraj->AddKeyPoint(amphix::Scalar(10.), amphix::Transform(amphix::Quaternion(M_PI,0.,0.), amphix::Vector3(-7.0, 0.0, -1.0)));
    camTraj->AddKeyPoint(amphix::Scalar(15.), amphix::Transform(amphix::Quaternion(1.5*M_PI,0.,0.), amphix::Vector3(0.0, -7.0, -1.0)));
    camTraj->AddKeyPoint(amphix::Scalar(20.), amphix::Transform(amphix::Quaternion(0.,0.,0.), amphix::Vector3(7.0, 0.0, -1.0)));
#else
    camTraj->AddKeyPoint(amphix::Scalar(0.), amphix::Transform(amphix::Quaternion(-2.67, 0., 0.), amphix::Vector3(-9.6, -4.8, 9.0)));
    camTraj->AddKeyPoint(amphix::Scalar(2.), amphix::Transform(amphix::Quaternion(-2.67, 0., 0.), amphix::Vector3(-9.6, -4.8, 9.0)));
    camTraj->AddKeyPoint(amphix::Scalar(4.5), amphix::Transform(amphix::Quaternion(-1.86, 0., 0.), amphix::Vector3(-6.33, 1.4, 9.0)));
    camTraj->AddKeyPoint(amphix::Scalar(6.5), amphix::Transform(amphix::Quaternion(-2.85, 0., 0.), amphix::Vector3(-2.13, 8.08, 9.0)));
    camTraj->AddKeyPoint(amphix::Scalar(8.), amphix::Transform(amphix::Quaternion(-3.5, 0., 0.), amphix::Vector3(5.03, 7.02, 8.0)));
    camTraj->AddKeyPoint(amphix::Scalar(9.), amphix::Transform(amphix::Quaternion(M_PI_2, 0.,0.), amphix::Vector3(7.0, 5.0, 9.0)));
    camTraj->AddKeyPoint(amphix::Scalar(10.), amphix::Transform(amphix::Quaternion(M_PI_2, 0.,0.), amphix::Vector3(7.0, 0.0, 9.0)));
    camTraj->AddKeyPoint(amphix::Scalar(12.), amphix::Transform(amphix::Quaternion(0.85, 0.,0.), amphix::Vector3(6.85, -7.2, 9.0)));
    camTraj->AddKeyPoint(amphix::Scalar(14.), amphix::Transform(amphix::Quaternion(0.0, 0.,0.), amphix::Vector3(-5.75, -11.0, 9.0)));
    camTraj->AddKeyPoint(amphix::Scalar(17.), amphix::Transform(amphix::Quaternion(0.0, 0.,0.), amphix::Vector3(-11.0, -11.0, 9.0)));
#endif

    amphix::AnimatedEntity* camFrame = AddAnimatedEntity(std::make_unique<amphix::AnimatedEntity>("CamFrame", std::move(camTraj)));

    std::unique_ptr<amphix::ColorCamera> cCam = std::make_unique<amphix::ColorCamera>("ColorCamera", 400, 300, amphix::Scalar(90.0));
    cCam->SetDisplayOnScreen(true, 200, 0, 1.0);
    cCam->AttachToSolid(camFrame, amphix::Transform(amphix::Quaternion(-M_PI_2, 0.0, M_PI_2), amphix::Vector3(0,0,0)));
    AddSensor(std::move(cCam));

#ifdef TEST_THERMAL_CAMERA
    std::unique_ptr<amphix::ThermalCamera> tCam = std::make_unique<amphix::ThermalCamera>("ThermalCamera", 400, 300, amphix::Scalar(90.0), -100.f, 100.f); 
    tCam->SetNoise(0.2);
    tCam->SetDisplaySettings(amphix::ColorMap::JET, 5.0, 50.0);
    tCam->SetDisplayOnScreen(true, 600, 0, 1.0);
    tCam->AttachToSolid(camFrame, amphix::Transform(amphix::Quaternion(-M_PI_2, 0.0, M_PI_2), amphix::Vector3(0,0,0)));
    AddSensor(std::move(tCam));
#else
    std::unique_ptr<amphix::DepthCamera> dCam = std::make_unique<amphix::DepthCamera>("DepthCamera", 400, 300, amphix::Scalar(90), amphix::Scalar(0.01), amphix::Scalar(10.0));
    dCam->SetNoise(0.01);
    dCam->SetDisplayOnScreen(true, 200, 300, 1.0);
    dCam->AttachToSolid(camFrame, amphix::Transform(amphix::Quaternion(-M_PI_2, 0.0, M_PI_2), amphix::Vector3(0,0,0)));
    AddSensor(std::move(dCam));

    std::unique_ptr<amphix::OpticalFlowCamera> ofCam = std::make_unique<amphix::OpticalFlowCamera>("OpticalFlowCamera", 400, 300, amphix::Scalar(90));
    ofCam->SetDisplayOnScreen(true, 600, 0, 1.0);
    ofCam->SetDisplaySettings(800.0);
    ofCam->AttachToSolid(camFrame, amphix::Transform(amphix::Quaternion(-M_PI_2, 0.0, M_PI_2), amphix::Vector3(0,0,0)));
    AddSensor(std::move(ofCam));

    std::unique_ptr<amphix::EventBasedCamera> evbCam = std::make_unique<amphix::EventBasedCamera>("EventBasedCamera", 400, 300, amphix::Scalar(90.0), 0.1f, 0.1f, 1000, 10.0);
    evbCam->SetNoise(0.03, 0.03);
    evbCam->SetDisplayOnScreen(true, 600, 300, 1.0);
    evbCam->AttachToSolid(camFrame, amphix::Transform(amphix::Quaternion(-M_PI_2, 0.0, M_PI_2), amphix::Vector3(0,0,0)));
#ifdef DEBUG
    evbCam->InstallNewDataHandler([this](amphix::EventBasedCamera* cam)
    {
        std::cout << "EBC last event count: " << cam->getLastEventCount() << std::endl;
    });
#endif
    AddSensor(std::move(evbCam));
#endif
    std::unique_ptr<amphix::SegmentationCamera> sCam = std::make_unique<amphix::SegmentationCamera>("SegmentationCamera", 400, 300, amphix::Scalar(90.0));
    sCam->SetDisplayOnScreen(true, 200, 600, 1.0);
    sCam->AttachToSolid(camFrame, amphix::Transform(amphix::Quaternion(-M_PI_2, 0.0, M_PI_2), amphix::Vector3(0,0,0)));
    AddSensor(std::move(sCam));
}