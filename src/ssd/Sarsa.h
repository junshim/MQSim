#ifndef SARSA_H_
#define SARSA_H_

#include "../sim/Sim_Defs.h"
#include "../nvm_chip/flash_memory/Physical_Page_Address.h"
#include "../exec/Device_Parameter_Set.h"
#include <type_traits>
#include <random>
#include <map>

#define NUM_OF_PREVIOUS_INTERVAL 2
#define NUM_OF_PREVIOUS_ACTION 2
#define NUM_OF_CURRENT_INTERVAL 17
#define NUM_OF_ACTION_SET 4

typedef unsigned int Action;
typedef unsigned int Interval;
typedef double Reward;

class Q_Idx {
public:
	unsigned int prev_interval_idx;
	unsigned int prev_action_idx;
	unsigned int cur_interval_idx;
};

class State {
public:
	Interval prev_interval;
	Action prev_action;
	Interval cur_interval;
	State(Interval prev_interval=0, Action prev_action=0, Interval cur_interval=0);
};
class LatestChoice {
public:
	bool is_wait_reward;
	io_request_id_type id;
	State state;
	Action action;

	LatestChoice(bool is_rewarded = false, io_request_id_type id = "", State state = State(), Action action = 0);
};


class Sarsa {

	double learningRate = 0.8;
	double gamma = 0.9;
	double epsilon = 0.05;
	double warm_up_epsilon = 0.8;
	unsigned int warm_up_cnt = 150;

	double QTable[NUM_OF_PREVIOUS_INTERVAL][NUM_OF_PREVIOUS_ACTION][NUM_OF_CURRENT_INTERVAL][NUM_OF_ACTION_SET];

	sim_time_type cur_interval;
	sim_time_type cur_checkpoint;
	io_request_id_type cur_id;
	std::map<io_request_id_type, io_request_id_type> request_chain;

	LatestChoice**** late_choice_per_plane;

	unsigned int channel_cnt;
	unsigned int chip_cnt;
	unsigned int die_cnt;
	unsigned int plane_cnt;


	const unsigned int action_set[NUM_OF_ACTION_SET] = { 1,2,3,4 };
	const unsigned int interval_set[NUM_OF_CURRENT_INTERVAL - 1] = { 300, 1500, 3300, 4900, 7500, 10000, 16000, 23000, 31000, 39000, 48000, 59000, 70000, 90000, 120000, 150000 };

	//stat

	unsigned int interval_cnt[NUM_OF_CURRENT_INTERVAL] = { 0, };
	unsigned int action_cnt[NUM_OF_ACTION_SET] = { 0, };
	unsigned int response_cnt[4] = { 0, };
	unsigned int random_cnt[2] = { 0, };
	unsigned int update_cnt = 0;

public:
	Sarsa(Device_Parameter_Set& parameters);
	~Sarsa();

	void Init_late_choice_per_plane();
	void Update_Interval(sim_time_type new_checkpoint, io_request_id_type id);
	Action Get_Action(const NVM::FlashMemory::Physical_Page_Address& plane_address);
	Reward Get_Reward(sim_time_type response_time);
	void Broadcast_Reward(sim_time_type response_time, io_request_id_type id);
	void Update_Qtable(State s, Action a, Reward r);
	Q_Idx Get_Qtable_idx(Interval prev_interval, Action action, Interval cur_interval);
	bool Is_Wait_Reward(const NVM::FlashMemory::Physical_Page_Address& plane_address);

};

template<typename T>
T Generate_Random(T l, T r) {
	std::random_device rd;
	std::mt19937 gen(rd());

	if (std::is_same_v<T, unsigned int> || std::is_same_v<T, int>) {
		std::uniform_int_distribution<int> dis(l, r);
		return dis(gen);
	}
	else {
		std::uniform_real_distribution<float> dis(l, r);
		return dis(gen);
	}
}

#endif
