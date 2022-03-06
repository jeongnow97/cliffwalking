#include <iostream>
#include<tuple>
//#include <cstdlib>
//#include <ctime>

using namespace std;

#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3
#define ALPHA 0.5

double q_table[48][4] = { 0, }; //Q(s,a) 0행렬으로 초기화

class Env
{
private:
	bool game_over = false;
	int x_pos = 0;
	int y_pos = 0;
	int next_x_pos = 0;
	int next_y_pos = 0;
	int reward = 0;
	int gridworld[4][12] = { 0, };

public:
	tuple<int, int, bool> get_reward(int location, int action) // s, a를 받으면 r, s', 게임종료 여부를 반환
	{
		y_pos = location / 12; //몫->y좌표
		x_pos = location % 12; //나머지->x좌표

		switch (action) {
		case UP:
			next_x_pos = x_pos;
			next_y_pos = y_pos - 1;
			break;
		case DOWN:
			next_x_pos = x_pos;
			next_y_pos = y_pos + 1;
			break;
		case LEFT:
			next_x_pos = x_pos - 1;
			next_y_pos = y_pos;
			break;
		case RIGHT:
			next_x_pos = x_pos + 1;
			next_y_pos = y_pos;
			break;
		}

		if (next_x_pos == -1) //영역 밖으로 나갔을 때 원위치
			next_x_pos += 1;
		else if (next_x_pos == 12)
			next_x_pos -= 1;
		else if (next_y_pos == -1)
			next_y_pos += 1;
		else if (next_y_pos == 4)
			next_y_pos -= 1;

		int next_location = next_x_pos + next_y_pos * 12; //다음 상태 location으로 변환

		if (next_location > 36 && next_location < 47) //cliff에서는 reward -100 나머지는 -1
		{
			reward = -100;
			next_location = 36; //원위치
			//cout << "cliff에 빠졌습니다." << endl;
		}
		else
			reward = -1;

		if (next_location == 47) //게임종료여부 확인
			game_over = true;
		else
			game_over = false;

		/*
		gridworld[y_pos][x_pos] = 1;
		gridworld[next_y_pos][next_x_pos] = 1;

		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 12; j++)
			{
				cout << gridworld[i][j] << " ";
			}
			cout << endl;
		}
		*/

		return tuple<int, int, bool>(next_location, reward, game_over); // s,a를 넣어서 r과 s' 반환
	}
};

class Agent
{
private:
	int move;
	//double q_table[48][4] = { 0, }; //Q(s,a) 0행렬으로 초기화
public:
	int policy_epsilon_greedy(int location) // 액션 선정, 90%확률로 greedy하게 가고, 10% 확률로 0,1,2,3 중 하나 반환
	{
		//srand((unsigned int)time(NULL));
		if (rand() % 10 == 9) //10% 확률로
			move = rand() % 4; // 0,1,2,3 중 하나 무작위 반환
		else // 나머지 90%로 greedy하게 움직인다.
		{
			if (q_table[location][0] >= q_table[location][1] && q_table[location][0] >= q_table[location][2] && q_table[location][0] >= q_table[location][3])
				move = 0;
			else if (q_table[location][1] >= q_table[location][0] && q_table[location][1] >= q_table[location][2] && q_table[location][1] >= q_table[location][3])
				move = 1;
			else if (q_table[location][2] >= q_table[location][0] && q_table[location][2] >= q_table[location][1] && q_table[location][2] >= q_table[location][3])
				move = 2;
			else if (q_table[location][3] >= q_table[location][0] && q_table[location][3] >= q_table[location][1] && q_table[location][3] >= q_table[location][2])
				move = 3;
		}
		return move;
	}

	double sarsa(int location, int action, int reward, int next_location, int next_action)
	{
		q_table[location][action] = q_table[location][action] + ALPHA * (reward + q_table[next_location][next_action] - q_table[location][action]);
		return q_table[location][action];
	}

};

int main(void)
{
	double episode_reward; //에피소드 안에서 스텝별 보상 저장하는 곳
	int cur_location; //현 위치
	int cur_action; //현 액션

	int reward; //s,a에 의해 발생하는 보상
	int next_location; //다음 위치
	int next_action; //다음 액션
	bool game_over; //게임 종료 여부

	int footprint[48] = { 0, }; //이동경로

	Agent agent; // 에이전트 클래스
	Env env; //환경 클래스

	for (int i = 0; i < 500; i++)
	{
		int footprint[48] = { 0, };
		footprint[36] = 1;
		game_over = false;
		episode_reward = 0;
		cur_location = 36; //위치 초기화
		cur_action = agent.policy_epsilon_greedy(cur_location); //action 0,1,2,3 중 하나 반환
		while (game_over == false) //goalin 할 때까지 반복
		{
			tuple<int, int, int> env_ = env.get_reward(cur_location, cur_action);
			next_location = get<0>(env_);
			reward = get<1>(env_);
			game_over = get<2>(env_);
			//cout << "game over : " << game_over << " reward : " << reward << endl;

			episode_reward += reward;

			next_action = agent.policy_epsilon_greedy(next_location);
			q_table[cur_location][cur_action] = agent.sarsa(cur_location, cur_action, reward, next_location, next_action);

			cur_location = next_location;
			cur_action = next_action;
			footprint[next_location] = 1;
		}
		cout << "episode : " << i + 1 << " reward : " << episode_reward << endl;
		if (i == 499)
		{
			for (int j = 0; j < 48; j++)
			{
				if (j > 0 && j % 12 == 0)
					cout << endl;
				cout << footprint[j] << " ";
				if (j == 47)
					cout << endl;
			}
		}
	}

	/*
	int a, b, c;
	int location = 36;
	int action = 0;
	Env env;
	tuple<int, int, int> first = env.get_reward(location, action);

	a = get<0>(first);
	b = get<1>(first);
	c = get<2>(first);
	cout << "다음상태:" << a << " 보상:" << b << " 종료여부: " << c << endl;
	*/
	//double asdf[][4] = q_table[][4];
}