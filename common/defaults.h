/*
defaults.h - set up default configuration
Copyright (C) 2016 Mittorn

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef DEFAULTS_H
#define DEFAULTS_H

#include "backends.h"

/*
===================================================================

SETUP BACKENDS DEFINITIONS

===================================================================
*/
#ifndef XASH_DEDICATED

	#ifdef XASH_SDL

		// by default, use SDL subsystems
		#ifndef XASH_VIDEO
			#define XASH_VIDEO VIDEO_SDL
		#endif // XASH_VIDEO

		#ifndef XASH_TIMER
			#define XASH_TIMER TIMER_SDL
		#endif

		#ifndef XASH_INPUT
			#define XASH_INPUT INPUT_SDL
		#endif

		#ifndef XASH_SOUND
			#define XASH_SOUND SOUND_SDL
		#endif

	#endif //XASH_SDL

	#if defined __ANDROID__ && !defined XASH_SDL

		#ifndef XASH_VIDEO
			#define XASH_VIDEO VIDEO_ANDROID
		#endif

		#ifndef XASH_TIMER
			#define XASH_TIMER TIMER_LINUX
		#endif

		#ifndef XASH_INPUT
			#define XASH_INPUT INPUT_ANDROID
		#endif

		#ifndef XASH_SOUND
			#define XASH_SOUND SOUND_OPENSLES
		#endif
	#endif // android case

#endif // XASH_DEDICATED

// no timer - no xash
#ifndef XASH_TIMER
	#ifdef _WIN32
		#define XASH_TIMER TIMER_WIN32
	#else
		#define XASH_TIMER TIMER_LINUX
	#endif
#endif

//
// fallback to NULL
//
#ifndef XASH_VIDEO
	#define XASH_VIDEO VIDEO_NULL
#endif

#ifndef XASH_SOUND
	#define XASH_SOUND SOUND_NULL
#endif

#ifndef XASH_INPUT
	#define XASH_INPUT INPUT_NULL
#endif

/*
=========================================================================

Default build-depended cvar and constant values

=========================================================================
*/

#if defined __ANDROID__ || defined __SAILFISH__
	#define DEFAULT_TOUCH_ENABLE "1"
	#define DEFAULT_M_IGNORE "1"
#else
	#define DEFAULT_TOUCH_ENABLE "0"
	#define DEFAULT_M_IGNORE "0"
#endif

#if defined __ANDROID__ || defined __SAILFISH__
// this means that libraries are provided with engine, but not in game data
// You need add library loading code to library.c when adding new platform
#define XASH_INTERNAL_GAMELIBS
#endif

#if defined XASH_NANOGL || defined XASH_WES
#ifndef XASH_GLES
#define XASH_GLES
#endif // XASH_GLES
#ifndef XASH_GL_STATIC
#define XASH_GL_STATIC
#endif // XASH_GL_STATIC
#endif // XASH_NANOGL || XASH_WES

#define DEFAULT_PRIMARY_MASTER "ms.tyabus.co.uk:27010"
#define FWGS_PRIMARY_MASTER "mentality.rip:27010"
// Set ForceSimulating to 1 by default for dedicated, because AMXModX timers require this
// TODO: enable simulating for any server?
#ifdef XASH_DEDICATED
	#define DEFAULT_SV_FORCESIMULATING "1"
#else
	#define DEFAULT_SV_FORCESIMULATING "0"
#endif

// Big endian cpus are mostly low end and cant keep up with this on
#ifdef XASH_BIG_ENDIAN
	#define DEFAULT_SV_ALLOWCOMPRESSION "0"
#else
	#define DEFAULT_SV_ALLOWCOMPRESSION "1"
#endif

#define DEFAULT_SLEEPTIME "1"

// allow override for developer/debug builds
#ifndef DEFAULT_DEV
	#define DEFAULT_DEV 0
#endif

#ifndef DEFAULT_FULLSCREEN
#define DEFAULT_FULLSCREEN 1
#endif

#define DEFAULT_CON_MAXFRAC "1"

#endif // DEFAULTS_H
