#include <Windows.h>
#include <atomic>

namespace standard
{
	using namespace std;

	template <typename T>
	struct node;

	template <typename T>
	struct refCounter
	{
		refCounter() : refCount(0), nodePtr(nullptr) {
		}
		refCounter(node<T>* node) : refCount(0), nodePtr(node) {
		}

		int refCount;
		node<T>* nodePtr;
	};

	template <typename T>
	struct node
	{
		node(T v) : value(v), deleteCount(0),  next() {
		}
		~node() {
		}

		T value;
		atomic<int> deleteCount;
		refCounter<T> next;
	};

	template <typename T>
	class LockFreeStack {
	public:
		LockFreeStack() : _size(0), top(nullptr) {
		}
		~LockFreeStack() {};


		void push(T v)
		{
			refCounter<T> newNodePtr(new node<T>(v));
			newNodePtr.refCount++; //스택이 바라볼 참조 카운터를 하나 추가한채 시작한다.
			newNodePtr.nodePtr->next = top;

			while (true)
			{
				if (top.compare_exchange_weak(newNodePtr.nodePtr->next, newNodePtr))
				{
					_size++;
					break;
				}
			}
		}

		T pop()
		{
			refCounter<T> popNodePtr = top;
			int ret;

			while (true)
			{
				while (true)
				{
					refCounter<T> tempPtr = popNodePtr;
					tempPtr.refCount++;

					if (top.compare_exchange_strong(popNodePtr, tempPtr))
					{
						popNodePtr.refCount = tempPtr.refCount;
						break;
					}
				}


				node<T>* popNode = popNodePtr.nodePtr;
				if (popNode == nullptr)
					throw;

				if (top.compare_exchange_strong(popNodePtr, popNode->next))
				{
					ret = popNode->value;
					_size--;

					int d = popNodePtr.refCount - 2; // POP에 성공한 스레드의 참조카운터 1과 스택의 참조카운터 1을 한번에 제거해준다.
					if (popNode->deleteCount.fetch_add(d) == -d)
						delete popNode;

					break;
				}
				else if (popNode->deleteCount.fetch_sub(1) == 1)
					delete popNode;

			}

			return ret;
		}

		size_t size()
		{
			return _size;
		}

		atomic<size_t> _size;
		atomic<refCounter<T>> top;
	};
}