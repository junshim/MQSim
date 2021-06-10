#ifndef RETRY_MODEL_H
#define RETRY_MODEL_H

#include <torch/script.h>
#include <stdlib.h>
#include <random>
namespace SSD_Components
{
	void initiate_theta_0(at::Tensor theta_0);
	void tensor_multiply(at::Tensor dest, at::Tensor src, int row, int col);
	void simple_convert_into_pi_from_theta(at::Tensor theta_0, at::Tensor pi, int row, int col);
	int get_action(int state, at::Tensor Q, double epsilon, at::Tensor pi_0);
	void sarsa(int s, int a, double r, int s_next, int a_next, at::Tensor Q, double eta, double gamma);
}

#endif