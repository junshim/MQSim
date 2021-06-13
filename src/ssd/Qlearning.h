#ifndef SARSA_H_
#define SARSA_H_

#include "../sim/Sim_Defs.h"
#include "../nvm_chip/flash_memory/Physical_Page_Address.h"
#include "../exec/Device_Parameter_Set.h"
#include "User_Request.h"
#include <type_traits>
#include <random>
#include <map>
#include <tuple>

#define NUM_OF_PREVIOUS_INTERVAL 2
#define NUM_OF_PREVIOUS_ACTION 2
#define NUM_OF_CURRENT_INTERVAL 7
#define NUM_OF_RW_INTENSIVE_RATIO 1
#define NUM_OF_ACTION_SET 4
#define NUM_OF_RQ_SIZE 4

#define MID_OF_RW_RATIO 25

typedef unsigned int Action;
typedef uint64_t Interval;
typedef double Reward;
typedef double QValue;
typedef unsigned int Idx;

class State {
public:
	Interval prev_interval;
	Action prev_action;
	Interval cur_interval;
	unsigned int cur_rq_size;

	State(Interval prev_interval = 0, Action prev_action = 0, Interval cur_interval = 0, unsigned int cur_rq_size = 0);
};
class LatestChoice {
public:
	bool is_wait_reward;
	io_request_id_type id;
	State state;
	Action action;

	LatestChoice(bool is_rewarded = false, io_request_id_type id = "", State state = State(), Action action = 0);
};


class Qlearning {

	double learningRate = 0.8;
	double gamma = 0.9;
	double lamb = 0.95;
	double epsilon = 0.05;
	double warm_up_epsilon = 0.8;
	unsigned int warm_up_cnt = 100;

	double QTable[NUM_OF_PREVIOUS_INTERVAL][NUM_OF_PREVIOUS_ACTION][NUM_OF_CURRENT_INTERVAL][NUM_OF_RQ_SIZE][NUM_OF_ACTION_SET];
	double Eligibility[NUM_OF_PREVIOUS_INTERVAL][NUM_OF_PREVIOUS_ACTION][NUM_OF_CURRENT_INTERVAL][NUM_OF_RQ_SIZE][NUM_OF_ACTION_SET];



	sim_time_type*** cur_interval;
	sim_time_type*** cur_checkpoint;
	io_request_id_type*** cur_id;
	unsigned int*** cur_rq_size;
	std::map<io_request_id_type, io_request_id_type> request_chain;

	LatestChoice*** late_choice_per_plane;

	unsigned int channel_cnt;
	unsigned int chip_cnt;
	unsigned int plane_cnt;

	sim_time_type total_response_time;
	unsigned int response_num;

	const unsigned int action_set[NUM_OF_ACTION_SET] = { 1,2,3,4 };
	const unsigned int rq_set[NUM_OF_RQ_SIZE] = { 2, 10, 20, 40 };
	const unsigned int interval_set[NUM_OF_CURRENT_INTERVAL - 1] = { 30000, 70000, 200000, 450000, 1300000, 2000000 };

	//stat

	unsigned int interval_cnt[NUM_OF_CURRENT_INTERVAL] = { 0, };
	unsigned int action_cnt[NUM_OF_ACTION_SET] = { 0, };
	unsigned int response_cnt[4] = { 0, };
	unsigned int random_cnt[2] = { 0, };
	unsigned int update_cnt = 0;

public:
	Qlearning(Device_Parameter_Set& parameters);
	~Qlearning();

	void Reset();
	void Update_Interval(const NVM::FlashMemory::Physical_Page_Address& plane_address, sim_time_type new_checkpoint, io_request_id_type id);
	Action Get_Action(const NVM::FlashMemory::Physical_Page_Address& plane_address);
	Reward Get_Reward(sim_time_type response_time, unsigned int rq_size);
	void Broadcast_Reward(sim_time_type response_time, io_request_id_type id);
	double Update_Qtable(State s, Action a, Reward r, State s_prime);
	bool Is_Wait_Reward(const NVM::FlashMemory::Physical_Page_Address& plane_address);
	QValue& Get_Qvalue(State s, Action a);
	std::tuple< Idx, Idx, Idx, Idx> Get_Idx(State s, Action a);
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
