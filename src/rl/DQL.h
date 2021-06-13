/*
#ifndef DQL_H
#define DQL_H

#include <vector>
#include <list>
#include <map>
#include <torch/torch.h>

class Qfunction : torch::nn::Module {
private:
	torch::nn::Linear input_linear;
	torch::nn::Linear output_linear;
public:
	Qfunction(unsigned int input_size, unsigned int hidden_size, unsigned intoutput_size);
	
	torch::Tensor forward(torch::Tensor input);

};


class  DQL {
private:
	int n_state = 3;
	int n_action = 4;
	int n_hidden = 50;
	struct state {
		int p_inter;
		int p_action;
		int c_inter;
	};


public:
	std::map<int, state> state_buf;
	std::list<at::Tensor*> replay_buf;


	void insert_state_t();
	void set_p_reward() {};
	void insert_transition() {};
	int get_action() {};


};


#endif // !DQL_H
*/