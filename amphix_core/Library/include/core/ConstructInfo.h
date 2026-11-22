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
//  ConstructInfo.h
//  Amphix
//
//  Created by Patryk Cieslak on 20/07/26.
//  Copyright (c) 2026 Patryk Cieslak. All rights reserved.
//

#pragma once

#include "AmphixCommon.h"
#include "graphics/OpenGLDataStructs.h"
#include <map>
#include <unordered_map>
#include <variant>

namespace amphix
{

enum class ConstructInfoValueType {BOOL, INT, SCALAR, VECTOR3, TRANSFORM, STRING, COLORMAP};

struct ConstructInfoValue
{
    ConstructInfoValueType valueType;
    bool optional;
    std::variant<bool, int, Scalar, Vector3, Transform, std::string, ColorMap> value;  
    bool valid;

    ConstructInfoValue() : 
        valueType(ConstructInfoValueType::SCALAR),
        optional(true),
        value(Scalar(0.)),
        valid(false)
    {}

    ConstructInfoValue(ConstructInfoValueType type, bool isOptional) :
        valueType(type),
        optional(isOptional),
        value(Scalar(0.)),
        valid(false)
    {}
};

struct ConstructInfoNode
{
    std::map<std::string, ConstructInfoNode> childNodes;
    std::unordered_map<std::string, ConstructInfoValue> attributes;
    bool optional;
};

struct ConstructInfo
{
    std::unordered_map<std::string, ConstructInfoNode> nodes;
};

}