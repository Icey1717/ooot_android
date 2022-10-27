/*
* Copyright (C) 2010 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#include <malloc.h>

#include <jni.h>
#include <errno.h>

#include <string.h>
#include <unistd.h>
#include <sys/resource.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>

#include <android/log.h>
#include "android_native_app_glue.h"

#include "global.h"
#include <string>
#include <vector>
#include "audiomgr.h"
#include "../controller/tas.h"
#include "../options.h"
#include "padmgr.h"
#include "ultra64/exception.h"
#include "ultra64/rcp.h"
#include "ultra64/types.h"
#include "ultra64/vi.h"
#include "z64audio.h"
#include "z64game.h"
#include "def/audio.h"
#include "def/audioMgr.h"
#include "def/sys_cfb.h"
#include "def/z_debug.h"

// f32 qNaN0x3FFFFF;
f32 qNaN0x10000;
// f32 sNaN0x3FFFFF;

SchedContext gSchedContext;
PadMgr gPadMgr;
uintptr_t gSegments[NUM_SEGMENTS];

AudioMgr gAudioMgr;

u8 osAppNmiBuffer[0x40];
u32 osResetType = 0;

// u64 osClockRate = OS_CLOCK_RATE;
s32 osViClock = VI_NTSC_CLOCK;
u32 __osShutdown = 0;
OSHWIntr __OSGlobalIntMask = OS_IM_ALL;

u32 osRomBase;

u32 osMemSize = 0x800000;

// void (*D_801755D0)(void) = NULL;

void xxxDebugDisplay_Init(void)
{
}

struct DebugDispObject* xxxDebugDisplay_AddObject(f32 posX, f32 posY, f32 posZ, s16 rotX, s16 rotY, s16 rotZ, f32 scaleX, f32 scaleY, f32 scaleZ, u8 red, u8 green, u8 blue, u8 alpha, s16 type, struct GraphicsContext* gfxCtx)
{
	return NULL;
}

__osHwInt __osHwIntTable[4] = { 0 };

s32 __osDisableInt(void)
{
	return 0;
}

void __osRestoreInt(s32 a)
{
}

void osInvalICache(void* vaddr, s32 nbytes)
{
}

void osInvalDCache(void* vaddr, s32 nbytes)
{
}

void __osEnqueueAndYield(OSThread** a)
{
}

void __osEnqueueThread(OSThread** a, OSThread* b)
{
}

OSThread* __osPopThread(OSThread** a)
{
	return NULL;
}

void __osDispatchThread()
{
}

void __osCleanupThread(void)
{
}

void __osSetWatchLo(u32 a)
{
}

void osUnmapTLBAll(void)
{
}

void __osSetFpcCsr(u32 a)
{
}

u32 __osGetFpcCsr()
{
	return 0;
}

void osWritebackDCache(void* vaddr, s32 nbytes)
{
}

void __osSetSR(u32 a)
{
}

u32 __osGetSR()
{
	return 0;
}

OSIntMask osSetIntMask(OSIntMask a)
{
	return 0;
}

s32 osSetTimer(OSTimer* timer, OSTime countdown, OSTime interval, OSMesgQueue* mq, OSMesg msg)
{
	return 0;
}

s32 osStopTimer(OSTimer* timer)
{
	return 0;
}

void __osSetCompare(u32 a)
{
}

void main_func(void);

extern f32 gViConfigXScale;
extern f32 gViConfigYScale;
extern u32 gViConfigFeatures;

void ParseCommandLineArguments(const std::vector<std::string>& commands)
{
	auto size_left = commands.size();
	for (size_t i = 0; i < commands.size(); i++)
	{
		auto& cmd = commands[i];
		size_left--;

		/*if (cmd == "-tas" && size_left > 0) TODO FIX
		{
			auto tas_filename = std::move(commands[++i]);
			oot::hid::tas::setTasFileName(tas_filename);
			oot::hid::tas::playTas(true);
		}*/
	}
}

void run()
{
	oot::setMaxFramerate(oot::config().game().framerate());
	s16* msg;

	gViConfigFeatures = OS_VI_GAMMA_OFF | OS_VI_DITHER_FILTER_ON;
	gViConfigXScale = 1.0f;
	gViConfigYScale = 1.0f;

	osSyncPrintf("mainproc execution start\n"); // "Start running"
	SysCfb_Init(0);

	func_800636C0();

	R_ENABLE_ARENA_DBG = 0;

	AudioMgr_Init(&gAudioMgr, NULL, NULL, 0xA, NULL, NULL);

	main_func();
}

int main2(int argc, char** argv)
{
	std::vector<std::string> commands;
	for (int i = 1; i < argc; i++)
		commands.push_back(argv[i]);

	ParseCommandLineArguments(commands);
	run();
	return 0;
}

#if defined(_MSC_VER)
#include <string.h>

void bzero(void* __s, size_t __n)
{
	memset(__s, '\0', __n);
}

void bcopy(void* __s, void* __d, size_t __n)
{
	memmove(__d, __s, __n);
}
#endif


#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "AndroidProject1.NativeActivity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "AndroidProject1.NativeActivity", __VA_ARGS__))

/**
* Our saved state data.
*/
struct saved_state {
	float angle;
	int32_t x;
	int32_t y;
};

/**
* Shared state for our app.
*/
struct engine {
	struct android_app* app;

	ASensorManager* sensorManager;
	const ASensor* accelerometerSensor;
	ASensorEventQueue* sensorEventQueue;

	int animating;
	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
	int32_t width;
	int32_t height;
	struct saved_state state;
};

/**
* Initialize an EGL context for the current display.
*/
static int engine_init_display(struct engine* engine) {
	// initialize OpenGL ES and EGL

	/*
	* Here specify the attributes of the desired configuration.
	* Below, we select an EGLConfig with at least 8 bits per color
	* component compatible with on-screen windows
	*/
	const EGLint attribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_NONE
	};
	EGLint w, h, format;
	EGLint numConfigs;
	EGLConfig config;
	EGLSurface surface;
	EGLContext context;

	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	eglInitialize(display, 0, 0);

	/* Here, the application chooses the configuration it desires. In this
	* sample, we have a very simplified selection process, where we pick
	* the first EGLConfig that matches our criteria */
	eglChooseConfig(display, attribs, &config, 1, &numConfigs);

	/* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
	* guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
	* As soon as we picked a EGLConfig, we can safely reconfigure the
	* ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

	ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

	surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
	context = eglCreateContext(display, config, NULL, NULL);

	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
		LOGW("Unable to eglMakeCurrent");
		return -1;
	}

	eglQuerySurface(display, surface, EGL_WIDTH, &w);
	eglQuerySurface(display, surface, EGL_HEIGHT, &h);

	engine->display = display;
	engine->context = context;
	engine->surface = surface;
	engine->width = w;
	engine->height = h;
	engine->state.angle = 0;

	// Initialize GL state.
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
	glEnable(GL_CULL_FACE);
	glShadeModel(GL_SMOOTH);
	glDisable(GL_DEPTH_TEST);

	return 0;
}

/**
* Just the current frame in the display.
*/
static void engine_draw_frame(struct engine* engine) {
	if (engine->display == NULL) {
		// No display.
		return;
	}

	// Just fill the screen with a color.
	glClearColor(((float)engine->state.x) / engine->width, engine->state.angle,
		((float)engine->state.y) / engine->height, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	eglSwapBuffers(engine->display, engine->surface);
}

/**
* Tear down the EGL context currently associated with the display.
*/
static void engine_term_display(struct engine* engine) {
	if (engine->display != EGL_NO_DISPLAY) {
		eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (engine->context != EGL_NO_CONTEXT) {
			eglDestroyContext(engine->display, engine->context);
		}
		if (engine->surface != EGL_NO_SURFACE) {
			eglDestroySurface(engine->display, engine->surface);
		}
		eglTerminate(engine->display);
	}
	engine->animating = 0;
	engine->display = EGL_NO_DISPLAY;
	engine->context = EGL_NO_CONTEXT;
	engine->surface = EGL_NO_SURFACE;
}

/**
* Process the next input event.
*/
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
	struct engine* engine = (struct engine*)app->userData;
	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
		engine->state.x = AMotionEvent_getX(event, 0);
		engine->state.y = AMotionEvent_getY(event, 0);
		return 1;
	}
	return 0;
}

/**
* Process the next main command.
*/
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
	struct engine* engine = (struct engine*)app->userData;
	switch (cmd) {
	case APP_CMD_SAVE_STATE:
		// The system has asked us to save our current state.  Do so.
		engine->app->savedState = malloc(sizeof(struct saved_state));
		*((struct saved_state*)engine->app->savedState) = engine->state;
		engine->app->savedStateSize = sizeof(struct saved_state);
		break;
	case APP_CMD_INIT_WINDOW:
		// The window is being shown, get it ready.
		if (engine->app->window != NULL) {
			engine_init_display(engine);
			engine_draw_frame(engine);
		}
		break;
	case APP_CMD_TERM_WINDOW:
		// The window is being hidden or closed, clean it up.
		engine_term_display(engine);
		break;
	case APP_CMD_GAINED_FOCUS:
		// When our app gains focus, we start monitoring the accelerometer.
		if (engine->accelerometerSensor != NULL) {
			ASensorEventQueue_enableSensor(engine->sensorEventQueue,
				engine->accelerometerSensor);
			// We'd like to get 60 events per second (in microseconds).
			ASensorEventQueue_setEventRate(engine->sensorEventQueue,
				engine->accelerometerSensor, (1000L / 60) * 1000);
		}
		break;
	case APP_CMD_LOST_FOCUS:
		// When our app loses focus, we stop monitoring the accelerometer.
		// This is to avoid consuming battery while not being used.
		if (engine->accelerometerSensor != NULL) {
			ASensorEventQueue_disableSensor(engine->sensorEventQueue,
				engine->accelerometerSensor);
		}
		// Also stop animating.
		engine->animating = 0;
		engine_draw_frame(engine);
		break;
	}
}

extern "C"
{
	__attribute__((visibility("default"))) int SDL_main(int argc, char* argv[])
	{
		const char* szWorkingDirectory = "/data/data/com.OOT_Android/files/";

		int iDirResult = chdir(szWorkingDirectory);

		if (iDirResult < 0)
		{
			return 1;
		}

		run();
	}
}

/**
* This is the main entry point of a native application that is using
* android_native_app_glue.  It runs in its own thread, with its own
* event loop for receiving input events and doing other things.
*/
void android_main(struct android_app* state) {
	struct engine engine;

	memset(&engine, 0, sizeof(engine));
	state->userData = &engine;
	state->onAppCmd = engine_handle_cmd;
	state->onInputEvent = engine_handle_input;
	engine.app = state;

	android_app_read_cmd(state);;

	// Prepare to monitor accelerometer
	engine.sensorManager = ASensorManager_getInstance();
	engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager,
		ASENSOR_TYPE_ACCELEROMETER);
	engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager,
		state->looper, LOOPER_ID_USER, NULL, NULL);

	if (state->savedState != NULL) {
		// We are starting with a previous saved state; restore from it.
		engine.state = *(struct saved_state*)state->savedState;
	}

	engine.animating = 1;

	const char* szWorkingDirectory = "/data/data/com.OOT_Android/files/";

	int iDirResult = chdir(szWorkingDirectory);

	if (iDirResult < 0)
	{
		return;
	}

	run();

	// loop waiting for stuff to do.

	while (1) {
		// Read all pending events.
		int ident;
		int events;
		struct android_poll_source* source;

		// If not animating, we will block forever waiting for events.
		// If animating, we loop until all events are read, then continue
		// to draw the next frame of animation.
		while ((ident = ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events,
			(void**)&source)) >= 0) {

			// Process this event.
			if (source != NULL) {
				source->process(state, source);
			}

			// If a sensor has data, process it now.
			if (ident == LOOPER_ID_USER) {
				if (engine.accelerometerSensor != NULL) {
					ASensorEvent event;
					while (ASensorEventQueue_getEvents(engine.sensorEventQueue,
						&event, 1) > 0) {
						LOGI("accelerometer: x=%f y=%f z=%f",
							event.acceleration.x, event.acceleration.y,
							event.acceleration.z);
					}
				}
			}

			// Check if we are exiting.
			if (state->destroyRequested != 0) {
				engine_term_display(&engine);
				return;
			}
		}

		if (engine.animating) {
			// Done with events; draw next animation frame.
			engine.state.angle += .01f;
			if (engine.state.angle > 1) {
				engine.state.angle = 0;
			}

			// Drawing is throttled to the screen update rate, so there
			// is no need to do timing here.
			engine_draw_frame(&engine);
		}
	}
}
