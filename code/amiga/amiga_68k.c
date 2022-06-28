
#include <intuition/intuition.h>
#include <proto/exec.h>
#include <proto/keymap.h>

struct MsgStruct
{
	ULONG Class;
	UWORD Code;
	WORD MouseX;
	WORD MouseY;
	WORD rawkey;
};


int GetMessages68k( __reg("a1") struct MsgPort *port, __reg("a0") struct MsgStruct *msg, __reg("d1") int decodechar )
{
	int i = 0;
	struct IntuiMessage *imsg;
	struct InputEvent ie;
	struct ExecBase *SysBase;

	#define BUFFERLEN 4

	UBYTE buf[BUFFERLEN];
	UWORD result;

	SysBase = *(struct ExecBase **)4L;

	while ((imsg = (struct IntuiMessage *)GetMsg(port)))
	{
		if (i < 50) // maxmsg
		{
			msg[i].Class = imsg->Class;
			msg[i].Code = imsg->Code;
			msg[i].MouseX = imsg->MouseX;
			msg[i].MouseY = imsg->MouseY;
			msg[i].rawkey = 0;

			if( decodechar && msg[i].Class == IDCMP_RAWKEY ) //&& (msg[i].Code & ~IECODE_UP_PREFIX) )
			{
				ie.ie_Class = IECLASS_RAWKEY;
				ie.ie_SubClass = 0;
				ie.ie_Code = msg[i].Code;
				ie.ie_Qualifier = imsg->Qualifier;
				ie.ie_EventAddress = NULL;

				result = MapRawKey(&ie, buf, BUFFERLEN, 0);

				#if 0

				if (result != 1 )
					msg[i].rawkey = 0;

				else
					msg[i].rawkey = buf[0];

				#else

				if ( result == 1 )
					msg[i].rawkey = buf[0];

				#endif
			}
			
			i++;
		}

		ReplyMsg((struct Message *)imsg);
	}

	return i;
}



