#ifndef SIMULATOR_EVENT_H
#define SIMULATOR_EVENT_H

#include "Sim_Defs.h"
#include "Sim_Object.h"
#include <vector>

namespace MQSimEngine
{


	class Sim_Object;
	class Sim_Event
	{
	public:
		Sim_Event(sim_time_type fireTime, Sim_Object* targetObject, void* parameters = NULL, int type = 0)
			: Fire_time(fireTime), Target_sim_object(targetObject), Parameters(parameters), Type(type), Next_event(NULL), Ignore(false)
#if REMOVE_LATENCY_SEND
			, is_subERS(false)
#endif
		{}
		sim_time_type Fire_time;
		Sim_Object* Target_sim_object;
		void* Parameters = NULL;
		int Type;
		Sim_Event* Next_event;//Used to store event list in the MQSim's engine
		bool Ignore;//If true, this event will not be executed
#if NEW_SUBMIT
		int is_subERS;
		bool is_GCWL; //to check if this event is from GC page copy. I want to know how many GC pgm is delayed by prog/resume suspend/resume //doodu
#endif
	};
}

#endif // !SIMULATOR_EVENT_H
