#include <iostream>
#include <thread>
#include <vector>

#include "lockFreeStack_windows.h"
#include "lockFreeStack_standard.h"

using namespace std;

winAPI::LockFreeStack<unsigned long long int> stack;
//standard::LockFreeStack<unsigned long long int> stack;
unsigned long long int seqCount;

bool threadRun;
int threadCount;
unsigned long long int testCount;
vector<vector<unsigned long long int>> result;
vector<vector<LoggingStruct>> loggingResult;

void PushPop_process(int startNumber, int pushPopSize)
{
	while (!threadRun)
	{
		//wait for start signal
	};

	unsigned long long int currentNumber = startNumber;
	while (threadRun)
	{
		if (currentNumber > testCount)
			break;
		InterlockedIncrement(&seqCount);
		for (int i = 0; i < pushPopSize; i++)
		{
			stack.push(currentNumber);
			currentNumber += threadCount;
		}

		for (int i = 0; i < pushPopSize; i++)
		{
			result[startNumber].push_back(stack.pop());
		}
	}
}

void loggingPushPop_process(int startNumber, int pushPopSize)
{
	while (!threadRun)
	{
		//wait for start signal
	};

	unsigned long long int currentNumber = startNumber;
	while (threadRun)
	{
		if (currentNumber > testCount)
			break;

		for (int i = 0; i < pushPopSize; i++)
		{
			auto logBlock = stack.logging_push(currentNumber);
			logBlock.seqNo = InterlockedIncrement(&seqCount);
			logBlock.threadID = startNumber;

			loggingResult[startNumber].push_back(logBlock);
			currentNumber += threadCount;
		}

		for (int i = 0; i < pushPopSize; i++)
		{
			auto logBlock = stack.logging_pop();
			logBlock.seqNo = InterlockedIncrement(&seqCount);
			logBlock.threadID = startNumber;

			loggingResult[startNumber].push_back(logBlock);
		}
	}
}

void main()
{
	testCount = 10000000;
	threadCount = 4;
	// �׽�Ʈ ������ 2�� ����

	vector<thread> threads;
	result.resize(threadCount);
	loggingResult.resize(threadCount);

	for (int i = 0; i < threadCount; i++)
		threads.push_back(thread(PushPop_process, i, 10000));
	

	threadRun = true;

	Sleep(100000);

	threadRun = false;
	for (int i = 0; i < threadCount; i++)
		threads[i].join();

	// ���������� break �� result Ȯ��
	cout << stack.size();
	cout << "break here";
}

