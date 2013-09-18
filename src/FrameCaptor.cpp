// Copyright (c) 2013 Andrey Tuganov
//
// The zlib/libpng license
//
// This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.

#include "FrameCaptor.h"

#include "OpenCVFrameCaptor.h"

#include <iostream>

#include "global.h"
#include "error.h"

using namespace std;

static FrameCaptor *instance = nullptr;

FrameCaptor *FrameCaptor::get()
{
    return instance;
}

void FrameCaptor::create(Type type)
{
    switch ( type )
    {
        case OpenCV:
            instance = new OpenCVFrameCaptor();
            break;
        default:
            break;
    }

    if ( instance == nullptr )
        error::throw_ex("unable to create frame captor",__FILE__,__LINE__);

    instance->init();
}

FrameCaptor::FrameCaptor()
{
    m_frameID = -1;
}

FrameCaptor::~FrameCaptor()
{

}

