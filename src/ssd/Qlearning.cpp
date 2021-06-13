#include "Qlearning.h"
#include <iostream>
#include <random>

Qlearning *qlearning_unit;

State::State(Interval prev_interval, Action prev_action, Interval cur_interval, unsigned int cur_rq_size)
	: prev_interval(prev_interval), prev_action(prev_action), cur_interval(cur_interval), cur_rq_size(cur_rq_size) {};

LatestChoice::LatestChoice(bool is_wait_reward, io_request_id_type id, State state, Action action)
	: is_wait_reward(is_wait_reward), id(id), state(state), action(action) { };

Qlearning::Qlearning(Device_Parameter_Set& parameters) {
	//initial QTable
	for (int i = 0; i < NUM_OF_PREVIOUS_INTERVAL; i++)
		for (int j = 0; j < NUM_OF_PREVIOUS_ACTION; j++)
			for (int k = 0; k < NUM_OF_CURRENT_INTERVAL; k++)
				for (int m = 0; m < NUM_OF_RQ_SIZE; m++) 
					for (int n = 0; n < NUM_OF_ACTION_SET; n++) {
						QTable[i][j][k][m][n] = Generate_Random<float>(-1.0, 1.0);
						Eligibility[i][j][k][m][n] = 0;
					}

	//init interval
	total_response_time = 0;
	response_num = 0;

	//init late_choice_per_plane
	channel_cnt = parameters.Flash_Channel_Count;
	chip_cnt = parameters.Chip_No_Per_Channel;
	plane_cnt = parameters.Flash_Parameters.Plane_No_Per_Die;

	late_choice_per_plane = new LatestChoice** [channel_cnt];
	cur_checkpoint = new sim_time_type **[channel_cnt];
	cur_interval = new sim_time_type **[channel_cnt];
	cur_id = new io_request_id_type **[channel_cnt];
	cur_rq_size = new unsigned int** [channel_cnt];
	for (unsigned int i = 0; i < channel_cnt; i++) {
		late_choice_per_plane[i] = new LatestChoice*[chip_cnt];
		cur_checkpoint[i] = new sim_time_type *[chip_cnt];
		cur_interval[i] = new sim_time_type *[chip_cnt];
		cur_id[i] = new io_request_id_type *[chip_cnt];
		cur_rq_size[i] = new unsigned int* [chip_cnt];
		for (unsigned  j = 0; j < chip_cnt; j++) {
			late_choice_per_plane[i][j] = new LatestChoice[plane_cnt];
			cur_checkpoint[i][j] = new sim_time_type[plane_cnt];
			cur_interval[i][j] = new sim_time_type[plane_cnt];
			cur_id[i][j] = new io_request_id_type[plane_cnt];
			cur_rq_size[i][j] = new unsigned int[plane_cnt];
			for (unsigned  k = 0; k < plane_cnt; k++) {
				late_choice_per_plane[i][j][k].is_wait_reward = false;
				cur_checkpoint[i][j][k] = 0;
				cur_interval[i][j][k] = 0;
				cur_id[i][j][k] = io_request_id_type();
				cur_rq_size[i][j][k] = 0;
				
			}
		}
	}
}

void Qlearning::Reset() {
	for (unsigned int i = 0; i < channel_cnt; i++)
		for (unsigned int j = 0; j < chip_cnt; j++)
			for (unsigned int k = 0; k < plane_cnt; k++) {
				late_choice_per_plane[i][j][k].is_wait_reward = false;
				cur_checkpoint[i][j][k] = 0;
				cur_interval[i][j][k] = 0;
				cur_id[i][j][k] = io_request_id_type();
				cur_rq_size[i][j][k] = 0;
			}
}

Qlearning::~Qlearning() {

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
		for (int j = 0; j < NUM_OF_PREVIOUS_ACTION; j++)
			for (int k = 0; k < NUM_OF_CURRENT_INTERVAL; k++)
				for (int m = 0; m < NUM_OF_RQ_SIZE; m++) {
					for (int n = 0; n < NUM_OF_ACTION_SET; n++) 
						std::cout << QTable[i][j][k][m][n] << "\t";
					std::cout << std::endl;
				}


	for (unsigned int i = 0; i < channel_cnt; i++) {
		for (unsigned int j = 0; j < chip_cnt; j++) {
			delete[] late_choice_per_plane[i][j];
			delete[] cur_checkpoint[i][j];
			delete[] cur_interval[i][j];
			delete[] cur_id[i][j];
			delete[] cur_rq_size[i][j];
		}
		delete[] late_choice_per_plane[i];
		delete[] cur_checkpoint[i];
		delete[] cur_interval[i];
		delete[] cur_id[i];
		delete[] cur_rq_size[i];
	}
	delete[] late_choice_per_plane;
	delete[] cur_checkpoint;
	delete[] cur_interval;
	delete[] cur_id;
	delete[] cur_rq_size;
}

void Qlearning::Update_Interval(const NVM::FlashMemory::Physical_Page_Address& plane_address, sim_time_type new_checkpoint, io_request_id_type id) {
	flash_channel_ID_type ChannelID = plane_address.ChannelID;
	flash_chip_ID_type ChipID = plane_address.ChipID;     
	flash_plane_ID_type PlaneID = plane_address.PlaneID;

	if (id != cur_id[ChannelID][ChipID][PlaneID]) {
		cur_interval[ChannelID][ChipID][PlaneID] = new_checkpoint - cur_checkpoint[ChannelID][ChipID][PlaneID];
		cur_checkpoint[ChannelID][ChipID][PlaneID] = new_checkpoint;
		request_chain.insert(std::pair<io_request_id_type, io_request_id_type>(id, cur_id[ChannelID][ChipID][PlaneID]));
		cur_id[ChannelID][ChipID][PlaneID] = id;
		cur_rq_size[ChannelID][ChipID][PlaneID] = 1;
	}
	else
		cur_rq_size[ChannelID][ChipID][PlaneID]++;
}

Action Qlearning::Get_Action(const NVM::FlashMemory::Physical_Page_Address& plane_address) {
	flash_channel_ID_type ChannelID = plane_address.ChannelID;
	flash_chip_ID_type ChipID = plane_address.ChipID;
	flash_die_ID_type DieID = plane_address.DieID;
	flash_plane_ID_type PlaneID = plane_address.PlaneID;

	LatestChoice sa = late_choice_per_plane[ChannelID][ChipID][PlaneID];
	State cur_state(sa.state.cur_interval, sa.action, cur_interval[ChannelID][ChipID][PlaneID], cur_rq_size[ChannelID][ChipID][PlaneID]);

	//select action
	unsigned int max_action_idx = 0;

	if (Generate_Random<float>(0, 1) < ((update_cnt < warm_up_cnt) ? warm_up_epsilon : epsilon)){
		max_action_idx = Generate_Random<unsigned int>(0, NUM_OF_ACTION_SET - 1);
		random_cnt[0]++;
	}
	else {
		QValue max_Qvalue = Get_Qvalue(cur_state, max_action_idx);
		for (unsigned int action_idx = 1; action_idx < NUM_OF_ACTION_SET; action_idx++) {
			QValue tmp_Qvalue = Get_Qvalue(cur_state, action_idx);
			if (tmp_Qvalue > max_Qvalue) {
				max_action_idx = action_idx;
				max_Qvalue = tmp_Qvalue;
			}
		}
		random_cnt[1]++;
	}
	late_choice_per_plane[ChannelID][ChipID][PlaneID] = LatestChoice(true, cur_id[ChannelID][ChipID][PlaneID], cur_state, max_action_idx);

	action_cnt[max_action_idx]++;
	return action_set[max_action_idx];

}

Reward Qlearning::Get_Reward(sim_time_type response_time, unsigned int rq_size) {
	total_response_time += response_time;
	response_num++;

	long double base = (long double)total_response_time / (long double)response_num;
	unsigned int bonus;
	if (rq_size > rq_set[3])
		bonus = 0.5;
	else if (rq_size > rq_set[2])
		bonus = 0.3;
	else if (rq_size > rq_set[1])
		bonus = 0.1;
	else
		bonus = 0;

	if (response_time < (0.7 + bonus) * base) {
		response_cnt[0]++;
		return 1.0 + 0.3 * bonus;
	}
	else if (response_time < (0.9 + bonus) * base) {
		response_cnt[1]++;
		return 0.5 + 0.3 * bonus;
	}
	else if (response_time < (1 + bonus) * base) {
		response_cnt[2]++;
		return 0.2 + 0.3 * bonus;
	}
	else {
		response_cnt[3]++;
		return -0.5 + 0.3 * bonus;
	}
}

void Qlearning::Broadcast_Reward(sim_time_type response_time, io_request_id_type id) {
	io_request_id_type past_id = request_chain[id];
	double total_gradient = 0;
	int cnt = 0;
	for (unsigned int i = 0; i < channel_cnt; i++) {
		for (unsigned int j = 0; j < chip_cnt; j++) {
			for (unsigned int k = 0; k < plane_cnt; k++) {
				if (late_choice_per_plane[i][j][k].is_wait_reward == true && late_choice_per_plane[i][j][k].id == past_id) {
					LatestChoice& tmp = late_choice_per_plane[i][j][k];
					Reward r = Get_Reward(response_time, tmp.state.cur_rq_size);
					total_gradient += Update_Qtable(tmp.state, tmp.action, r, State(tmp.state.cur_interval, tmp.action, cur_interval[i][j][k], cur_rq_size[i][j][k]));
					tmp.is_wait_reward = false;
					cnt++;
				}
			}
		}
	}
	if (cnt > 0) {
		for (int i = 0; i < NUM_OF_PREVIOUS_INTERVAL; i++) 
			for (int j = 0 ; j < NUM_OF_PREVIOUS_ACTION; j++)
				for (int k = 0; k < NUM_OF_CURRENT_INTERVAL; k++)
					for (int m = 0; m < NUM_OF_RQ_SIZE; m++) {
						double Q_max = 0;
						int max_idx = 0;
						for (int n = 0; n < NUM_OF_ACTION_SET; n++)
						{
							double Q_tmp = QTable[i][j][k][m][n];
							QTable[i][j][k][m][n] += learningRate * (total_gradient / cnt) * Eligibility[i][j][k][m][n];
							
							if (Q_max > Q_tmp)
								Eligibility[i][j][k][m][n] = 0;
							else {
								Eligibility[i][j][k][m][max_idx] = 0;
								Q_max = Q_tmp;
								max_idx = n;
							}
						}
						Eligibility[i][j][k][m][max_idx] *= lamb * gamma;
					}
	}

	request_chain.erase(id);
}


double Qlearning::Update_Qtable(State s, Action a, Reward r, State s_prime) {

	update_cnt++;

	//std::cout << "Update!\n";
	//std::cout << "prev_inter" << s.prev_interval << "cur_inter" << s.cur_interval << std::endl;
	//std::cout << "prev_inter_idx: " << prev_idx.prev_interval_idx << ", prev_action_idx: " << prev_idx.prev_action_idx << ", cur_inter_idx: " << prev_idx.cur_interval_idx << std::endl;
	int max_action_idx = 0;
	QValue cur_Q = Get_Qvalue(s_prime, 0);

	for (int action_idx = 1; action_idx < NUM_OF_ACTION_SET; action_idx++) {
		QValue tmp_Q = Get_Qvalue(s_prime, action_idx);
		if (tmp_Q > cur_Q) {
			max_action_idx = action_idx;
			cur_Q = tmp_Q;
		}
	}
	QValue& prev_Q = Get_Qvalue(s, a);

	Idx prev_interval_idx, prev_action_idx, cur_interval_idx, cur_rq_size_idx;

	std::tie(prev_interval_idx, prev_action_idx, cur_interval_idx,cur_rq_size_idx) = Get_Idx(s, a);

	Eligibility[prev_interval_idx][prev_action_idx][cur_interval_idx][cur_rq_size_idx][a]++;

	return (r + gamma * cur_Q - prev_Q);
}

bool Qlearning::Is_Wait_Reward(const NVM::FlashMemory::Physical_Page_Address& plane_address) {
	return late_choice_per_plane[plane_address.ChannelID][plane_address.ChipID][plane_address.PlaneID].is_wait_reward;
}

QValue& Qlearning::Get_Qvalue(State s, Action a) {
	Idx prev_interval_idx, prev_action_idx, cur_interval_idx, cur_rq_size_idx;

	std::tie(prev_interval_idx, prev_action_idx, cur_interval_idx, cur_rq_size_idx) = Get_Idx(s, a);

	interval_cnt[cur_interval_idx]++;

	return QTable[prev_interval_idx][prev_action_idx][cur_interval_idx][cur_rq_size_idx][a];
}

std::tuple< Idx, Idx, Idx, Idx> Qlearning::Get_Idx(State s, Action a) {
	Idx prev_interval_idx, prev_action_idx, cur_interval_idx, cur_rq_size_idx;

	//select prev_interval
	if (s.prev_interval < interval_set[(NUM_OF_CURRENT_INTERVAL / 2)])
		prev_interval_idx = 0;
	else
		prev_interval_idx = 1;

	//select prev_action
	if (s.prev_action < NUM_OF_ACTION_SET / 2)
		prev_action_idx = 0;
	else
		prev_action_idx = 1;

	//select cur_interval
	cur_interval_idx = NUM_OF_CURRENT_INTERVAL - 1;
	for (unsigned int idx = 0; idx < NUM_OF_CURRENT_INTERVAL - 1; idx++) {
		if (s.cur_interval < interval_set[idx]) {
			cur_interval_idx = idx;
			break;
		}
	}

	//select cur_rq_size
	cur_rq_size_idx = NUM_OF_RQ_SIZE - 1;
	for (unsigned int idx = 0; idx < NUM_OF_RQ_SIZE - 1; idx++) {
		if (s.cur_rq_size < rq_set[idx]) {
			cur_rq_size_idx = idx;
			break;
		}
	}

	return std::tuple<Idx, Idx, Idx, Idx>(prev_interval_idx, prev_action_idx, cur_interval_idx, cur_rq_size_idx);
}