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
	typedef unsigned long long int key;

	template <typename T>
	struct node
	{
		node(T v) : value(v), next(0) {
		};

		T value;
		key next;
	};

	template <typename T>
	class LockFreeStack {
	public:

		LockFreeStack() : _size(0), topKey(0), pushNo(0) {
		}
		~LockFreeStack() {};

		inline key MAKE_KEY(node<T>* nodePtr, unsigned long long int pushNo)
		{
			return (pushNo << 48) | (key)nodePtr;
		}

		inline node<T>* MAKE_NODEPTR(key _key)
		{
			return (node<T>*)(0x0000FFFFFFFFFFFF & _key);
		}



		void push(T v)
		{
			node<T>* newNodePtr = new node<T>(v);
			key newNodeKey = MAKE_KEY(newNodePtr, InterlockedIncrement(&pushNo));
			key oldTopKey;

			while (true)
			{
				oldTopKey = topKey;
				newNodePtr->next = oldTopKey;

				if (InterlockedCompareExchangePointer((PVOID*)&topKey, (PVOID)newNodeKey, (PVOID)oldTopKey) == (PVOID)oldTopKey)
				{
					InterlockedIncrement(&_size);
					break;
				}

			}
		}

		LoggingStruct logging_push(T v)
		{
			node<T>* newNodePtr = new node<T>(v);
			key newNodeKey = MAKE_KEY(newNodePtr, InterlockedIncrement(&pushNo));
			key oldTopKey;
			LoggingStruct ret;

			while (true)
			{
				oldTopKey = topKey;
				newNodePtr->next = oldTopKey;

				if (InterlockedCompareExchangePointer((PVOID*)&topKey, (PVOID)newNodeKey, (PVOID)oldTopKey) == (PVOID)oldTopKey)
				{
					InterlockedIncrement(&_size);
					break;
				}

			}

			ret.popNodePtr = 0;
			ret.pushNodePtr = (unsigned long long int)newNodePtr;
			ret.pushPopValue = (unsigned long long int)v;
			ret.topPtr = (unsigned long long int)MAKE_NODEPTR(oldTopKey);

			return ret;
		}

		T pop()
		{
			key nextTopKey;
			key oldTopKey;
			node<T>* popNodePtr;
			T ret;

			while (true)
			{
				oldTopKey = topKey;
				popNodePtr = MAKE_NODEPTR(oldTopKey);
				if (popNodePtr == nullptr)
					throw;

				nextTopKey = popNodePtr->next;
				
				if (InterlockedCompareExchangePointer((PVOID*)&topKey, (PVOID)nextTopKey, (PVOID)oldTopKey) == (PVOID)oldTopKey)
				{
					ret = popNodePtr->value;
					InterlockedDecrement(&_size);
					break;
				}
			}

			delete popNodePtr;

			return ret;
		}

		LoggingStruct logging_pop()
		{
			key nextTopKey;
			key oldTopKey;
			node<T>* popNodePtr;
			LoggingStruct ret;
			T retValue;

			while (true)
			{
				oldTopKey = topKey;
				popNodePtr = MAKE_NODEPTR(oldTopKey);
				if (popNodePtr == nullptr)
					throw;

				nextTopKey = popNodePtr->next;

				if (InterlockedCompareExchangePointer((PVOID*)&topKey, (PVOID)nextTopKey, (PVOID)oldTopKey) == (PVOID)oldTopKey)
				{
					retValue = popNodePtr->value;
					InterlockedDecrement(&_size);
					break;
				}
			}

			ret.popNodePtr = (unsigned long long int)popNodePtr;
			ret.pushNodePtr = 0;
			ret.pushPopValue = (unsigned long long int)retValue;
			ret.topPtr = (unsigned long long int)MAKE_NODEPTR(nextTopKey);

			delete popNodePtr;

			return ret;
		}

		size_t size()
		{
			return _size;
		}

		size_t _size;
		key topKey;
		unsigned long long int pushNo;
	};
}