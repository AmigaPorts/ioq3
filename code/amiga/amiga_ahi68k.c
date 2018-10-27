
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/ahi.h>

ULONG EffFunc(__reg("a0") struct Hook *hook, __reg("a2") struct AHIAudioCtrl *actrl, __reg("a1") struct AHIEffChannelInfo *info)
{
  	hook->h_Data = (APTR)(info->ahieci_Offset[0]);
  	return 0;
}