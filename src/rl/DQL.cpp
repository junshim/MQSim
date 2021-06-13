/*
#include "DQL.h"
#include <torch/torch.h>

Qfunction::Qfunction(unsigned int input_size, unsigned int  hidden_size, unsigned int output_size) {

	input_linear = torch::nn::Linear(input_size, hidden_size);
	output_linear = torch::nn::Linear(hidden_size, output_size);

}

torch::Tensor Qfunction::forward(torch::Tensor input) {
	torch::Tensor output = input_linear->forward(input);
	output = output_linear->forward(output);

	return output;
}

void DQL::insert_state_t(int request_id) {

	if (request_id == 0) {
		state temp = { 0, 0, 0 };
		state_buf.insert(std::pair<int, state>(request_id, temp));
	}
	else {
		state temp = { 0, 0, 0 };
		p_state = state_buf.find(request_id - 1);
		if(p_state)

	}

}
*/