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
//  UnderwaterTestManager.h
//  Amphix
//
//  Created by Patryk Cieslak on 04/03/2014.
//  Copyright(c) 2014-2026 Patryk Cieslak. All rights reserved.
//

#pragma once

#include <core/SimulationManager.h>
#include <iostream>

//#define PARSED_SCENARIO

namespace amphix
{
    class FLS;
    class MSIS;
    class SSS;
}

class UnderwaterTestManager : public amphix::SimulationManager
{
public:
    UnderwaterTestManager(amphix::Scalar stepsPerSecond);
    
    void BuildScenario();
    void SimulationStepCompleted(amphix::Scalar timeStep);
    
private:
    void FLSDataCallback(amphix::FLS* fls);
    void MSISDataCallback(amphix::MSIS* msis);
    void SSSDataCallback(amphix::SSS* sss);

    template <typename T>
    void PrintData(void* data, size_t length)
    {
        for (size_t i=0; i<length; ++i)
            std::cout << std::to_string(((T*)data)[i]) << ", ";
    }
};
