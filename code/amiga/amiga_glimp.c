/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "../renderer/tr_local.h"
#include "../client/client.h"
#include "amiga_local.h"

#pragma amiga-align

#include <exec/exec.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <intuition/intuition.h>
#include <graphics/gfx.h>
#include <cybergraphx/cybergraphics.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/cybergraphics.h>

#ifdef __PPC__
#include <powerpc/powerpc.h>
#include <proto/powerpc.h>
#endif

#pragma default-align

#include <mgl/gl.h>

//struct GLContext_t *context = 0;
//GLboolean old_context = GL_FALSE;

extern qboolean mouse_active;
extern int mx, my;

extern cvar_t *r_finish;
cvar_t *r_closeworkbench;
cvar_t *r_guardband; //
cvar_t *r_vertexbuffersize; //
cvar_t *r_glbuffers; //
cvar_t *r_perspective_fast; //
//cvar_t *r_lockmode; //

extern cvar_t *in_nograb;

static unsigned short *mousePtr = 0;

extern struct MsgPort *Sys_EventPort;

struct Window *win = NULL;

void (APIENTRYP qglActiveTextureARB) (GLenum texture);
void (APIENTRYP qglClientActiveTextureARB) (GLenum texture);
void (APIENTRYP qglMultiTexCoord2fARB) (GLenum target, GLfloat s, GLfloat t);

void (APIENTRYP qglLockArraysEXT) (GLint first, GLsizei count); //
void (APIENTRYP qglUnlockArraysEXT) (void);

void GLimp_RenderThreadWrapper( void *stub ) {}
void GLimp_SetGamma( unsigned char red[256], unsigned char green[256], unsigned char blue[256] ) {}
void GLW_InitGamma(void) {}
void GLW_RestoreGamma(void) {}

#if 0
void GL_SetLockMode(char *mode)
{
	#ifdef AUTOMATIC_LOCKING_ENABLE // surgeon: see mgl/config.h

	if (0 == strcmp(mode, "SMART"))
	{
		ri.Printf(PRINT_ALL,"MiniGL Lock Mode: Smart\n");
		mglLockMode(MGL_LOCK_SMART);
	}

	else if (0 == strcmp(mode, "AUTO"))
	{
		ri.Printf(PRINT_ALL,"MiniGL Lock Mode: Automatic\n");
		mglLockMode(MGL_LOCK_AUTOMATIC);
	}

	else
	{
		ri.Printf(PRINT_ALL,"MiniGL Lock Mode: Manual (per Frame)\n");
		mglLockMode(MGL_LOCK_MANUAL);
	}

	#else

	ri.Printf("MiniGL Lock Mode: Manual (per Frame)\n");
	
	#endif
}
#endif

static qboolean GLW_StartDriverAndSetMode( const char *drivername, int mode, int colorbits, qboolean fullscreen )
{
	BOOL useStencil = /*TRUE*/ FALSE;

	ri.Printf(PRINT_ALL, "...setting mode %d:", mode);
	
	if (!R_GetModeInfo( &glConfig.vidWidth, &glConfig.vidHeight, &glConfig.windowAspect, mode))
	{
		ri.Printf(PRINT_ALL, " invalid mode\n");
		return qfalse;
	}
	
	if (glConfig.hardwareType == GLHW_3DFX_2D3D)
		useStencil = FALSE;
	
	//if (colorbits != 16)
		//colorbits = 24;

	colorbits = 16; // Cowcat
	
	ri.Printf( PRINT_ALL, " %d %d %s\n", glConfig.vidWidth, glConfig.vidHeight, fullscreen ? "fullscreen" : "windowed" );

	/*
	** surgeon: The highest number of verts is 4096
	** (max point-particles) and in that case no clipping-space
	** is needed. However, plenty of clippingspace is needed for
	** locked vertexarrays which offsets the transformation to
	** buffersize/4
	**/

	/*
	** MAX_VERTS is 2048 (for alias models) 
	** MiniGL offsets clip/transform to either 
	** buffersize/2 or buffersize/4 for glDrawElements 
	** Therefore 4096 is enough for the vertexbuffer 
	*/

	/*
	** The multitexture buffer is able to store
	** buffersize/4 polygons after backface-culling, which
	** means that 4096 is able to store 1024 polys with
	** max 4096 verts in total (tightly packed)
	*/

	#if 0
	if(old_context)
	{
		ri.Printf(PRINT_ALL, "Deleting old context \n");

		mglDeleteContext();
		old_context = GL_FALSE;
	}
	#endif

	//MGLInit(); // now in amiga_qgl.c/ QGL_init - 

	r_vertexbuffersize  = Cvar_Get("r_vertexbuffersize", "4096", CVAR_ARCHIVE);
	r_glbuffers  = Cvar_Get("r_glbuffers", "3", CVAR_ARCHIVE);
	r_perspective_fast = Cvar_Get("r_perspective_fast", "0", CVAR_ARCHIVE);
	//r_lockmode = Cvar_Get("r_lockmode", "SMART", CVAR_ARCHIVE);

	mglChoosePixelDepth(16); //glConfig.depthBits); 
	mglChooseVertexBufferSize( (int)r_vertexbuffersize->value ); // 4096 - 16384 //

	if(!fullscreen)
	{
		mglChooseWindowMode(GL_TRUE);
		mglChooseNumberOfBuffers(2);
		Cvar_Set("r_glbuffers", "2");
	}

	else
	{	
		mglChooseWindowMode(GL_FALSE);
		mglChooseNumberOfBuffers(3);
		Cvar_Set("r_glbuffers", "3");
	}

	r_guardband  = Cvar_Get("r_guardband", "0", CVAR_ARCHIVE);

	if(r_guardband->value)
	{
		mglChooseGuardBand(GL_TRUE);
		ri.Printf(PRINT_ALL, "guardband on\n");
	}

	else
	{
		mglChooseGuardBand(GL_FALSE);
		ri.Printf(PRINT_ALL, "guardband off\n");
	}

	#if 0 // no multitexture in this version - Cowcat
	//base the size on #of polygons to store
	gl_mtexbuffersize = ri.Cvar_Get("gl_mtexbuffersize", "1024", CVAR_ARCHIVE);

	if((int)gl_mtexbuffersize <= 1024)
		mglChooseMtexBufferSize( 4096 ); 

	else
		mglChooseMtexBufferSize( (int)gl_mtexbuffersize->value * 4);
	#endif
	
	//old_context = (NULL == mglCreateContext(0, 0, glConfig.vidWidth, glConfig.vidHeight) ? GL_FALSE : GL_TRUE);

	if(!mglCreateContext(0, 0, glConfig.vidWidth, glConfig.vidHeight))
	//if(!old_context)
	{
		return qfalse;
	}

	mglGetWindowHandle(); // update buffers;

	//
	if (fullscreen && r_glbuffers->value == 3)
	{
	    	mglEnableSync(GL_FALSE);
		ri.Printf(PRINT_ALL, "triplebuffer enabled\n");
		ri.Printf(PRINT_ALL, "sync disabled\n");
	}

	else
	{
	    	mglEnableSync(GL_TRUE);
	}

	mglLockMode(MGL_LOCK_SMART);

	//GL_SetLockMode(r_lockmode->string);

	/*
	if (r_finish)
		mglEnableSync(r_finish->integer);

	else
		mglEnableSync(GL_TRUE);
	*/

	#if 1
	win = mglGetWindowHandle();

	//ModifyIDCMP((struct Window *)mglGetWindowHandle(), IDCMP_RAWKEY|IDCMP_MOUSEMOVE|IDCMP_MOUSEBUTTONS|IDCMP_DELTAMOVE);
	ModifyIDCMP(win, IDCMP_RAWKEY|IDCMP_MOUSEMOVE|IDCMP_MOUSEBUTTONS|IDCMP_DELTAMOVE);
	ReportMouse(TRUE, win);
	#endif

	//Sys_EventPort = ((struct Window *)mglGetWindowHandle())->UserPort;
	Sys_EventPort = win->UserPort;

	ri.Printf(PRINT_ALL, "... Sys_EventPort at %p\n", Sys_EventPort);
	
	#if 1 // keep this here - Cowcat
	glConfig.colorBits = colorbits;
	glConfig.depthBits = 16;
	glConfig.stencilBits = 0;

	/*
	if (glConfig.hardwareType == GLHW_3DFX_2D3D)
	{
		glConfig.depthBits = 16;
		glConfig.stencilBits = 0;
	}

	else
	{
		glConfig.depthBits = 16;
		glConfig.stencilBits = 8;
	}
	*/

	glConfig.isFullscreen = fullscreen;
	#endif
	
	// test - Cowcat
	if(r_perspective_fast->value)
	{
		glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
		ri.Printf(PRINT_ALL, "GL_Perspective_Correction_hint: fast\n");
	}
	//

	r_closeworkbench = Cvar_Get("r_closeworkbench", "0", CVAR_ARCHIVE);

	if (fullscreen && r_closeworkbench->value)
		mglProposeCloseDesktop(GL_TRUE);

	// clear windowmode - Cowcat

	qglClearColor(0,0,0,1);
	qglClear(GL_COLOR_BUFFER_BIT);
	
	return qtrue;
}
	

static void GLW_Shutdown(void)
{
	//if(old_context == GL_TRUE)
	mglDeleteContext();

	//old_context = GL_FALSE;

	//MGLTerm(); // now in amigaqgl.c/QGL_Shutdown

	Sys_EventPort = 0;
}

static qboolean GLW_LoadOpenGL(const char *name)
{
	qboolean fullscreen;
	
	if (QGL_Init(name))
	{
		fullscreen = r_fullscreen->integer;
		
		if (!GLW_StartDriverAndSetMode(name, r_mode->integer, r_colorbits->integer, fullscreen))
		{
			if (r_mode->integer != 3)
			{
				if (!GLW_StartDriverAndSetMode(name, 3, r_colorbits->integer, fullscreen))
				{
					QGL_Shutdown();
					return qfalse;
				}
			}
		}
		
		return qtrue;
	}

	ri.Printf(PRINT_ALL, "failed\n");

	QGL_Shutdown();
	
	return qfalse;
}

void GLW_StartOpenGL(void)
{
	GLW_LoadOpenGL("minigl.library");
}

static void GLW_InitExtensions( void )
{
	if ( !r_allowExtensions->integer )
	{
		ri.Printf( PRINT_ALL, "*** IGNORING OPENGL EXTENSIONS ***\n" );
		return;
	}

	ri.Printf( PRINT_ALL, "Initializing OpenGL extensions\n" );

	// GL_S3_s3tc
	glConfig.textureCompression = TC_NONE;

	if ( strstr( glConfig.extensions_string, "GL_S3_s3tc" ) )
	{
		if ( r_ext_compressed_textures->integer )
		{
			glConfig.textureCompression = TC_S3TC;
			ri.Printf( PRINT_ALL, "...using GL_S3_s3tc\n" );
		}

		else
		{
			glConfig.textureCompression = TC_NONE;
			ri.Printf( PRINT_ALL, "...ignoring GL_S3_s3tc\n" );
		}
	}

	else
	{
		ri.Printf( PRINT_ALL, "...GL_S3_s3tc not found\n" );
	}

	// GL_EXT_texture_env_add
	glConfig.textureEnvAddAvailable = qfalse;

	if ( strstr( glConfig.extensions_string, "EXT_texture_env_add" ) )
	{
		if ( r_ext_texture_env_add->integer )
		{
			glConfig.textureEnvAddAvailable = qtrue;
			ri.Printf( PRINT_ALL, "...using GL_EXT_texture_env_add\n" );
		}

		else
		{
			glConfig.textureEnvAddAvailable = qfalse;
			ri.Printf( PRINT_ALL, "...ignoring GL_EXT_texture_env_add\n" );
		}
	}

	else
	{
		ri.Printf( PRINT_ALL, "...GL_EXT_texture_env_add not found\n" );
	}

	// GL_ARB_multitexture
	if ( strstr( glConfig.extensions_string, "GL_ARB_multitexture" )  )
	{
		if ( r_ext_multitexture->integer )
		{
			if ( 1 )
			{
				//qglGetIntegerv( GL_MAX_TEXTURE_UNITS_ARB/*GL_MAX_ACTIVE_TEXTURES_ARB*/, &glConfig.maxActiveTextures );
				GLint glint = 0;
				qglGetIntegerv( GL_MAX_TEXTURE_UNITS_ARB, &glint );
				glConfig.numTextureUnits = (int) glint;
				
				if ( glConfig.numTextureUnits > 1 )
				{
					//qglMultiTexCoord2fARB = &glMultiTexCoord2f; // Cowcat
					//qglActiveTextureARB = &glActiveTexture;
					//qglClientActiveTextureARB = glClientActiveTexture;
					
					ri.Printf( PRINT_ALL, "...using GL_ARB_multitexture\n" );
				}

				else
				{
					qglMultiTexCoord2fARB = NULL;
					qglActiveTextureARB = NULL;
					qglClientActiveTextureARB = NULL;

					ri.Printf( PRINT_ALL, "...not using GL_ARB_multitexture, < 2 texture units\n" );
				}
			}
		}

		else
		{
			ri.Printf( PRINT_ALL, "...ignoring GL_ARB_multitexture\n" );
		}
	}

	else
	{
		ri.Printf( PRINT_ALL, "...GL_ARB_multitexture not found\n" );
	}

	// GL_EXT_compiled_vertex_array
	//if ( strstr( glConfig.extensions_string, "GL_EXT_compiled_vertex_array" ) && ( glConfig.hardwareType != GLHW_RIVA128 ) )
	{
		if ( r_ext_compiled_vertex_array->integer )
		{
			//qglLockArraysEXT = &glLockArrays; 
			//qglUnlockArraysEXT = glUnlockArrays;
			//qglLockArraysEXT = &GLLockArrays; 
			//qglUnlockArraysEXT = GLUnlockArrays;

			//LockArrays = TRUE;
			//varrayenabled = TRUE;
 
			ri.Printf( PRINT_ALL, "...using GL_EXT_compiled_vertex_array\n" );
		}

		else
		{
			ri.Printf( PRINT_ALL, "...ignoring GL_EXT_compiled_vertex_array\n" );
		}
	}

	/*
	else
	{
		ri.Printf( PRINT_ALL, "...GL_EXT_compiled_vertex_array not found\n" );
	}
	*/
}

void GLimp_Init(void)
{
	char buf[1024];
	ri.Printf(PRINT_ALL, "Initializing OpenGL subsystem\n");
	
	GLW_StartOpenGL();

	// get our config strings
	Q_strncpyz( glConfig.vendor_string, qglGetString (GL_VENDOR), sizeof( glConfig.vendor_string ) );
	Q_strncpyz( glConfig.renderer_string, qglGetString (GL_RENDERER), sizeof( glConfig.renderer_string ) );
	Q_strncpyz( glConfig.version_string, qglGetString (GL_VERSION), sizeof( glConfig.version_string ) );
	Q_strncpyz( glConfig.extensions_string, qglGetString (GL_EXTENSIONS), sizeof( glConfig.extensions_string ) );

	//
	// chipset specific configuration
	//

	Q_strncpyz( buf, glConfig.renderer_string, sizeof(buf) );
	Q_strlwr( buf );

	glConfig.driverType = GLDRV_ICD;
	glConfig.hardwareType = GLHW_GENERIC;

	if ( strstr( buf, "voodoo avenger" ) )
	{
		glConfig.hardwareType = GLHW_3DFX_2D3D;
	}
	
	if ( strstr( buf, "permedia2" ) )
	{
		glConfig.hardwareType = GLHW_PERMEDIA2;
	}
	
	GLW_InitExtensions();
	GLW_InitGamma();

	#if 1
	if (glConfig.isFullscreen)
	{
		// Clear the mouse pointer in fullscreen mode
		#ifdef __PPC__
		mousePtr = (unsigned short *)AllocVecPPC( 8, MEMF_CHIP|MEMF_CLEAR, 0 );
		#else
		mousePtr = (unsigned short *)AllocVec( 8, MEMF_CHIP|MEMF_CLEAR);
		#endif

		if (mousePtr) 
			SetPointer( win, mousePtr, 0, 0, 0, 0 );
	}
	#endif

}

void GLimp_Shutdown(void)
{
	#if 1
	if (mousePtr)
	{
		ClearPointer(win);

		#ifdef __PPC__
		FreeVecPPC(mousePtr);
		#else
		FreeVec(mousePtr);
		#endif
		
		mousePtr = 0;
	}
	#endif

	GLW_RestoreGamma();
	GLW_Shutdown();
	QGL_Shutdown();
	
	//memset( &glConfig, 0, sizeof( glConfig ) ); // now done in r_init/RE_Shutdown
	//memset( &glState, 0, sizeof( glState ) );
}

#if 0 // Cowcat
void GLimp_LogComment( char *comment ) 
{
	if ( glw_state.log_fp )
	{
		fprintf( glw_state.log_fp, "%s", comment );
	}
}
#endif

void GLimp_EndFrame(void)
{
	if (r_finish->modified)
	{
		mglEnableSync(r_finish->integer);
		r_finish->modified = qfalse;
	}
	
	//mglUnlockDisplay(); // why if we are using SMART lock ? - Cowcat
	mglSwitchDisplay();

	/*
	if (!r_fullscreen->integer && !in_nograb->integer)
	{
		static uint32 lastGrab = 0;
		uint32 now;

		// Capture the pointer in window mode, unless user wants nograb!
		now = Sys_Milliseconds();

		if (now - lastGrab > 1000)
		{
			IIntuition->SetWindowAttrs((struct Window *) mglGetWindowHandle(), WA_GrabFocus, 100, TAG_DONE);
			lastGrab = now;
		}
	}
	*/
}

void install_grabs (void)
{
	//mglGrabFocus(GL_TRUE);
	mouse_active = qfalse;
}

void uninstall_grabs (void)
{
	//mglGrabFocus(GL_FALSE);
	mx = 0;
	my = 0;
	mouse_active = qtrue;
}
