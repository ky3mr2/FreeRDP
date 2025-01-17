/**
 * FreeRDP: A Remote Desktop Protocol Implementation
 * Client Channels
 *
 * Copyright 2014 Marc-Andre Moreau <marcandre.moreau@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FREERDP_LIB_CORE_CLIENT_H
#define FREERDP_LIB_CORE_CLIENT_H

#include <winpr/crt.h>
#include <winpr/stream.h>
#include <winpr/wtsapi.h>

#include <freerdp/freerdp.h>
#include <freerdp/constants.h>

#include <freerdp/svc.h>
#include <freerdp/peer.h>
#include <freerdp/addin.h>
#include <freerdp/api.h>

#include <freerdp/client/channels.h>
#include <freerdp/client/drdynvc.h>
#include <freerdp/channels/channels.h>

#ifndef CHANNEL_MAX_COUNT
#define CHANNEL_MAX_COUNT 30
#endif

typedef struct
{
	PVIRTUALCHANNELENTRY entry;
	PVIRTUALCHANNELENTRYEX entryEx;
	PCHANNEL_INIT_EVENT_FN pChannelInitEventProc;
	PCHANNEL_INIT_EVENT_EX_FN pChannelInitEventProcEx;
	void* pInitHandle;
	void* lpUserParam;
} CHANNEL_CLIENT_DATA;

typedef struct
{
	char name[CHANNEL_NAME_LEN + 1];
	uint32_t OpenHandle;
	ULONG options;
	int flags;
	void* pInterface;
	rdpChannels* channels;
	void* lpUserParam;
	PCHANNEL_OPEN_EVENT_FN pChannelOpenEventProc;
	PCHANNEL_OPEN_EVENT_EX_FN pChannelOpenEventProcEx;
} CHANNEL_OPEN_DATA;

typedef struct
{
	void* Data;
	UINT32 DataLength;
	void* UserData;
	CHANNEL_OPEN_DATA* pChannelOpenData;
} CHANNEL_OPEN_EVENT;

/**
 * pInitHandle: handle that identifies the client connection
 * Obtained by the client with VirtualChannelInit
 * Used by the client with VirtualChannelOpen
 */

typedef struct
{
	rdpChannels* channels;
	void* pInterface;
} CHANNEL_INIT_DATA;

struct rdp_channels
{
	int clientDataCount;
	CHANNEL_CLIENT_DATA clientDataList[CHANNEL_MAX_COUNT];

	int openDataCount;
	CHANNEL_OPEN_DATA openDataList[CHANNEL_MAX_COUNT];

	int initDataCount;
	CHANNEL_INIT_DATA initDataList[CHANNEL_MAX_COUNT];

	/* control for entry into MyVirtualChannelInit */
	int can_call_init;

	/* true once freerdp_channels_post_connect is called */
	BOOL connected;

	/* used for locating the channels for a given instance */
	freerdp* instance;

	wMessageQueue* queue;

	DrdynvcClientContext* drdynvc;
	CRITICAL_SECTION channelsLock;

	wHashTable* channelEvents;
};

FREERDP_LOCAL void freerdp_channels_free(rdpChannels* channels);

WINPR_ATTR_MALLOC(freerdp_channels_free, 1)
FREERDP_LOCAL rdpChannels* freerdp_channels_new(freerdp* instance);

FREERDP_LOCAL UINT freerdp_channels_disconnect(rdpChannels* channels, freerdp* instance);
FREERDP_LOCAL void freerdp_channels_close(rdpChannels* channels, freerdp* instance);

FREERDP_LOCAL void freerdp_channels_register_instance(rdpChannels* channels, freerdp* instance);
FREERDP_LOCAL UINT freerdp_channels_pre_connect(rdpChannels* channels, freerdp* instance);
FREERDP_LOCAL UINT freerdp_channels_post_connect(rdpChannels* channels, freerdp* instance);

/** @since version 3.9.0 */
FREERDP_LOCAL SSIZE_T freerdp_client_channel_get_registered_event_handles(rdpChannels* channels,
                                                                          HANDLE* events,
                                                                          DWORD count);

#endif /* FREERDP_LIB_CORE_CLIENT_H */
