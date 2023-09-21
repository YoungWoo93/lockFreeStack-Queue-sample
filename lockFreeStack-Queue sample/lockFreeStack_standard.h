#include <Windows.h>
#include <atomic>


namespace standard
{
	using namespace std;

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
		LockFreeStack() : _size(0), top(nullptr){
		}
		~LockFreeStack() {};


		void push(T v)
		{
			node<T>* newNode = new node<T>(v);
			node<T>* oldTop;

			newNode->next = top;

			while (true)
			{
				if (top.compare_exchange_weak(newNode->next, newNode))
				{
					_size++;
					break;
				}

			}
		}

		T pop()
		{
			node<T>* popNode = top;
			int ret;

			while (true)
			{
				if (popNode == nullptr)
					throw;

				if (top.compare_exchange_weak(popNode, popNode->next))
				{
					ret = popNode->value;
					_size--;
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

		atomic<size_t> _size;
		atomic<node<T>*> top;
	};
}