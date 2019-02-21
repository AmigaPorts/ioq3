
//#include "../renderer/tr_local.h" // This messes up DoIO() warpos gcc. Go figure...
#include "../client/client.h"
#include "amiga_local.h"

#ifdef __VBCC__
#pragma amiga-align
#elif defined(WARPUP)
#pragma pack(push,2)
#endif

#include <proto/exec.h>
#include <proto/dos.h>
#include <exec/interrupts.h>
#include <devices/input.h>
#include <devices/inputevent.h>
#include <clib/alib_protos.h>

#ifdef __PPC__
#if defined(__GNUC__)
#include <powerpc/powerpc.h>
#include <powerpc/powerpc_protos.h>
#else
#include <powerpc/powerpc.h>
#include <proto/powerpc.h>
#endif
#endif

#ifdef __VBCC__
#pragma default-align
#elif defined (WARPUP)
#pragma pack(pop)
#endif

extern qboolean mouse_avail;

// Cowcat windowmode MouseHandler stuff 

static struct MsgPort	*InputPort = NULL;
static struct IOStdReq	*InputIO = NULL;
static struct Interrupt InputHandler;

qboolean mhandler;

// Magic Cowcat 68k sauce: Just left mouse button changed to ctrl key.
unsigned short InputCode[]= {
	0x48e7, 0x0020, 0x2208, 0x45e8,                    
	0x0004, 0x0c12, 0x0002, 0x6626,                    
	0x43e8, 0x0006, 0x3011, 0x907c,
	0x0068, 0x6708, 0x907c, 0x0080,
	0x670c, 0x6012, 0x14bc, 0x0001,
	0x32bc, 0x0063, 0x6008, 0x14bc,
	0x0001, 0x32bc, 0x00e3, 0x2050,
	0x4a88, 0x66ca, 0x2001, 0x245f,
	0x4e75                    
};

/* actually this:

#include <proto/exec.h>
#include <proto/intuition.h>
#include <devices/inputevent.h>

struct InputEvent *InputCode (__reg("a0") struct InputEvent *ielist)
{
	struct InputEvent *ie;
	ie = ielist;

	do
	{
		if (ie->ie_Class == IECLASS_RAWMOUSE) 		
		{
                	switch(ie->ie_Code)
			{
				case IECODE_LBUTTON: 	
		 			ie->ie_Class = IECLASS_RAWKEY;
		 			ie->ie_Code  = 0x63;
		 			break;

				case IECODE_LBUTTON|IECODE_UP_PREFIX: 	
		 			ie->ie_Class = IECLASS_RAWKEY;
		 			ie->ie_Code  = 0x63|IECODE_UP_PREFIX;
		 			break;
			}
		}
	
		ie = ie->ie_NextEvent;

   	} while (ie);	
	
	return(ielist);
}
*/

void MouseHandler (void)
{
	if ( mouse_avail && !mhandler )
	{
		//Com_Printf("mousehandler\n");

		if ( InputPort = CreateMsgPort() )
		{
			if ( InputIO = (struct IOStdReq *) CreateIORequest(InputPort, sizeof(struct IOStdReq)) )
			{
				if(!OpenDevice( "input.device", 0, (struct IORequest *)InputIO, 0 ))
				{
					InputHandler.is_Node.ln_Type = NT_INTERRUPT;
					InputHandler.is_Node.ln_Pri = 90;
					InputHandler.is_Code = (APTR)InputCode;

					InputIO->io_Data = (void *)&InputHandler;
					InputIO->io_Command = IND_ADDHANDLER;

					DoIO((struct IORequest *)InputIO);

					mhandler = qtrue;

					return;
				}
			}
		}
	}
}

void MouseHandlerOff (void)
{	 
	if ( mouse_avail && mhandler )
	{
		//Com_Printf("mousehandleroff\n");

		InputIO->io_Data=(void *)&InputHandler;
		InputIO->io_Command = IND_REMHANDLER;
		DoIO( (struct IORequest *)InputIO );

		CloseDevice( (struct IORequest *)InputIO );		
		DeleteIORequest((struct IORequest *)InputIO);
		DeleteMsgPort(InputPort);
	
		mhandler = qfalse; 
	}
}

