#include "Sarsa.h"
#include <iostream>
#include <random>

Sarsa *sarsa_unit;

State::State(Interval prev_interval, Action prev_action, Interval cur_interval)
	: prev_interval(prev_interval), prev_action(prev_action), cur_interval(cur_interval) {};

LatestChoice::LatestChoice(bool is_wait_reward, io_request_id_type id, State state, Action action)
	: is_wait_reward(is_wait_reward), id(id), state(state), action(action) { };

Sarsa::Sarsa(Device_Parameter_Set& parameters) {
	//initial QTable
	for (int i = 0; i < NUM_OF_PREVIOUS_INTERVAL; i++)
		for (int j = 0; j < NUM_OF_PREVIOUS_ACTION; j++)
			for (int k = 0; k < NUM_OF_CURRENT_INTERVAL; k++)
				for (int l = 0; l < NUM_OF_ACTION_SET; l++)
					QTable[i][j][k][l] = 0;

	//init interval
	cur_checkpoint = 0;
	cur_interval = 0;

	cur_id = io_request_id_type();

	//init late_choice_per_plane
	channel_cnt = parameters.Flash_Channel_Count;
	chip_cnt = parameters.Chip_No_Per_Channel;
	die_cnt = parameters.Flash_Parameters.Die_No_Per_Chip;
	plane_cnt = parameters.Flash_Parameters.Plane_No_Per_Die;

	late_choice_per_plane = new LatestChoice*** [channel_cnt];
	for (int i = 0; i < channel_cnt; i++) {
		late_choice_per_plane[i] = new LatestChoice**[chip_cnt];
		for (int j = 0; j < chip_cnt; j++) {
			late_choice_per_plane[i][j] = new LatestChoice*[die_cnt];
			for (int k = 0; k < die_cnt; k++) {
				late_choice_per_plane[i][j][k] = new LatestChoice[plane_cnt];
			}
		}
	}
}

Sarsa::~Sarsa() {

	std::cout << "\n<Update cnt> \n";
	std::cout << "Update: " << update_cnt << std::endl;

	std::cout << "\n<Random cnt> \n";
	std::cout << "Random: " << random_cnt[0] << ", Greedy: " << random_cnt[1] << std::endl;

	std::cout << "\n<Interval cnt> \n";
	for (int i = 0; i < NUM_OF_CURRENT_INTERVAL; i++)
		std::cout << "Interval" << i + 1 << ": " << interval_cnt[i] << std::endl;

	std::cout << "\n<Action cnt> \n";
	for (int i = 0; i < NUM_OF_ACTION_SET; i++)
		std::cout << "aciton" << i + 1 << ": " << action_cnt[i] << std::endl;

	std::cout << "\n<Response cnt> \n";
	for (int i = 0; i < 4; i++)
		std::cout << "response" << i + 1 << ": " << response_cnt[i] << std::endl;

	std::cout << "\n<QTable>\n";
	for (int i = 0; i < NUM_OF_PREVIOUS_INTERVAL; i++)
		for (int j = 0; j <NUM_OF_PREVIOUS_ACTION; j++)
			for (int k = 0; k < NUM_OF_CURRENT_INTERVAL; k++) {
				for (int l = 0; l < NUM_OF_ACTION_SET; l++)
					std::cout << QTable[i][j][k][l] << "\t";
				std::cout << std::endl;
			}


	for (int i = 0; i < channel_cnt; i++) {
		for (int j = 0; j < chip_cnt; j++) {
			for (int k = 0; k < die_cnt; k++) {
				delete[] late_choice_per_plane[i][j][k];
			}
			delete[] late_choice_per_plane[i][j];
		}
		delete[] late_choice_per_plane[i];
	}
	delete[] late_choice_per_plane;
}

void Sarsa::Update_Interval(sim_time_type new_checkpoint, io_request_id_type id) {
	cur_interval = new_checkpoint - cur_checkpoint;
	cur_checkpoint = new_checkpoint;
	request_chain.insert(std::pair<io_request_id_type, io_request_id_type>(id, cur_id));
	cur_id = id;
}

Action Sarsa::Get_Action(const NVM::FlashMemory::Physical_Page_Address& plane_address) {

	LatestChoice sa = late_choice_per_plane[plane_address.ChannelID][plane_address.ChipID][plane_address.DieID][plane_address.PlaneID];

	Q_Idx q_idx = Get_Qtable_idx(sa.state.cur_interval, sa.action, cur_interval);
	
	//select action
	unsigned int max_idx = 0;

	if (Generate_Random<float>(0, 1) < ((update_cnt < warm_up_cnt) ? warm_up_epsilon : epsilon)){
		max_idx = Generate_Random<unsigned int>(0, NUM_OF_ACTION_SET - 1);
		random_cnt[0]++;
	}
	else {
		for (unsigned int idx = 1; idx < NUM_OF_ACTION_SET; idx++)
			if (QTable[q_idx.prev_interval_idx][q_idx.prev_action_idx][q_idx.cur_interval_idx][idx] > 
				QTable[q_idx.prev_interval_idx][q_idx.prev_action_idx][q_idx.cur_interval_idx][max_idx])
				max_idx = idx;
		random_cnt[1]++;
	}
	late_choice_per_plane[plane_address.ChannelID][plane_address.ChipID][plane_address.DieID][plane_address.PlaneID] = LatestChoice(true, cur_id, State(sa.state.cur_interval, sa.action, cur_interval), max_idx);

	action_cnt[max_idx]++;
	return action_set[max_idx];

}

Reward Sarsa::Get_Reward(sim_time_type response_time) {
	if (response_time < 6000) {
		response_cnt[0]++;
		return 1.0;
	}
	else if (response_time < 25000) {
		response_cnt[1]++;
		return 0.5;
	}
	else if (response_time < 60000) {
		response_cnt[2]++;
		return 0.2;
	}
	else {
		response_cnt[3]++;
		return -0.5;
	}
}

void Sarsa::Broadcast_Reward(sim_time_type response_time, io_request_id_type id) {
	io_request_id_type past_id = request_chain[id];
	
	for (int i = 0; i < channel_cnt; i++) {
		for (int j = 0; j < chip_cnt; j++) {
			for (int k = 0; k < die_cnt; k++) {
				for (int l = 0; l < plane_cnt; l++) {
					if (late_choice_per_plane[i][j][k][l].is_wait_reward == true && late_choice_per_plane[i][j][k][l].id == past_id) {
						LatestChoice& tmp = late_choice_per_plane[i][j][k][l];
						Reward r = Get_Reward(response_time);
						Update_Qtable(tmp.state, tmp.action, r);
						tmp.is_wait_reward = false;
					}
				}
			}
		}
	}
	request_chain.erase(id);
}


void Sarsa::Update_Qtable(State s, Action a, Reward r) {

	update_cnt++;
	Q_Idx prev_idx = Get_Qtable_idx(s.prev_interval, s.prev_action, s.cur_interval);
	Q_Idx cur_idx = Get_Qtable_idx(s.cur_interval, a, cur_interval);

	//std::cout << "Update!\n";
	//std::cout << "prev_inter" << s.prev_interval << "cur_inter" << s.cur_interval << std::endl;
	//std::cout << "prev_inter_idx: " << prev_idx.prev_interval_idx << ", prev_action_idx: " << prev_idx.prev_action_idx << ", cur_inter_idx: " << prev_idx.cur_interval_idx << std::endl;
	int max_idx = 0;
	for (int idx = 1; idx < NUM_OF_ACTION_SET; idx++)
		if (QTable[cur_idx.prev_interval_idx][cur_idx.prev_action_idx][cur_idx.cur_interval_idx][idx] > 
			QTable[cur_idx.prev_interval_idx][cur_idx.prev_action_idx][cur_idx.cur_interval_idx][max_idx])
			max_idx = idx;

	QTable[prev_idx.prev_interval_idx][prev_idx.prev_action_idx][prev_idx.cur_interval_idx][a] += learningRate * 
		(r + gamma * QTable[cur_idx.prev_interval_idx][cur_idx.prev_action_idx][cur_idx.cur_interval_idx][max_idx] - 
			QTable[prev_idx.prev_interval_idx][prev_idx.prev_action_idx][prev_idx.cur_interval_idx][a]);
}

Q_Idx Sarsa::Get_Qtable_idx(Interval prev_interval, Action prev_action, Interval cur_interval) {
	Q_Idx res;
	
	//select prev_interval
	if (prev_interval < interval_set[(NUM_OF_CURRENT_INTERVAL/2)])
		res.prev_interval_idx = 0;
	else
		res.prev_interval_idx = 1;

	//select prev_action
	if (prev_action < NUM_OF_ACTION_SET/2)
		res.prev_action_idx = 0;
	else
		res.prev_action_idx = 1;

	//select cur_interval
	res.cur_interval_idx = NUM_OF_CURRENT_INTERVAL - 1;
	for (unsigned int idx = 0; idx < NUM_OF_CURRENT_INTERVAL - 1; idx++) {
		if (cur_interval < interval_set[idx]) {
			res.cur_interval_idx = idx;
			break;
		}
	}
	interval_cnt[res.cur_interval_idx]++;
	return res;
}

bool Sarsa::Is_Wait_Reward(const NVM::FlashMemory::Physical_Page_Address& plane_address) {
	return late_choice_per_plane[plane_address.ChannelID][plane_address.ChipID][plane_address.DieID][plane_address.PlaneID].is_wait_reward;
}