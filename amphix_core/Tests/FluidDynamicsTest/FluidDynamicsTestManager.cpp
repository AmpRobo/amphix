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
//  FluidDynamicsTestManager.cpp
//  Amphix
//
//  Created by Patryk Cieslak on 01/07/2024.
//  Copyright(c) 2024-2026 Patryk Cieslak. All rights reserved.
//

#include "FluidDynamicsTestManager.h"

#include <entities/solids/Polyhedron.h>
#include <entities/solids/Box.h>
#include <entities/solids/Sphere.h>
#include <entities/solids/Torus.h>
#include <entities/solids/Cylinder.h>
#include <entities/solids/Compound.h>
#include <actuators/SimpleThruster.h>
#include <sensors/scalar/Odometry.h>
#include <sensors/Sample.h>
#include <core/FeatherstoneRobot.h>
#include <graphics/OpenGLTrackball.h>
#include <utils/SystemUtil.hpp>
#include <utils/DebugUtil.hpp>
#include <utils/UnitSystem.h>
#include <core/NED.h>

FluidDynamicsTestManager::FluidDynamicsTestManager(amphix::Scalar stepsPerSecond)
: SimulationManager(stepsPerSecond, amphix::Solver::SI, amphix::CollisionFilter::EXCLUSIVE)
{
}

void FluidDynamicsTestManager::BuildScenario()
{    
    ///////SOLVER///////////
    amphix::Scalar erp, stopErp;
    GetJointErp(erp, stopErp);
    amphix::Scalar erp2 = GetDynamicsWorld()->getSolverInfo().m_erp2;
    amphix::Scalar globalDamping = GetDynamicsWorld()->getSolverInfo().m_damping;
    amphix::Scalar globalFriction = GetDynamicsWorld()->getSolverInfo().m_friction;
    amphix::Scalar linSleep, angSleep;
    GetSleepingThresholds(linSleep, angSleep);

    globalDamping = 0.1;
    SetSolverParams(erp, stopErp, erp2, globalDamping, globalFriction, linSleep, angSleep);
    SetFluidDynamicsPrescaler(1);
    amphix::SimulationApp::GetApp()->SetMaxPhysicsThreads(16);

    ///////MATERIALS////////
    CreateMaterial("Light", amphix::UnitSystem::Density(amphix::CGS, amphix::MKS, 0.9), 0.3);
    CreateMaterial("Neutral", amphix::UnitSystem::Density(amphix::CGS, amphix::MKS, 1.0), 0.5);
    CreateMaterial("Heavy", amphix::UnitSystem::Density(amphix::CGS, amphix::MKS, 2.0), 0.9);
    SetMaterialsInteraction("Light", "Light", 0.5, 0.2);
    SetMaterialsInteraction("Heavy", "Heavy", 0.5, 0.2);
    SetMaterialsInteraction("Neutral", "Neutral", 0.5, 0.5);
    SetMaterialsInteraction("Light", "Neutral", 0.5, 0.2);
    SetMaterialsInteraction("Heavy", "Neutral", 0.5, 0.2);
    SetMaterialsInteraction("Heavy", "Light", 0.5, 0.2);
    
    ///////LOOKS///////////
    CreateLook("Yellow", amphix::Color::RGB(1.f, 0.9f, 0.1f), 0.3f, 0.f);
    CreateLook("Red", amphix::Color::RGB(1.f, 0.1f, 0.1f), 0.3f, 0.f);
    
    ////////OBJECTS    
    EnableOcean(0.0);
    GetOcean()->SetWaterType(0.2);
    GetAtmosphere()->SetSunPosition(0.0, 60.0);

    //Externals
    amphix::PhysicsSettings phy;
    phy.mode = amphix::PhysicsMode::SUBMERGED;
    phy.collisions = true;
    phy.buoyancy = true;

    //Sphere
    amphix::Scalar sphereRadius = 0.5;
    AddSolidEntity(std::make_unique<amphix::Sphere>("Sphere", phy, sphereRadius, amphix::I4(), "Light", "Yellow"), amphix::Transform(amphix::IQ(), amphix::Vector3(0, 0, 0)));

    //Box
    amphix::Vector3 boxDims(1.0, 0.5, 0.2);
    AddSolidEntity(std::make_unique<amphix::Box>("Box", phy, boxDims, amphix::I4(), "Light", "Yellow"), amphix::Transform(amphix::IQ(), amphix::Vector3(2, 0, 0)));

    //Compound
    //1st part - sphere
    std::unique_ptr<amphix::Sphere> comp1 = std::make_unique<amphix::Sphere>("Comp1", phy, sphereRadius, amphix::I4(), "Light", "Yellow");
    //2nd part - box
    std::unique_ptr<amphix::Sphere> comp2 = std::make_unique<amphix::Sphere>("Comp2", phy, sphereRadius, amphix::I4(), "Light", "Yellow");
    std::unique_ptr<amphix::Compound> compound = std::make_unique<amphix::Compound>("Compound", phy, std::move(comp1), amphix::Transform(amphix::IQ(), amphix::Vector3(0, 0, 0)));
    compound->AddExternalPart(std::move(comp2), amphix::Transform(amphix::IQ(), amphix::Vector3(2.0, 0, 0)));
    AddSolidEntity(std::move(compound), amphix::Transform(amphix::Quaternion(0.0, 0.5, 0.0), amphix::Vector3(6, 0, -5)));

    //Polyhedron
    AddSolidEntity(std::make_unique<amphix::Polyhedron>("Hull", phy, amphix::GetDataPath() + "hull_hydro.obj", amphix::Scalar(1), amphix::I4(), "Light", "Yellow"), 
        amphix::Transform(amphix::Quaternion(0.0, 0.5, 0.0), amphix::Vector3(8, 0, -5)));
    AddSolidEntity(std::make_unique<amphix::Polyhedron>("SphereR1M", phy, amphix::GetDataPath() + "sphere_R=1.obj", amphix::Scalar(1), amphix::I4(), "Light", "Yellow"),
        amphix::Transform(amphix::Quaternion(0.0, 0.5, 0.0), amphix::Vector3(11, 0, -5)));

    //Featherstone robot
    std::unique_ptr<amphix::FeatherstoneRobot> robot = std::make_unique<amphix::FeatherstoneRobot>("Robot", false);
    robot->DefineLinks(std::make_unique<amphix::Box>("Link1", phy, boxDims, amphix::I4(), "Light", "Yellow"));
    robot->BuildKinematicStructure();
    AddRobot(std::move(robot), amphix::Transform(amphix::IQ(), amphix::Vector3(4, 0, 0)));

    // Drag testing
    // Single solid entities     
    amphix::SolidEntity* hull1 = AddSolidEntity(std::make_unique<amphix::Box>("Hull1", phy, boxDims, amphix::I4(), "Neutral", "Yellow"), 
        amphix::Transform(amphix::IQ(), amphix::Vector3(-7, 0, 1)));

    amphix::SolidEntity* hull2 = AddSolidEntity(std::make_unique<amphix::Box>("Hull2", phy, boxDims, amphix::I4(), "Neutral", "Yellow"),
        amphix::Transform(amphix::Quaternion(M_PI_2, 0, 0), amphix::Vector3(-5, 2, 1)));

    std::unique_ptr<amphix::SimpleThruster> thruster1 = std::make_unique<amphix::SimpleThruster>("Thruster1", std::make_unique<amphix::Box>("Propeller", phy, amphix::Vector3(0.02,0.2,0.05), amphix::I4(), "Neutral", "Red"),
        true, false);
    thruster1->AttachToSolid(hull1, amphix::Transform(amphix::IQ(), amphix::Vector3(-0.6, 0, 0)));
    thruster1->SetSetpoint(10.0, 0.0);
    AddActuator(std::move(thruster1));

    std::unique_ptr<amphix::SimpleThruster> thruster2 = std::make_unique<amphix::SimpleThruster>("Thruster1", std::make_unique<amphix::Box>("Propeller", phy, amphix::Vector3(0.02,0.2,0.05), amphix::I4(), "Neutral", "Red"),
        true, false);    
    thruster2->AttachToSolid(hull2, amphix::Transform(amphix::IQ(), amphix::Vector3(-0.6, 0, 0)));
    thruster2->SetSetpoint(10.0, 0.0);
    AddActuator(std::move(thruster2));

    // Compound solid entity
    for (int i = 0; i < 8; ++i)
    {
        std::unique_ptr<amphix::Box> part1 = std::make_unique<amphix::Box>("Part1_" + std::to_string(i), phy, boxDims, amphix::I4(), "Neutral", "Yellow");
        std::unique_ptr<amphix::Box> part2 = std::make_unique<amphix::Box>("Part2_" + std::to_string(i), phy, boxDims, amphix::I4(), "Neutral", "Yellow");
        std::unique_ptr<amphix::Compound> comp = std::make_unique<amphix::Compound>("Comp" + std::to_string(i), phy, std::move(part1), amphix::Transform(amphix::IQ(), amphix::Vector3(0, 0.5, 0)));
        comp->AddExternalPart(std::move(part2), amphix::Transform(amphix::IQ(), amphix::Vector3(0, -0.5, 0)));
        amphix::SolidEntity* compound = AddSolidEntity(std::move(comp), amphix::Transform(amphix::Quaternion(M_PI_4*i, 0.0, 0.0), amphix::Vector3(0, 0, 2 + 2*i)));

        std::unique_ptr<amphix::Odometry> odometry = std::make_unique<amphix::Odometry>("Odometry" + std::to_string(i));
        odometry->AttachToSolid(compound, amphix::Transform(amphix::IQ(), amphix::Vector3(0, 0, 0)));
        AddSensor(std::move(odometry));

        std::unique_ptr<amphix::SimpleThruster> thruster = std::make_unique<amphix::SimpleThruster>("ThrusterComp" + std::to_string(i), std::make_unique<amphix::Box>("Propeller", phy, amphix::Vector3(0.02,0.2,0.05), amphix::I4(), "Neutral", "Red"),
            true, false);
        thruster->AttachToSolid(compound, amphix::Transform(amphix::IQ(), amphix::Vector3(-0.6, 0, 0)));
        thruster->SetSetpoint(20.0, 0.0);
        AddActuator(std::move(thruster));
    }
}

void FluidDynamicsTestManager::SimulationStepCompleted(amphix::Scalar timeStep)
{
    std::cout << "Hydrodynamics time: " << GetPerformanceMonitor().GetHydrodynamicsTimeAverage() << " us\n";

    // std::cout << "----------------------------------------------------\n";

    // amphix::Scalar rho = GetOcean()->GetLiquid().density;

    // //SPHERE
    // amphix::Sphere* sphere = (amphix::Sphere*)GetEntity("Sphere");
    // amphix::Scalar sphereRadius = 0.5;
    // amphix::Vector3 sphereInertia = sphere->GetInertia();

    // amphix::Scalar expectedSphereVolume = 4.0 / 3.0 * M_PI * sphereRadius * sphereRadius * sphereRadius;
    // amphix::Scalar expectedSphereMass = sphere->GetMaterial().density * expectedSphereVolume;
    // amphix::Scalar expectedSphereInertia = 2.0 / 5.0 * expectedSphereMass * sphereRadius * sphereRadius;
    // amphix::Scalar expectedSphereSubmergedVolume = expectedSphereVolume * sphere->GetMaterial().density/rho;

    // amphix::testScalar("Sphere mass", sphere->GetMass(), expectedSphereMass);
    // amphix::testScalar("Sphere volume", sphere->GetVolume(), expectedSphereVolume);
    // amphix::testVector3("Sphere inertia", sphereInertia, amphix::Vector3(expectedSphereInertia, expectedSphereInertia, expectedSphereInertia));
    // amphix::testScalar("Sphere submerged volume", sphere->GetSubmergedVolume(), expectedSphereSubmergedVolume, 1e-3);
    
    // //BOX
    // amphix::Box* box = (amphix::Box*)GetEntity("Box");
    // amphix::Vector3 boxDims(1.0, 0.5, 0.2);
    // amphix::Vector3 boxInertia = box->GetInertia();

    // amphix::Scalar expectedBoxVolume = boxDims.getX() * boxDims.getY() * boxDims.getZ();
    // amphix::Scalar expectedBoxMass = box->GetMaterial().density * expectedBoxVolume;
    // amphix::Vector3 expectedBoxInertia = 1.0 / 12.0 * expectedBoxMass * amphix::Vector3(boxDims.getY() * boxDims.getY() + boxDims.getZ() * boxDims.getZ(),
    //     boxDims.getX() * boxDims.getX() + boxDims.getZ() * boxDims.getZ(), boxDims.getX() * boxDims.getX() + boxDims.getY() * boxDims.getY());
    // amphix::Scalar expectedBoxSubmergedVolume = expectedBoxVolume * box->GetMaterial().density/rho;

    // amphix::testScalar("Box mass", box->GetMass(), expectedBoxMass);
    // amphix::testScalar("Box volume", box->GetVolume(), expectedBoxVolume);
    // amphix::testVector3("Box inertia", boxInertia, expectedBoxInertia);
    // amphix::testScalar("Box submerged volume", box->GetSubmergedVolume(), expectedBoxSubmergedVolume, 1e-3);

    // //POLYHEDRON
    // amphix::Polyhedron* hull = (amphix::Polyhedron*)GetEntity("Hull");
    // amphix::Vector3 hullInertia = hull->GetInertia();
    // amphix::Transform cgTransform = hull->GetCG2OTransform().inverse();
    // amphix::Scalar expectedHullVolume = 0.096699;
    // amphix::Scalar expectedHullMass = hull->GetMaterial().density * expectedHullVolume;
    // amphix::Vector3 expectedHullInertia = hull->GetMaterial().density * amphix::Vector3(0.001080, 0.014772, 0.014772);
    // amphix::Scalar expectedHullSubmergedVolume = expectedHullVolume * hull->GetMaterial().density/rho;
    
    // amphix::testScalar("Hull mass", hull->GetMass(), expectedHullMass);
    // amphix::testScalar("Hull volume", hull->GetVolume(), expectedHullVolume);
    // amphix::testVector3("Hull inertia", hullInertia, expectedHullInertia);
    // amphix::testScalar("Hull submerged volume", hull->GetSubmergedVolume(), expectedHullSubmergedVolume);
    
    // //POLYHEDRON 2
    // amphix::Polyhedron* sphR1M = (amphix::Polyhedron*)GetEntity("SphereR1M");
    // amphix::Vector3 sphR1MInertia = sphR1M->GetInertia();
    // amphix::Transform sphR1MCGTransform = sphR1M->GetCG2OTransform().inverse();

    // amphix::Scalar expectedSphR1MVolume = 4.0 / 3.0 * M_PI;
    // amphix::Scalar expectedSphR1MMass = sphR1M->GetMaterial().density * expectedSphR1MVolume;
    // amphix::Vector3 expectedSphR1MInertia = 2.0 / 5.0 * expectedSphR1MMass * amphix::Vector3(1, 1, 1);
    // amphix::Scalar expectedSphR1MSubmergedVolume = expectedSphR1MVolume * sphR1M->GetMaterial().density/rho;

    // amphix::testScalar("SphereR1M mass", sphR1M->GetMass(), expectedSphR1MMass, 1e-2);
    // amphix::testScalar("SphereR1M volume", sphR1M->GetVolume(), expectedSphR1MVolume, 1e-2);
    // amphix::testVector3("SphereR1M inertia", sphR1MInertia, expectedSphR1MInertia, 1e-2);
    // amphix::testScalar("SphereR1M submerged volume", sphR1M->GetSubmergedVolume(), expectedSphR1MSubmergedVolume, 1e-2);

    // // DRAG TESTING
    // auto v1 = static_cast<amphix::SolidEntity*>(GetEntity("Hull1"))->GetLinearVelocity();
    // auto v2 = static_cast<amphix::SolidEntity*>(GetEntity("Hull2"))->GetLinearVelocity();

    // amphix::testScalar("Solid body drag", v1.length(), v2.length(), 0.01);

    // auto odometry = GetSensor("Odometry0");
    // auto s = static_cast<amphix::Odometry*>(odometry)->GetLastSample();
    // amphix::Scalar velX = s.GetValue(3);

    // for (int i = 1; i < 8; ++i)
    // {
    //     auto odometry = GetSensor("Odometry" + std::to_string(i));
    //     auto s = static_cast<amphix::Odometry*>(odometry)->GetLastSample();   
    //     amphix::testScalar("Comp" + std::to_string(i) + " velocity", s.GetValue(3), velX, 0.01);
    // }

    // std::cout << std::endl;
}