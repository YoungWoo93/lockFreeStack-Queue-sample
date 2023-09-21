#include <Windows.h>
#include <atomic>


namespace winAPI
{
	template <typename T>
	struct node
	{
		node(T v) : value(v), next(nullptr) {
		};

		T value;
		node* next;
	};

	template <typename T>
	class LockFreeStack {
	public:

		LockFreeStack() : _size(0), top(nullptr) {
			seqCount = 0;
		}
		~LockFreeStack() {};

		int seqCount;



		void push(T v)
		{
			node<T>* newNode = new node<T>(v);
			node<T>* oldTop;

			while (true)
			{
				oldTop = top;
				newNode->next = oldTop;

				if (InterlockedCompareExchangePointer((PVOID*)&top, newNode, oldTop) == oldTop)
				{
					InterlockedIncrement(&_size);
					break;
				}

			}
		}

		T pop()
		{
			node<T>* popNode;
			node<T>* oldTop;
			T ret;

			while (true)
			{
				oldTop = top;
				if (oldTop == nullptr)
					throw;

				popNode = oldTop;
				if (InterlockedCompareExchangePointer((PVOID*)&top, popNode->next, oldTop) == oldTop)
				{
					ret = popNode->value;
					InterlockedDecrement(&_size);
					break;
				}

			}

			delete popNode;
			return ret;
		}

		size_t size()
		{
			return _size;
		}

		size_t _size;
		node<T>* top;
	};
}