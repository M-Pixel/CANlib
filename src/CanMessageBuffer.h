/*
 * CanMessageBuffer.h
 *
 *  Created on: 20 Sep 2018
 *      Author: David
 */

#ifndef SRC_CAN_CANMESSAGEBUFFER_H_
#define SRC_CAN_CANMESSAGEBUFFER_H_

#include <cstdint>
#include <cstddef>

#include "CanId.h"
#include "CanMessageFormats.h"

// Can message buffer management
class CanMessageBuffer
{
public:
	CanMessageBuffer(CanMessageBuffer *prev) noexcept : next(prev) { }

	static void Init(unsigned int numCanBuffers) noexcept;
	static CanMessageBuffer *Allocate() noexcept;
	static void Free(CanMessageBuffer*& buf) noexcept;
	static unsigned int FreeBuffers() noexcept { return numFree; }
	static unsigned int MinFreeBuffers() noexcept { return minNumFree; }

	// Set up a message buffer to carry a particular message type, setting the priority and code fields.
	// Return a pointer to the message data cast to the requested type.
	CanMessageGeneric *SetupGenericRequestMessage(CanRequestId rid, CanAddress src, CanAddress dest, CanMessageType msgType, unsigned int dataLen) noexcept
	{
		id.SetRequest(msgType, src, dest);
		dataLength = dataLen;
		extId = 1;
		fdMode = 1;
		useBrs = 0;
		remote = 0;
		msg.generic.requestId = rid;
		return &msg.generic;
	}

	// Set up a message buffer to carry a particular message type, setting the dataLength, priority and code fields.
	// Return a pointer to the message data cast to the requested type.
	// Class T must be one of the supported CAN message types.
	template<class T> T* SetupRequestMessage(CanRequestId rid, CanAddress src, CanAddress dest) noexcept
	{
		id.SetRequest(T::messageType, src, dest);
		dataLength = sizeof(T);
		extId = 1;
		fdMode = 1;
		useBrs = 0;
		remote = 0;
		T* rslt = reinterpret_cast<T*>(&msg);
		rslt->SetRequestId(rid);
		return rslt;
	}

	// Set up a message buffer to carry a particular message type, setting the dataLength, priority and code fields.
	// Return a pointer to the message data cast to the requested type.
	// Class T must be one of the supported CAN message types.
	template<class T> T* SetupRequestMessage(CanRequestId rid, CanAddress src, CanAddress dest, CanMessageType msgType) noexcept
	{
		id.SetRequest(msgType, src, dest);
		dataLength = sizeof(T);
		extId = 1;
		fdMode = 1;
		useBrs = 0;
		remote = 0;
		T* rslt = reinterpret_cast<T*>(&msg);
		rslt->SetRequestId(rid);
		return rslt;
	}

	// Set up a message buffer to carry a particular message type, setting the dataLength, priority and code fields.
	// Return a pointer to the message data cast to the requested type.
	// Class T must be one of the supported CAN message types.
	template<class T> T* SetupResponseMessage(CanRequestId rid, CanAddress src, CanAddress dest) noexcept
	{
		id.SetResponse(T::messageType, src, dest);
		dataLength = sizeof(T);
		extId = 1;
		fdMode = 1;
		useBrs = 0;
		remote = 0;
		T* rslt = reinterpret_cast<T*>(&msg);
		rslt->SetRequestId(rid);
		return rslt;
	}

	// Set up a message buffer to carry a particular broadcast message type, setting the dataLength, priority and code fields.
	// Return a pointer to the message data cast to the requested type.
	// Class T must be one of the supported CAN message types.
	template<class T> T* SetupBroadcastMessage(CanAddress src) noexcept
	{
		id.SetBroadcast(T::messageType, src);
		dataLength = sizeof(T);
		extId = 1;
		fdMode = 1;
		useBrs = 0;
		remote = 0;
		return reinterpret_cast<T*>(&msg);
	}

	// Set up a message buffer to carry a particular non-broadcast status message type, setting the dataLength, priority and code fields.
	// Return a pointer to the message data cast to the requested type.
	// Class T must be one of the supported CAN message types.
	template<class T> T* SetupStatusMessage(CanAddress src, CanAddress dest) noexcept
	{
		id.SetRequest(T::messageType, src, dest);
		dataLength = sizeof(T);
		extId = 1;
		fdMode = 1;
		useBrs = 0;
		remote = 0;
		return reinterpret_cast<T*>(&msg);
	}

	void DebugPrint(const char *prefix) noexcept;

	CanMessageBuffer *next;
	CanId id;
	size_t dataLength;
	uint32_t extId : 1,
			fdMode : 1,
			useBrs : 1,
			remote : 1;
	CanMessage msg;

private:
	static CanMessageBuffer * volatile freelist;
	static volatile unsigned int numFree;
	static volatile unsigned int minNumFree;
};

// Helper class to manage CAN message buffer pointers, to ensure they get released if an exception occurs
class CanMessageBufferHandle
{
public:
	CanMessageBufferHandle(CanMessageBuffer *b) : buf(b) { }
	~CanMessageBufferHandle() { if (buf != nullptr) { CanMessageBuffer::Free(buf); } }

	CanMessageBuffer *Access() const { return buf; }
	CanMessageBuffer *HandOver() { CanMessageBuffer *ret = buf; buf = nullptr; return ret; }

private:
	CanMessageBuffer *buf;
};

#endif /* SRC_CAN_CANMESSAGEBUFFER_H_ */
