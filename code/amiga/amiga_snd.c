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

#include "../client/client.h"
#include "../client/snd_local.h"

#ifdef __VBCC__
#pragma amiga-align
#elif defined(WARPUP)
#pragma pack(2)
#endif

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/timer.h>
#include <proto/ahi.h>

#ifdef __PPC__
#if defined(__GNUC__)
#include <powerpc/powerpc_protos.h>
#else
#include <powerpc/powerpc.h>
#include <proto/powerpc.h>
#endif 
#endif

#ifdef __VBCC__
#pragma default-align
#elif defined (WARPUP)
#pragma pack()
#endif

/* Implemented 100% on recycled Quake 2 code */

#include "snd_68k.h"

#if 0 // Cowcat - actually snd_68k.h is this callback function manually converted to hexadecimal from a 68k vbcc compiled object...
ULONG callback(__reg("a0") struct Hook *hook, __reg("a2") struct AHIAudioCtrl *actrl, __reg("a1") struct AHIEffChannelInfo *info)
{
  	hook->h_Data = (APTR)(info->ahieci_Offset[0]);
  	return 0;
}
#endif


#pragma amiga-align

struct ChannelInfo
{
  	struct AHIEffChannelInfo cinfo;
  	ULONG x[1];
};

#pragma default-align

struct Library *AHIBase = NULL;
static struct MsgPort *AHImp = NULL;
static struct AHIRequest *AHIio = NULL;
static BYTE AHIDevice = -1;
static struct AHIAudioCtrl *actrl = NULL;
static ULONG rc = 1;
static struct ChannelInfo info;

cvar_t *sndbits;
cvar_t *sndspeed;
cvar_t *sndchannels;
/*uint32*/ ULONG samplepos;

static int speed;
static UBYTE *dmabuf = NULL;
static int buflen;

#define MINBUFFERSIZE 4*16384

//extern ULONG EffFunc(struct Hook *hook, struct AHIAudioCtrl *actrl, struct AHIEffChannelInfo *info);

struct Hook EffHook = 
{
  	0, 0,
  	(HOOKFUNC)callback,
	//(HOOKFUNC)EffFunc,
  	0, 0,
};

qboolean SNDDMA_Init(void)
{
	struct AHISampleInfo 	sample;
	int 			i;
	ULONG 			mixfreq, playsamples;
	UBYTE 			name[256];
	ULONG 			mode;
	ULONG 			type;
	int 			ahichannels = 2;
	int 			ahibits = 16;

	info.cinfo.ahieci_Channels = 1;
	info.cinfo.ahieci_Func = &EffHook;
	info.cinfo.ahie_Effect = AHIET_CHANNELINFO;
	EffHook.h_Data = 0;

	sndbits = Cvar_Get("sndbits", "16", CVAR_ARCHIVE);
	sndspeed = Cvar_Get("sndspeed", "22050", CVAR_ARCHIVE); // was 441000 - Cowcat
	sndchannels = Cvar_Get("sndchannels", "2", CVAR_ARCHIVE);

	switch ((int)sndspeed->value)
	{
		case 48000:
		case 44100:
		case 22050:
		case 11025:
			speed = sndspeed->value;
			break;

		default:
			speed = 11025;
			break;

	}
	
	if (sndchannels->value == 2) 
		ahichannels = 2;

	else if (sndchannels->value == 1) 
		ahichannels = 1;

	else 
		ahichannels = 2;

	if (sndbits->value == 16) 
		ahibits = 16;

	else if (sndbits->value == 8) 
		ahibits = 8;

	else 
		ahibits = 16;

	Com_Printf("AHI sound initialisation\n");
	Com_Printf("...Frequency: %d Hz\n", speed);
	Com_Printf("...Channels: %d\n", ahichannels);
	Com_Printf("...Bits: %d\n", ahibits);

	if (ahichannels == 1) 
	{
		if (ahibits == 16) 
			type = AHIST_M16S;

		else
			type = AHIST_M8S;
	} 

	else 
	{
		if (ahibits == 16) 
			type = AHIST_S16S;

		else
			type = AHIST_S8S;
	}

	if ((AHImp = CreateMsgPort()) == NULL) 
	{
		Com_Printf("ERROR: Can't create AHI message port\n");
		return qfalse;
	}

	if ((AHIio = (struct AHIRequest *)CreateIORequest(AHImp, sizeof(struct AHIRequest))) == NULL) 
	{
		Com_Printf("ERROR:Can't create AHI io request\n");
		return qfalse;
	}

	AHIio->ahir_Version = 4;

	if ((AHIDevice = OpenDevice("ahi.device", AHI_NO_UNIT, (struct IORequest *)AHIio, 0)) != 0) 
	{
		Com_Printf("ERROR: Can't open ahi.device version 4\n");
		return qfalse;
	}

	AHIBase = (struct Library *)AHIio->ahir_Std.io_Device;

	if ((actrl = AHI_AllocAudio(AHIA_AudioID, AHI_DEFAULT_ID,
					AHIA_MixFreq, 	speed,
					AHIA_Channels, 	1,
					AHIA_Sounds, 	1,
					TAG_END)) == NULL) 
	{
		Com_Printf("ERROR: Can't allocate audio\n");
		return qfalse;
	}

	AHI_GetAudioAttrs(AHI_INVALID_ID, actrl, 
				AHIDB_MaxPlaySamples, 	&playsamples,
				AHIDB_BufferLen, 	256, 
				AHIDB_Name, 		(ULONG)&name,
				AHIDB_AudioID,		&mode, 
				TAG_END);
			
	AHI_ControlAudio(actrl, AHIC_MixFreq_Query, &mixfreq, TAG_END);
			
	buflen = playsamples * speed / mixfreq;

	if (buflen < MINBUFFERSIZE) 
		buflen = MINBUFFERSIZE;

	#ifdef __PPC__
	if ((dmabuf = AllocVecPPC(buflen, MEMF_ANY|MEMF_PUBLIC|MEMF_CLEAR, 0)) == NULL)
	#else
	if ((dmabuf = AllocVec(buflen, MEMF_ANY|MEMF_PUBLIC|MEMF_CLEAR)) == NULL) 
	#endif
	{
		Com_Printf("ERROR: Can't allocate AHI dma buffer\n");
		return qfalse;
	}

	dma.buffer = (unsigned char *)dmabuf;
	dma.channels = ahichannels;
	dma.speed = speed;
	dma.samplebits = ahibits;
	dma.samples = buflen / (dma.samplebits / 8);
	dma.submission_chunk = 1;
	samplepos = 0;

	sample.ahisi_Type = type;
	sample.ahisi_Address = (APTR)dmabuf;
	sample.ahisi_Length = buflen / AHI_SampleFrameSize(type);

	if ((rc = AHI_LoadSound(0, AHIST_DYNAMICSAMPLE, &sample, actrl)) != 0) 
	{
		Com_Printf("ERROR: Can't load sound\n");
		return qfalse;
	}
 
	if (AHI_ControlAudio(actrl, AHIC_Play, TRUE, TAG_END) != 0) 
	{
		Com_Printf("ERROR: Can't start playback\n");
		return qfalse;
	}

	Com_Printf("AHI audio initialized\n");
	Com_Printf("AHI mode: %s (%08x)\n", name, mode);
	Com_Printf("Output: %ibit %s\n", ahibits, ahichannels == 2 ? "stereo" : "mono");
	Com_Printf("AHI snd written by Jarmo Laakkonen and Hans-Joerg Frieden\n");

	AHI_Play(actrl, 
			AHIP_BeginChannel, 	0,
			AHIP_Freq, 		speed,
			AHIP_Vol, 		0x10000,
			AHIP_Pan, 		0x8000,
			AHIP_Sound,		0,
			AHIP_EndChannel, 	NULL,
			TAG_END);

	AHI_SetEffect(&info, actrl);

	return qtrue;
}

int SNDDMA_GetDMAPos(void)
{
	return (samplepos = (int)(EffHook.h_Data) * dma.channels);
}

void SNDDMA_Shutdown(void)
{
	if (actrl) 
	{
		info.cinfo.ahie_Effect = AHIET_CHANNELINFO | AHIET_CANCEL;
		AHI_SetEffect(&info, actrl);
		AHI_ControlAudio(actrl, AHIC_Play, FALSE, TAG_END);
	}

	if (rc == 0 && actrl)
	{
		AHI_UnloadSound(0, actrl);
		rc = 1;
	}

	if (dmabuf) 
	{
		#ifdef __PPC__
		FreeVecPPC(dmabuf);
		#else
		FreeVec(dmabuf);
		#endif
		dmabuf = NULL;
	}

	if (actrl) 
	{
		AHI_FreeAudio(actrl);
		actrl = NULL;
	}

	if (AHIDevice == 0) 
	{
		CloseDevice((struct IORequest *)AHIio);
		AHIDevice = -1;
	}

	if (AHIio) 
	{
		DeleteIORequest((struct IORequest *)AHIio);
		AHIio = NULL;
	}

	if (AHImp) 
	{
		DeleteMsgPort(AHImp);
		AHImp = NULL;
	}
}

void SNDDMA_BeginPainting (void)
{
}

void SNDDMA_Submit(void)
{
}
