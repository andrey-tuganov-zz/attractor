// Copyright (c) 2013 Andrey Tuganov
//
// The zlib/libpng License
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

#include "gltools.h"

#include <iostream>
#include <cstring>

#include "Application.h"
#include "LorenzAttractorDemo.h"
#include "Solver.h"
#include "Demo.h"
#include "FrameCaptor.h"

#include "global.h"
#include "error.h"

using namespace std;

static Application *instance = nullptr;

Application *Application::get()
{
	if(!instance)
		instance = new Application();

	return instance;
}

Application::Application()
{
	m_window = nullptr;
	m_simTime = 0.f;
	m_simDeltaTime = 0.f;
	m_cursorX = 0.f;
	m_cursorY = 0.f;
}

Application::~Application()
{
	if ( m_window )
	{
		glfwDestroyWindow(m_window);
		m_window = nullptr;
	}
}

void error_callback(int error, const char* description)
{
	error::throw_ex(description);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

void cursor_pos_callback(GLFWwindow* window, double dx,double dy)
{
	if (Application::get())
		Application::get()->setCursorPos(float(dx),float(dy));
}

void Application::init()
{
	glfwSetErrorCallback(error_callback);

	if( !glfwInit() )
		error::throw_ex("unable to initialize GLFW",__FILE__,__LINE__);

	int windowWidth = global::par().getInt("windowWidth");
	int windowHeight = global::par().getInt("windowHeight");
	string windowTitle = global::par().getString("windowTitle");

	m_window = glfwCreateWindow(windowWidth, windowHeight, windowTitle.c_str(), nullptr, nullptr);

	if( !m_window )
	{
		glfwTerminate();
		error::throw_ex("unable to create GLFW window",__FILE__,__LINE__);
	}

	glfwMakeContextCurrent(m_window);

	if ( glewInit() != GLEW_OK )
		error::throw_ex("unable to initialize GLEW",__FILE__,__LINE__);

	glfwSetKeyCallback(m_window,key_callback);
	//glfwSetCursorPosCallback(m_window, cursor_pos_callback);

	glViewport(0, 0, windowWidth, windowHeight);

}

void Application::run()
{
	setupLorenzAttractor();

	mainLoop();

	glfwDestroyWindow(m_window);

	m_window = nullptr;

	glfwTerminate();

}

void Application::mainLoop()
{
	int framesLastSecond = 0;
	int lastSecond = 0;

	int curFrame = 0;
	int exportStartFrame = global::par().getInt("exportStartFrame");
	int simulationEndFrame = global::par().getInt("simulationEndFrame");

	while (!glfwWindowShouldClose(m_window))
	{
		float realTime = getRealTime();
		++framesLastSecond;
		if ( lastSecond != (int)realTime )
		{
			lastSecond = (int)realTime;
			cout << "FPS: " << framesLastSecond << endl;
			framesLastSecond = 0;
		}

		Demo::get()->render(m_simTime);

		glfwSwapBuffers(m_window);

		if ( FrameCaptor::get() && curFrame >= exportStartFrame )
			FrameCaptor::get()->capture();

		if ( simulationEndFrame && curFrame == simulationEndFrame )
			break;

		Solver::get()->step(m_simTime,m_simDeltaTime);

		Demo::get()->update();

		glfwPollEvents();

		m_simTime += m_simDeltaTime;

		++curFrame;
	}

	if ( FrameCaptor::get() )
		FrameCaptor::get()->release();
}

void Application::setCursorPos(float x, float y)
{
	m_cursorX = x;
	m_cursorY = y;
}

float Application::getRealTime()
{
	return glfwGetTime();
}

float Application::getSimTime()
{
	return m_simTime;
}

void Application::setupLorenzAttractor()
{
	m_simTime = 0.f;
	m_simDeltaTime = 1.f/60.f;

	int nRows = 256;
	int nParticles = nRows*nRows*nRows;

	global::par().setInt("nParticles",nParticles);

	global::par().setString("vertexShaderFilename","shader/lorenz.vert");
	global::par().setString("fragmentShaderFilename","shader/lorenz.frag");

	global::par().setString("kernelFilename","kernel/lorenz.cl");
	global::par().enable("CL_GL_interop");

	void *onePiece = nullptr;
	//if ( posix_memalign(&buffer, 16, 8*nParticles*sizeof(float)) || buffer == nullptr )
	onePiece = (float*) malloc(8*nParticles*sizeof(float));
	if ( onePiece == nullptr )
		error::throw_ex("memory allocation failed",__FILE__,__LINE__);
	// TODO write a reasonable memory manager, for now just keep the memory allocated till the end of the application

	float *pos = (float *)onePiece;
	float *color = pos + 4*nParticles;

	memset(color,0,4*nParticles*sizeof(float));

#if 0
	auto initState = [](float *pos, float x, float y, float z, float spread)
	{
		pos[0] = x+spread*(2.f*float(rand())/RAND_MAX-1.f);
		pos[1] = x+spread*(2.f*float(rand())/RAND_MAX-1.f);
		pos[2] = x+spread*(2.f*float(rand())/RAND_MAX-1.f);
		pos[3] = 1.f;
	};

	for( int i = 0; i < nParticles; ++i )
	{
		/*
		if ( i < nParticles*0.1 )
			initState(&pos[4*i],-10.f,0.f,30.f,20.f);
		else if ( i < nParticles*0.2 )
			initState(&pos[4*i],10.f,10.f,10.f,20.f);
		else if ( i < nParticles*0.3 )
			initState(&pos[4*i],-10.f,-10.f,100.f,20.f);
		else*/
		initState(pos+4*i,0.f,0.f,0.f,100.f);
	}
#endif

	{
		float side = 100.f;
		for( int i = 0; i < nRows; ++i )
		{
			for( int j = 0; j < nRows; ++j )
			{
				for( int k = 0; k < nRows; ++k )
				{
					int idx = 4*((i*nRows+k)*nRows+j);
					pos[idx+0] = side*float(2*i-nRows)/float(nRows);
					pos[idx+1] = side*float(2*j-nRows)/float(nRows);
					pos[idx+2] = side*float(2*k-nRows)/float(nRows);
					pos[idx+3] = 1.f;
				}
			}
		}
	}

	global::par().setPtr("pos",(void*)pos);
	global::par().setPtr("color",(void*)color);

	Demo::create(Demo::LorenzAttractor); // would throw if failed
	Solver::create(Solver::LorenzAttractorOpenCL);

	Demo::get()->init();
	Solver::get()->init();

	if ( global::par().isEnabled("export") )
	{
		FrameCaptor::create(FrameCaptor::OpenCV);
		FrameCaptor::get()->init();
	}
}

