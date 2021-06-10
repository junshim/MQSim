#include "RL.h"

namespace SSD_Components
{

		void initiate_theta_0(at::Tensor theta_0) {
			theta_0[0][0] = 1; theta_0[0][1] = 2; theta_0[0][2] = 3; theta_0[0][3] = 4;
			theta_0[1][0] = 1; theta_0[1][1] = 2; theta_0[1][2] = 3; theta_0[1][3] = 3;
			theta_0[2][0] = 1; theta_0[2][1] = 2; theta_0[2][2] = 2; theta_0[2][3] = 2;
			theta_0[3][0] = 1; theta_0[3][1] = 1; theta_0[3][2] = 1; theta_0[3][3] = 1;
		}

		void tensor_multiply(at::Tensor dest, at::Tensor src, int row, int col) {
			for (int r = 0; r < row; r++) {
				for (int c = 0; c < col; c++) {
					dest[r][c] *= src[r][c];
				}
			}
		}

		void simple_convert_into_pi_from_theta(at::Tensor theta_0, at::Tensor pi, int row, int col) {
			//at::Tensor tensor1 = torch::randn({ 1, 2, 3 });
			//at::Tensor pi = torch::zeros((row, col));


			//auto theta_arr = torch::Tensor(theta, (4, 4), torch::Device(torch::kCUDA));
			//auto theta_arr = torch::Tensor(theta, (4, 4));
			std::cout << "theta_0: " << theta_0 << std::endl;
			//at::Tensor pi = torch::randn({row,col});
			//std::cout << "nansum: "<<theta_0.nansum(1) << std::endl; //if arg==1, sum over all colums.->output:1-dim array https://stackoverflow.com/questions/44790670/torch-sum-a-tensor-along-an-axis
			//std::cout << "first ele of nansum tensor: " << theta_0.nansum(1)[0];
			//std::cout << "second ele of nansum tensor: " << theta_0.nansum(1)[1];

			for (int r = 0; r < row; r++) {
				for (int c = 0; c < col; c++) {
					pi[r][c] = theta_0[r][c] / theta_0.sum(1)[r];  //theta_0.sum(1)[r]은 행마다의 합
				}
			}
			std::cout << "converted pi from theta_0 :" << pi << std::endl;

			//return (float*)theta;
		}

		int get_action(int state, at::Tensor Q, double epsilon, at::Tensor pi_0) {
			int num_of_victims[4] = { 1, 2, 3, 4 };
			double max = 32767;
			int next_num_of_victims;
			srand(time(NULL));
			int r = rand() % 4;

			if ((rand() / max) < epsilon) {
				next_num_of_victims = pi_0[state][r].item<int>();
			}
			else {
				next_num_of_victims = Q.argmax(1)[state].item<int>(); //Q.argmax(1) -> 각 행에서 최대를 갖는 arg idex들을 모아 arr를 output으로 줌.
				//next_num_of_victims = max_element(pi_0[state][0], pi_0[state][3]);
			}

			return next_num_of_victims;
		}

		void sarsa(int s, int a, double r, int s_next, int a_next, at::Tensor Q, double eta, double gamma) {
			//do system have destination?
			if (s_next == 1) {
				Q[s][a] = Q[s][a] + eta * (r - Q[s][a]);
			}
			else {
				Q[s][a] = Q[s][a] + eta * (r + gamma * Q[s_next][a_next] - Q[s][a]);
			}
		}
	
}