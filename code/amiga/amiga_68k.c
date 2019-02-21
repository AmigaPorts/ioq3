
#include <intuition/intuition.h>
#include <proto/exec.h>
#include <proto/keymap.h>

struct MsgStruct
{
	ULONG Class;
	UWORD Code;
	UWORD Qualifier;
	WORD MouseX;
	WORD MouseY;
	int rawkey;
};

int GetMessages68k( __reg("a1") struct MsgPort *port, __reg("a0") struct MsgStruct *msg, __reg("d0") int maxmsg )
{
  	int i = 0;
  	struct IntuiMessage *imsg;
	struct InputEvent ie;

	UBYTE buf[4];
	WORD result;

  	while ((imsg = (struct IntuiMessage *)GetMsg(port)))
	{
    		if (i < maxmsg)
		{
      			msg[i].Code = imsg->Code;
      			msg[i].Class = imsg->Class;
			msg[i].Qualifier = imsg->Qualifier;
			msg[i].MouseX = imsg->MouseX;
			msg[i].MouseY = imsg->MouseY;

			if(msg[i].Class == IDCMP_RAWKEY)
			{
				ie.ie_Class = IECLASS_RAWKEY;
				ie.ie_SubClass = 0;
				ie.ie_Code = imsg->Code;
				ie.ie_Qualifier = imsg->Qualifier;
				ie.ie_EventAddress = 0;

				result = MapRawKey(&ie, buf, 4, 0);
				
				if (result != 1 )
					msg[i].rawkey = -1;

				else
					msg[i].rawkey = buf[0];
			}

      			i++;
    		}

    		ReplyMsg((struct Message *)imsg);
  	}

  	return i;
}
