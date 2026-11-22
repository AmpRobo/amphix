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
//  UnderwaterTestManager.cpp
//  Amphix
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright(c) 2014-2026 Patryk Cieslak. All rights reserved.
//

#include "UnderwaterTestManager.h"

#include "UnderwaterTestApp.h"
#include <core/FeatherstoneRobot.h>
#include <entities/statics/Plane.h>
#include <entities/statics/Obstacle.h>
#include <entities/solids/Polyhedron.h>
#include <entities/solids/Box.h>
#include <entities/solids/Sphere.h>
#include <entities/solids/Torus.h>
#include <entities/solids/Cylinder.h>
#include <entities/solids/Compound.h>
#include <entities/solids/Wing.h>
#include <graphics/OpenGLPointLight.h>
#include <graphics/OpenGLSpotLight.h>
#include <graphics/OpenGLTrackball.h>
#include <utils/SystemUtil.hpp>
#include <entities/statics/Obstacle.h>
#include <entities/statics/Terrain.h>
#include <actuators/Thruster.h>
#include <actuators/Servo.h>
#include <actuators/VariableBuoyancy.h>
#include <sensors/scalar/Pressure.h>
#include <sensors/scalar/Odometry.h>
#include <sensors/scalar/DVL.h>
#include <sensors/scalar/Compass.h>
#include <sensors/scalar/IMU.h>
#include <sensors/scalar/GPS.h>
#include <sensors/Contact.h>
#include <sensors/vision/ColorCamera.h>
#include <sensors/vision/DepthCamera.h>
#include <sensors/vision/Lidar.h>
#include <sensors/vision/FLS.h>
#include <sensors/vision/SSS.h>
#include <sensors/vision/MSIS.h>
#include <comms/AcousticModem.h>
#include <sensors/Sample.h>
#include <actuators/Light.h>
#include <sensors/scalar/RotaryEncoder.h>
#include <sensors/scalar/Accelerometer.h>
#include <entities/FeatherstoneEntity.h>
#include <entities/forcefields/Trigger.h>
#include <entities/forcefields/Pipe.h>
#include <entities/forcefields/Jet.h>
#include <entities/forcefields/Uniform.h>
#include <entities/AnimatedEntity.h>
#include <sensors/scalar/Profiler.h>
#include <sensors/scalar/Multibeam.h>
#include <utils/UnitSystem.h>
#include <core/ScenarioParser.h>
#include <core/NED.h>
#include <iomanip>

UnderwaterTestManager::UnderwaterTestManager(amphix::Scalar stepsPerSecond)
: SimulationManager(stepsPerSecond, amphix::Solver::SI, amphix::CollisionFilter::EXCLUSIVE)
{
}

void UnderwaterTestManager::BuildScenario()
{
#ifdef PARSED_SCENARIO
    amphix::ScenarioParser parser(this);
    bool success = parser.Parse(amphix::GetDataPath() + "underwater_test.scn");
    parser.SaveLog("underwater_test.log");
    if(!success)
        cCritical("Scenario parser: Parsing failed!");
#else
    ///////MATERIALS////////
    CreateMaterial("Dummy", amphix::UnitSystem::Density(amphix::CGS, amphix::MKS, 0.9), 0.3);
    CreateMaterial("Fiberglass", amphix::UnitSystem::Density(amphix::CGS, amphix::MKS, 1.5), 0.9);
    CreateMaterial("Rock", amphix::UnitSystem::Density(amphix::CGS, amphix::MKS, 3.0), 0.6);
    SetMaterialsInteraction("Dummy", "Dummy", 0.5, 0.2);
    SetMaterialsInteraction("Fiberglass", "Fiberglass", 0.5, 0.2);
    SetMaterialsInteraction("Rock", "Rock", 0.9, 0.7);
    SetMaterialsInteraction("Fiberglass", "Dummy", 0.5, 0.2);
    SetMaterialsInteraction("Rock", "Dummy", 0.6, 0.4);
    SetMaterialsInteraction("Rock", "Fiberglass", 0.6, 0.4);
    
    ///////LOOKS///////////
    CreateLook("yellow", amphix::Color::RGB(1.f, 0.9f, 0.f), 0.3f, 0.f);
    CreateLook("grey", amphix::Color::RGB(0.3f, 0.3f, 0.3f), 0.4f, 0.5f);
    CreateLook("seabed", amphix::Color::RGB(0.7f, 0.7f, 0.5f), 0.9f, 0.f, 0.f, "", amphix::GetDataPath() + "sand_normal.png");
    CreateLook("propeller", amphix::Color::RGB(1.f, 1.f, 1.f), 0.3f, 0.f, 0.f, amphix::GetDataPath() + "propeller_tex.png");
    CreateLook("black", amphix::Color::RGB(0.1f, 0.1f, 0.1f), 0.4f, 0.5f);
    CreateLook("manipulator", amphix::Color::RGB(0.2f, 0.15f, 0.1f), 0.6f, 0.8f);
    CreateLook("link4", amphix::Color::RGB(1.f, 1.f, 1.f), 0.6f, 0.8f, 0.f, amphix::GetDataPath() + "link4_tex.png");
    
    ////////OBJECTS    
    //Create environment
    EnableOcean(0.0);
    GetOcean()->SetWaterType(0.2);
    GetOcean()->AddVelocityField(std::make_unique<amphix::Jet>(amphix::Vector3(0,0,1.0), amphix::VY(), 0.3, 5.0));
    GetOcean()->AddVelocityField(std::make_unique<amphix::Uniform>(amphix::Vector3(1.0,0.0,0.0)));
    GetOcean()->EnableVelocityFields();
    GetAtmosphere()->SetSunPosition(0.0, 60.0);
    GetNed()->Init(41.77737, 3.03376, 0.0);
    
    AddStaticEntity(std::make_unique<amphix::Terrain>("Seabed", amphix::GetDataPath() + "terrain.png", 1.0, 1.0, 5.0, "Rock", "seabed", 5.f),
        amphix::Transform(amphix::IQ(), amphix::Vector3(0,0,15.0)));
    AddStaticEntity(std::make_unique<amphix::Obstacle>("Cyl", 0.5, 5.0, amphix::I4(), "Fiberglass", "seabed"), 
        amphix::Transform(amphix::Quaternion(0,M_PI_2,0), amphix::Vector3(6.0,2.0,5.0)));
	
	std::unique_ptr<amphix::Light> spot = std::make_unique<amphix::Light>("Spot", 0.02, 50.0, amphix::Color::BlackBody(5000.0), 100.0);
	spot->AttachToWorld(amphix::Transform(amphix::Quaternion(0,0,M_PI/3.0), amphix::Vector3(0.0,0.0,1.0)));
	AddActuator(std::move(spot));
    
    std::unique_ptr<amphix::Light> omni = std::make_unique<amphix::Light>("Omni", 0.02, amphix::Color::BlackBody(5000.0), 10000.0);
	omni->AttachToWorld(amphix::Transform(amphix::Quaternion(0,0,M_PI/3.0), amphix::Vector3(2.0,2.0,0.5)));
	AddActuator(std::move(omni));

    //Create underwater vehicle body
    //Externals
    amphix::PhysicsSettings phy;
    phy.mode = amphix::PhysicsMode::SUBMERGED;
    phy.collisions = true;
    
    phy.buoyancy = false;
    std::unique_ptr<amphix::Polyhedron> hullB = std::make_unique<amphix::Polyhedron>("HullBottom", phy, amphix::GetDataPath() + "hull_hydro.obj", amphix::Scalar(1), amphix::I4(), "Fiberglass", "yellow", amphix::Scalar(0.003));
    std::unique_ptr<amphix::Polyhedron> hullP = std::make_unique<amphix::Polyhedron>("HullPort", phy, amphix::GetDataPath() + "hull_hydro.obj", amphix::Scalar(1), amphix::I4(), "Fiberglass", "yellow", amphix::Scalar(0.003));
    std::unique_ptr<amphix::Polyhedron> hullS = std::make_unique<amphix::Polyhedron>("HullStarboard", phy, amphix::GetDataPath() + "hull_hydro.obj", amphix::Scalar(1), amphix::I4(), "Fiberglass", "yellow", amphix::Scalar(0.003));
    std::unique_ptr<amphix::Polyhedron> vBarStern = std::make_unique<amphix::Polyhedron>("VBarStern", phy, amphix::GetDataPath() + "vbar_hydro.obj", amphix::Scalar(1), amphix::I4(), "Dummy", "grey", amphix::Scalar(0.003));
    std::unique_ptr<amphix::Polyhedron> vBarBow = std::make_unique<amphix::Polyhedron>("VBarBow", phy, amphix::GetDataPath() + "vbar_hydro.obj", amphix::Scalar(1), amphix::I4(), "Dummy", "grey", amphix::Scalar(0.003));

    phy.buoyancy = true;
    std::unique_ptr<amphix::Polyhedron> ductSway = std::make_unique<amphix::Polyhedron>("DuctSway", phy, amphix::GetDataPath() + "duct_hydro.obj", amphix::Scalar(1), amphix::I4(), "Dummy", "black");
    std::unique_ptr<amphix::Polyhedron> ductSurgeP = std::make_unique<amphix::Polyhedron>("DuctSurgePort", phy, amphix::GetDataPath() + "duct_hydro.obj", amphix::Scalar(1), amphix::I4(), "Dummy", "black");
    std::unique_ptr<amphix::Polyhedron> ductSurgeS = std::make_unique<amphix::Polyhedron>("DuctSurgeStarboard", phy, amphix::GetDataPath() + "duct_hydro.obj", amphix::Scalar(1), amphix::I4(), "Dummy", "black");
    std::unique_ptr<amphix::Polyhedron> ductHeaveS = std::make_unique<amphix::Polyhedron>("DuctHeaveStern", phy, amphix::GetDataPath() + "duct_hydro.obj", amphix::Scalar(1), amphix::I4(), "Dummy", "black");
    std::unique_ptr<amphix::Polyhedron> ductHeaveB = std::make_unique<amphix::Polyhedron>("DuctHeaveBow", phy, amphix::GetDataPath() + "duct_hydro.obj", amphix::Scalar(1), amphix::I4(), "Dummy", "black");
    //Internals
    std::unique_ptr<amphix::Cylinder> batteryCyl = std::make_unique<amphix::Cylinder>("BatteryCylinder", phy, 0.13, 0.6, amphix::I4(), "Dummy", "manipulator");
    batteryCyl->ScalePhysicalPropertiesToArbitraryMass(amphix::Scalar(70));
    std::unique_ptr<amphix::Cylinder> portCyl = std::make_unique<amphix::Cylinder>("PortCylinder", phy, 0.13, 1.0, amphix::I4(), "Dummy", "manipulator");
    portCyl->ScalePhysicalPropertiesToArbitraryMass(amphix::Scalar(20));
    std::unique_ptr<amphix::Cylinder> starboardCyl = std::make_unique<amphix::Cylinder>("StarboardCylinder", phy, 0.13, 1.0, amphix::I4(), "Dummy", "manipulator");
    starboardCyl->ScalePhysicalPropertiesToArbitraryMass(amphix::Scalar(20));
    
    //Build whole body
    std::unique_ptr<amphix::Compound> vehicle = std::make_unique<amphix::Compound>("Vehicle", phy, std::move(hullB), amphix::I4());
    vehicle->AddExternalPart(std::move(hullP), amphix::Transform(amphix::IQ(), amphix::Vector3(0,-0.35,-0.7)));
    vehicle->AddExternalPart(std::move(hullS), amphix::Transform(amphix::IQ(), amphix::Vector3(0,0.35,-0.7)));
    vehicle->AddExternalPart(std::move(vBarStern), amphix::Transform(amphix::Quaternion::getIdentity(), amphix::Vector3(-0.25,0.0,-0.15)));
    vehicle->AddExternalPart(std::move(vBarBow), amphix::Transform(amphix::Quaternion::getIdentity(), amphix::Vector3(0.30,0.0,-0.15)));
    vehicle->AddExternalPart(std::move(ductSway), amphix::Transform(amphix::Quaternion(M_PI_2,M_PI,0), amphix::Vector3(-0.0137, 0.0307, -0.38)));
    vehicle->AddExternalPart(std::move(ductSurgeP), amphix::Transform(amphix::Quaternion(0,0,M_PI), amphix::Vector3(-0.2807,-0.2587,-0.38)));
    vehicle->AddExternalPart(std::move(ductSurgeS), amphix::Transform(amphix::Quaternion(0,0,0), amphix::Vector3(-0.2807,0.2587,-0.38)));
    vehicle->AddExternalPart(std::move(ductHeaveS), amphix::Transform(amphix::Quaternion(M_PI_2,-M_PI_2,0), amphix::Vector3(-0.5337,0.0,-0.6747)));
    vehicle->AddExternalPart(std::move(ductHeaveB), amphix::Transform(amphix::Quaternion(-M_PI_2,-M_PI_2,0), amphix::Vector3(0.5837,0.0,-0.6747)));
    vehicle->AddInternalPart(std::move(batteryCyl), amphix::Transform(amphix::Quaternion(0,M_PI_2,0), amphix::Vector3(-0.1,0,0)));
    vehicle->AddInternalPart(std::move(portCyl), amphix::Transform(amphix::Quaternion(0,M_PI_2,0), amphix::Vector3(0.0,-0.35,-0.7)));
    vehicle->AddInternalPart(std::move(starboardCyl), amphix::Transform(amphix::Quaternion(0,M_PI_2,0), amphix::Vector3(0.0,0.35,-0.7)));
    
    vehicle->SetDisplayInternalParts(false);
    
    //Manipulator bodies
    std::unique_ptr<amphix::Polyhedron> baseLink = std::make_unique<amphix::Polyhedron>("ArmBaseLink", phy, amphix::GetDataPath() + "base_link_uji_hydro.obj", amphix::Scalar(1), amphix::I4(), "Dummy", "manipulator");
    std::unique_ptr<amphix::Polyhedron> link1 = std::make_unique<amphix::Polyhedron>("ArmLink1", phy, amphix::GetDataPath() + "link1_hydro.obj", amphix::Scalar(1), amphix::I4(), "Dummy", "manipulator");
    std::unique_ptr<amphix::Polyhedron> link2 = std::make_unique<amphix::Polyhedron>("ArmLink2", phy, amphix::GetDataPath() + "link2_hydro.obj", amphix::Scalar(1), amphix::I4(), "Dummy", "manipulator");
    std::unique_ptr<amphix::Polyhedron> link3 = std::make_unique<amphix::Polyhedron>("ArmLink3", phy, amphix::GetDataPath() + "link3_hydro.obj", amphix::Scalar(1), amphix::I4(), "Dummy", "manipulator");
    std::unique_ptr<amphix::Polyhedron> link4 = std::make_unique<amphix::Polyhedron>("ArmLink4", phy, amphix::GetDataPath() + "link4ft_hydro.obj", amphix::Scalar(1), amphix::I4(), "Dummy", "link4");
    std::unique_ptr<amphix::Polyhedron> ee = std::make_unique<amphix::Polyhedron>("EE", phy, amphix::GetDataPath() + "eeprobe_hydro.obj", amphix::Scalar(1), amphix::I4(), "Neutral", "manipulator");
    std::unique_ptr<amphix::Polyhedron> finger1 = std::make_unique<amphix::Polyhedron>("Finger1", phy, amphix::GetDataPath() + "fingerA_hydro.obj", amphix::Scalar(1), amphix::I4(), "Dummy", "manipulator");
    std::unique_ptr<amphix::Polyhedron> finger2 = std::make_unique<amphix::Polyhedron>("Finger2", phy, amphix::GetDataPath() + "fingerA_hydro.obj", amphix::Scalar(1), amphix::I4(), "Dummy", "manipulator");
    
    std::vector<std::unique_ptr<amphix::SolidEntity>> arm;
    arm.push_back(std::move(baseLink));
    arm.push_back(std::move(link1));
    arm.push_back(std::move(link2));
    arm.push_back(std::move(link3));
    arm.push_back(std::move(link4));
    arm.push_back(std::move(ee));
    arm.push_back(std::move(finger1));
    arm.push_back(std::move(finger2));
    
    //Create manipulator servomotors
    std::unique_ptr<amphix::Servo> srv1 = std::make_unique<amphix::Servo>("Servo1", 1.0, 1.0, 100.0);
    std::unique_ptr<amphix::Servo> srv2 = std::make_unique<amphix::Servo>("Servo2", 1.0, 1.0, 100.0);
    std::unique_ptr<amphix::Servo> srv3 = std::make_unique<amphix::Servo>("Servo3", 1.0, 1.0, 100.0);
    std::unique_ptr<amphix::Servo> srv4 = std::make_unique<amphix::Servo>("Servo4", 1.0, 1.0, 100.0);
    std::unique_ptr<amphix::Servo> srv5 = std::make_unique<amphix::Servo>("FServo1", 1.0, 1.0, 10.0);
    std::unique_ptr<amphix::Servo> srv6 = std::make_unique<amphix::Servo>("FServo2", 1.0, 1.0, 10.0);
    
    //Create thrusters
    std::array<std::string, 5> thrusterNames = {"ThrusterSway", "ThrusterSurgePort", "ThrusterSurgeStarboard", "ThrusterHeaveStern", "ThrusterHeaveBow"}; 
    std::array<std::unique_ptr<amphix::Thruster>, 5> thrusters;
    for(size_t i=0; i<thrusterNames.size(); ++i)
    {
        std::unique_ptr<amphix::Polyhedron> propeller = std::make_unique<amphix::Polyhedron>("Propeller", phy, amphix::GetDataPath() + "propeller.obj", amphix::Scalar(1), amphix::I4(), "Dummy", "propeller");
        std::unique_ptr<amphix::MechanicalPI> rotorDynamics;
        rotorDynamics = std::make_unique<amphix::MechanicalPI>(1.0, 10.0, 5.0, 5.0);
        std::unique_ptr<amphix::FDThrust> thrustModel;
        thrustModel = std::make_unique<amphix::FDThrust>(0.18, 0.48, 0.48, 0.05, true, GetOcean()->GetLiquid().density);
        thrusters[i] = std::make_unique<amphix::Thruster>(thrusterNames[i], std::move(propeller), std::move(rotorDynamics), std::move(thrustModel), 0.18, true, 105.0, true, true);
    }

    //Create VBS
    //std::vector<std::string> vmeshes;
    //vmeshes.push_back(amphix::GetDataPath() + "vbs_max.obj");
    //vmeshes.push_back(amphix::GetDataPath() + "vbs_min.obj");
    //std::unique_ptr<VariableBuoyancy> vbs = std::make_unique<amphix::VariableBuoyancy>("VBS", vmeshes, 0.002);
       
    //Create sensors
    std::unique_ptr<amphix::Odometry> odom = std::make_unique<amphix::Odometry>("Odom");
    
    std::unique_ptr<amphix::Pressure> press = std::make_unique<amphix::Pressure>("Pressure");
    press->SetNoise(1.0);
    
    std::unique_ptr<amphix::DVL> dvl = std::make_unique<amphix::DVL>("DVL", 30.0, false);
    dvl->SetNoise(0.0, 0.02, 0.05, 0.0, 0.02);
    
    std::unique_ptr<amphix::IMU> imu = std::make_unique<amphix::IMU>("IMU");
    imu->SetNoise(amphix::V0(), amphix::Vector3(0.05, 0.05, 0.1), 0.0, amphix::Vector3(0.01, 0.01, 0.02));
    
    std::unique_ptr<amphix::Compass> fog = std::make_unique<amphix::Compass>("FOG");
    fog->SetNoise(0.01);
    
    std::unique_ptr<amphix::GPS> gps = std::make_unique<amphix::GPS>("GPS");
    gps->SetNoise(0.5);
    
    //std::unique_ptr<amphix::Lidar> mb = std::make_unique<amphix::Lidar>("Multibeam", 1000, 300, 50.0, 40.0, 0.1, 10.0, 10.0);
    //mb->SetDisplayOnScreen(true);
    
    //std::unique_ptr<amphix::DepthCamera> dc = std::make_unique<amphix::DepthCamera>("DepthCam", 1000, 350, 50.0, 0.1, 10.0, 10.0);
    //dc->SetDisplayOnScreen(true);
    
    std::unique_ptr<amphix::FLS> fls = std::make_unique<amphix::FLS>("FLS", 256, 500, 150.0, 30.0, 1.0, 20.0, amphix::SonarOutputFormat::U8);
    fls->SetNoise(0.05, 0.05);
    fls->SetDisplayOnScreen(true, 800, 250, 0.4f);
    //fls->InstallNewDataHandler(std::bind(&UnderwaterTestManager::FLSDataCallback, this, std::placeholders::_1));

    std::unique_ptr<amphix::MSIS> msis = std::make_unique<amphix::MSIS>("MSIS", 1.5, 500, 2.0, 30.0, -50, 50, 1.0, 100.0, amphix::SonarOutputFormat::U8);
    msis->SetDisplayOnScreen(true, 880, 455, 0.6f);
    //msis->InstallNewDataHandler(std::bind(&UnderwaterTestManager::MSISDataCallback, this, std::placeholders::_1));
    
    std::unique_ptr<amphix::SSS> sss = std::make_unique<amphix::SSS>("SSS", 800, 400, 70.0, 1.5, 50.0, 1.0, 100.0, amphix::SonarOutputFormat::U8);
    sss->SetDisplayOnScreen(true, 710, 5, 0.6f);
    //sss->InstallNewDataHandler(std::bind(&UnderwaterTestManager::SSSDataCallback, this, std::placeholders::_1));
    
    //std::unique_ptr<amphix::ColorCamera> cam = std::make_unique<amphix::ColorCamera>("Cam", 300, 200, 60.0, 10.0);
    //cam->SetDisplayOnScreen(true);
    
    //std::unique_ptr<amphix::ColorCamera> cam2 = std::make_unique<amphix::ColorCamera>("Cam", 300, 200, 60.0);
    
    //Create AUV
    std::unique_ptr<amphix::Robot> auv = std::make_unique<amphix::FeatherstoneRobot>("GIRONA500", false);
    
    //Mechanical structure
    auv->DefineLinks(std::move(vehicle), std::move(arm));
    auv->DefineFixedJoint("VehicleToArm", "Vehicle", "ArmBaseLink", amphix::Transform(amphix::IQ(), amphix::Vector3(0.74,0.0,0.0)));
    auv->DefineRevoluteJoint("Joint1", "ArmBaseLink", "ArmLink1", amphix::I4(), amphix::Vector3(0.0, 0.0, 1.0), std::make_pair(-1.0, 1.0));
    auv->DefineRevoluteJoint("Joint2", "ArmLink1", "ArmLink2", amphix::Transform(amphix::IQ(), amphix::Vector3(0.1065, 0.0, 0.0)), amphix::Vector3(0.0, 1.0, 0.0), std::make_pair(-1.0, 1.0));
    auv->DefineRevoluteJoint("Joint3", "ArmLink2", "ArmLink3", amphix::Transform(amphix::IQ(), amphix::Vector3(0.23332, 0.0, 0.0)), amphix::Vector3(0.0, 1.0, 0.0), std::make_pair(-1.0, 1.0));
    auv->DefineRevoluteJoint("Joint4", "ArmLink3", "ArmLink4", amphix::Transform(amphix::IQ(), amphix::Vector3(0.103, 0.0, 0.201)), amphix::Vector3(0.0, 0.0, 1.0),  std::make_pair(-1.0, 1.0));
    auv->DefineFixedJoint("Fix", "ArmLink4", "EE", amphix::Transform(amphix::IQ(), amphix::Vector3(0.0, 0.0, 0.05)));
    auv->DefineRevoluteJoint("Joint5", "EE", "Finger1", amphix::Transform(amphix::IQ(), amphix::Vector3(0.03,0,0.1)), amphix::VY(), std::make_pair(0.0, 1.0));
    auv->DefineRevoluteJoint("Joint6", "EE", "Finger2", amphix::Transform(amphix::Quaternion(M_PI, 0.0, 0.0), amphix::Vector3(-0.03,0,0.1)), amphix::VY(), std::make_pair(0.0, 1.0));
    auv->BuildKinematicStructure();
    
    //Joint motors
    auv->AddJointActuator(std::move(srv1), "Joint1");
    auv->AddJointActuator(std::move(srv2), "Joint2");
    auv->AddJointActuator(std::move(srv3), "Joint3");
    auv->AddJointActuator(std::move(srv4), "Joint4");
    auv->AddJointActuator(std::move(srv5), "Joint5");
    auv->AddJointActuator(std::move(srv6), "Joint6");
    
    //Thrusters
    auv->AddLinkActuator(std::move(thrusters[0]), "Vehicle", amphix::Transform(amphix::Quaternion(M_PI_2,M_PI,0), amphix::Vector3(-0.0137, 0.0307, -0.38)));
    auv->AddLinkActuator(std::move(thrusters[1]), "Vehicle", amphix::Transform(amphix::IQ(), amphix::Vector3(-0.2807,-0.2587,-0.38)));
    auv->AddLinkActuator(std::move(thrusters[2]), "Vehicle", amphix::Transform(amphix::IQ(), amphix::Vector3(-0.2807,0.2587,-0.38)));
    auv->AddLinkActuator(std::move(thrusters[3]), "Vehicle", amphix::Transform(amphix::Quaternion(0,-M_PI_2,0), amphix::Vector3(-0.5337,0.0,-0.6747)));
    auv->AddLinkActuator(std::move(thrusters[4]), "Vehicle", amphix::Transform(amphix::Quaternion(0,-M_PI_2,0), amphix::Vector3(0.5837,0.0,-0.6747)));
    //auv->AddLinkActuator(std::move(vbs), "Vehicle", amphix::Transform(amphix::IQ(), amphix::Vector3(-0.5,0.0,0.0)));
    
    //Sensors
    auv->AddLinkSensor(std::move(odom), "Vehicle", amphix::Transform(amphix::IQ(), amphix::Vector3(0,0,0)));
    auv->AddLinkSensor(std::move(press), "Vehicle", amphix::Transform(amphix::IQ(), amphix::Vector3(0.6,0,-0.7)));
    auv->AddLinkSensor(std::move(dvl), "Vehicle", amphix::Transform(amphix::Quaternion(-M_PI_4,0,M_PI), amphix::Vector3(-0.5,0,0.1)));
    auv->AddLinkSensor(std::move(imu), "Vehicle", amphix::Transform(amphix::IQ(), amphix::Vector3(0,0,-0.7)));
    auv->AddLinkSensor(std::move(fog), "Vehicle", amphix::Transform(amphix::IQ(), amphix::Vector3(0.3,0,-0.7)));
    auv->AddLinkSensor(std::move(gps), "Vehicle", amphix::Transform(amphix::IQ(), amphix::Vector3(-0.5,0,-0.9)));
    auv->AddVisionSensor(std::move(fls), "Vehicle", amphix::Transform(amphix::Quaternion(1.57, 0.0, 0.8), amphix::Vector3(0.0,0.0,1.0)));
    auv->AddVisionSensor(std::move(sss), "Vehicle", amphix::Transform(amphix::Quaternion(1.57, 0.0, 0.0), amphix::Vector3(0.0,0.0,0.0)));
    auv->AddVisionSensor(std::move(msis), "Vehicle", amphix::Transform(amphix::Quaternion(0.0, 0.0, 1.57), amphix::Vector3(0.0,0.0,1.0)));
    //auv->AddVisionSensor(std::move(cam), "Vehicle", amphix::Transform(amphix::Quaternion(1.57, 0.0, 1.57), amphix::Vector3(0.0,0.0,1.0)));
    //auv->AddVisionSensor(std::move(cam2), "Vehicle", amphix::Transform(amphix::Quaternion(1.57, 0.0, 1.57), amphix::Vector3(0.0,0.0,2.0)));
    AddRobot(std::move(auv), amphix::Transform(amphix::Quaternion(0,0,0), amphix::Vector3(0.0,0.0,2.0)));
#endif
} 

void UnderwaterTestManager::SimulationStepCompleted(amphix::Scalar timeStep)
{
    if(false)
    {
#ifdef PARSED_SCENARIO
        amphix::Thruster* th = dynamic_cast<amphix::Thruster*>(GetRobot("GIRONA500")->GetActuator("GIRONA500/ThrusterSurgePort"));
#else
        amphix::Thruster* th = dynamic_cast<amphix::Thruster*>(GetRobot("GIRONA500")->GetActuator("ThrusterSurgePort"));
#endif
        if (th)
        {
            double rpm = th->GetOmega() * amphix::Scalar(60.0) / amphix::Scalar(2.0 * M_PI);
            std::cout << std::setprecision(3) << "[" << th->GetName() << "] RPM: " << rpm << ", Thrust: " << th->GetThrust() << std::endl;
        }
    }
}

void UnderwaterTestManager::FLSDataCallback(amphix::FLS* fls)
{
    auto format = fls->GetOutputFormat();
    switch (format)
    {
        case amphix::SonarOutputFormat::U8:
            PrintData<GLubyte>(fls->GetImageDataPointer(), 10);
            break;

        case amphix::SonarOutputFormat::U16:
            PrintData<GLushort>(fls->GetImageDataPointer(), 10);
            break;

        case amphix::SonarOutputFormat::U32:
            PrintData<GLuint>(fls->GetImageDataPointer(), 10);
            break;

        case amphix::SonarOutputFormat::F32:
            PrintData<GLfloat>(fls->GetImageDataPointer(), 10);
            break;
    }
    std::cout << std::endl;
}

void UnderwaterTestManager::MSISDataCallback(amphix::MSIS* msis)
{
    auto format = msis->GetOutputFormat();
    switch (format)
    {
        case amphix::SonarOutputFormat::U8:
            PrintData<GLubyte>(msis->GetImageDataPointer(), 1000);
            break;

        case amphix::SonarOutputFormat::U16:
            PrintData<GLushort>(msis->GetImageDataPointer(), 1000);
            break;

        case amphix::SonarOutputFormat::U32:
            PrintData<GLuint>(msis->GetImageDataPointer(), 1000);
            break;

        case amphix::SonarOutputFormat::F32:
            PrintData<GLfloat>(msis->GetImageDataPointer(), 1000);
            break;
    }
    std::cout << std::endl;
}

void UnderwaterTestManager::SSSDataCallback(amphix::SSS* sss)
{
    auto format = sss->GetOutputFormat();
    switch (format)
    {
        case amphix::SonarOutputFormat::U8:
            PrintData<GLubyte>(sss->GetImageDataPointer(), 100);
            break;

        case amphix::SonarOutputFormat::U16:
            PrintData<GLushort>(sss->GetImageDataPointer(), 100);
            break;

        case amphix::SonarOutputFormat::U32:
            PrintData<GLuint>(sss->GetImageDataPointer(), 100);
            break;

        case amphix::SonarOutputFormat::F32:
            PrintData<GLfloat>(sss->GetImageDataPointer(), 100);
            break;
    }
    std::cout << std::endl;
}
