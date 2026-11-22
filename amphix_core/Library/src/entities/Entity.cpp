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
//  Entity.cpp
//  Amphix
//
//  Created by Patryk Cieslak on 11/28/12.
//  Copyright (c) 2012-2026 Patryk Cieslak. All rights reserved.
//

#include "entities/Entity.h"

#include "core/SimulationApp.h"
#include "core/SimulationManager.h"
#include "graphics/OpenGLContent.h"

namespace amphix
{

Entity::Entity(std::string_view uniqueName)
{
    name_ = SimulationApp::GetApp()->GetSimulationManager()->GetNameManager()->AddName(uniqueName);
    renderable_ = true;
}

Entity::~Entity()
{
    if(SimulationApp::GetApp() != nullptr)
        SimulationApp::GetApp()->GetSimulationManager()->GetNameManager()->RemoveName(name_);
}

void Entity::SetRenderable(bool render)
{
    renderable_ = render;
}

bool Entity::IsRenderable() const
{
    return renderable_;
}

const std::string& Entity::GetName() const
{
    return name_;
}
        
}
