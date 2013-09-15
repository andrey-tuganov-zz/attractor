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

#include "OpenCVFrameCaptor.h"

#include <iostream>
#include <sstream>
#include <iomanip>

#include "error.h"
#include "global.h"

//using namespace cv;
using namespace std;

OpenCVFrameCaptor::OpenCVFrameCaptor() : FrameCaptor ()
{
    m_writer = nullptr;
}

OpenCVFrameCaptor::~OpenCVFrameCaptor()
{
    if ( m_writer )
    {
        m_writer->release();
        delete m_writer;
    }
}

void OpenCVFrameCaptor::init()
{
    int windowWidth = global::par().getInt("windowWidth");
    int windowHeight = global::par().getInt("windowHeight");

    m_frame.create(windowHeight,windowWidth,CV_8UC3);

    if ( m_frame.empty() )
        error::throw_ex("unable to initialize cv::Mat frame object",__FILE__,__LINE__);

    CvSize size;
    size.width = windowWidth;
    size.height = windowHeight;
    m_writer = new cv::VideoWriter(global::par().getString("exportFilename"), CV_FOURCC('H','F','Y','U'), 30.,  size);

    if ( m_writer == nullptr || !m_writer->isOpened() )
        error::throw_ex("unable to initialise cv::VideoWriter",__FILE__,__LINE__);

}

void OpenCVFrameCaptor::capture()
{
    if ( !m_writer )
        return;

    glPixelStorei(GL_PACK_ALIGNMENT, (m_frame.step & 3) ? 1 : 4);
    glPixelStorei(GL_PACK_ROW_LENGTH, m_frame.step/m_frame.elemSize());
    glReadPixels(0, 0, m_frame.cols, m_frame.rows, GL_BGR, GL_UNSIGNED_BYTE, m_frame.data);

    flip(m_frame, m_frame, 0);

    (*m_writer) << m_frame;

    ++m_frameID;

    if ( m_frameID%100 == 0 )
        cout << "Exporting frame " << m_frameID << endl;

}

void OpenCVFrameCaptor::release()
{
    if ( m_writer )
    {
        m_writer->release();
        delete m_writer;
    }
}

