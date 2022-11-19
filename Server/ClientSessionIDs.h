#pragma once

#ifndef CLIENTSESSIONIDS_HPP
#define CLIENTSESSIONIDS_HPP


#include <iostream>
#include <thread>

struct ClientSessionIDs
{
	uint8_t ClientSession_ID;
	uint32_t ClientSession_threadID;
};

#endif // CLIENTSESSIONIDS_HPP