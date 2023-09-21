#include <Windows.h>
#include <atomic>


struct LoggingStruct
{
	unsigned long long int seqNo;
	unsigned long long int pushPopValue;
	unsigned long long int pushNodePtr;
	unsigned long long int topPtr;
	unsigned long long int popNodePtr;
	unsigned long long int threadID;
};

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

		LoggingStruct logging_push(T v)
		{
			node<T>* newNode = new node<T>(v);
			node<T>* oldTop;
			LoggingStruct ret;

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

			ret.popNodePtr = 0;
			ret.pushNodePtr = (unsigned long long int)newNode;
			ret.pushPopValue = (unsigned long long int)v;
			ret.topPtr = (unsigned long long int)oldTop;

			return ret;
		}

		T pop()
		{
			node<T>* nextTop;
			node<T>* oldTop;
			T ret;

			while (true)
			{
				oldTop = top;
				if (oldTop == nullptr)
					throw;

				nextTop = oldTop->next;
				if (InterlockedCompareExchangePointer((PVOID*)&top, nextTop, oldTop) == oldTop)
				{
					ret = oldTop->value;
					InterlockedDecrement(&_size);
					break;
				}

			}

			delete oldTop;
			return ret;
		}

		LoggingStruct logging_pop()
		{
			node<T>* nextTop;
			node<T>* oldTop;
			LoggingStruct ret;
			T retValue;

			while (true)
			{
				oldTop = top;
				if (oldTop == nullptr)
					throw;

				nextTop = oldTop->next;
				if (InterlockedCompareExchangePointer((PVOID*)&top, nextTop, oldTop) == oldTop)
				{
					retValue = oldTop->value;
					InterlockedDecrement(&_size);
					break;
				}

			}

			ret.popNodePtr = (unsigned long long int)oldTop;
			ret.pushNodePtr = 0;
			ret.pushPopValue = (unsigned long long int)retValue;
			ret.topPtr = (unsigned long long int)nextTop;

			delete oldTop;
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