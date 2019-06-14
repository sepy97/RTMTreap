#include <cstdio>
#include <cstdlib>
#include <utility>
#include <thread>
#include <mutex>
#include <immintrin.h>
#include <x86intrin.h>

#define INIT_PUSH 1000
#define MAXTHREADNUM 100
#define MAX_VOLUME 1000000

using namespace std;

class FastRandom {
private:
	unsigned long long rnd;
public:
	FastRandom(unsigned long long seed) { //time + threadnum
		rnd = seed;
	}
	unsigned long long rand() {
		rnd ^= rnd << 21;
		rnd ^= rnd >> 35;
		rnd ^= rnd << 4;
		return rnd;
	}
};

struct node
{
	//std::mutex m;
	node *left, *right;
	int key, priority;
	node () : key (0), priority (0), left (nullptr), right (nullptr) { }
	node (int key, int priority) : key (key), priority (priority), left (nullptr), right (nullptr) { }
	
};
typedef node* treap;

void dumpTreap (treap out, int spacingCounter = 0)
{
	if (out)
	{
		dumpTreap (out->right, spacingCounter + 1);
		for (int i = 0; i < spacingCounter; i++) printf ("_________");
		printf ("(%d.%d)\n", out->key, out->priority);
		dumpTreap (out->left, spacingCounter + 1);
	}
}

void split (treap root, treap& left, treap& right, int key, treap* dupl)
{
	if (root == nullptr)
	{
		left  = nullptr;
		right = nullptr;
		return;
	}
	
	if (root->key < key)
	{
		(*dupl) = nullptr;
		
		split (root->right, root, root->right, key, dupl);
	}
	else if (root->key > key)
	{
		(*dupl) = nullptr;
		
		split (root->left, root->left, root, key, dupl);
	}
	else
	{
		//auto tmp = *dupl;
		//auto tmp2 = root;
		//_m_prefetch (dupl);
		//__builtin_prefetch (dupl, 1, 3);
		//__builtin_prefetch (&root, 1, 3);
		auto volatile v = *root;
		auto volatile dv = *dupl;
		//auto volatile dvv = **dupl;
		auto volatile vl = left;
		auto volatile vr = right;
		auto volatile vll = root->left;
		auto volatile vlr = root->right;
		
		unsigned status = _xbegin ();
		/*while (status != _XBEGIN_STARTED)
		{
			std::this_thread::sleep_for (std::chrono::milliseconds(50+(FastRandom(time(nullptr)).rand ()%50)));
			status = _xbegin ();
		}*/
		if (status == _XBEGIN_STARTED)
		{
			(*dupl) = root;
			left    = root->left;
			right   = root->right;
			_xend ();
		}
		/*else
		{
			if (status & _XABORT_RETRY)
			{
				printf ("split retry\n");
			}
			else if (status & _XABORT_EXPLICIT)
			{
				printf ("split explicit\n");
			}
			else if (status & _XABORT_CONFLICT)
			{
				printf ("split conflict\n");
			}
			else if (status & _XABORT_CAPACITY)
			{
				printf ("split capacity\n");
			}
			else if (status & _XABORT_DEBUG)
			{
				printf ("split debug\n");
			}
			else if (status & _XABORT_NESTED)
			{
				printf ("split nested\n");
			}
			else
			{
				printf ("\n*****\nsplit status = %u\n*****\n", status);
			}
		}*/
	}
}

void merge (treap left, treap right, treap& result)
{
	if (left == nullptr || right == nullptr)
	{
		if (right == nullptr) result = left;
		else result = right;
		return;
	}
	
	if (left->key > right->key)
	{
		auto volatile v = *result;
		auto volatile vl = *left;
		auto volatile vr = *right;
		//__builtin_prefetch (&left, 1, 3);
		//__builtin_prefetch (&right, 1, 3);
		/*auto tmp = left;
		auto tmp2 = right;*/
		unsigned status = _xbegin ();
		if (status == _XBEGIN_STARTED)
		{
			std::swap (left, right);
			_xend ();
			
		}
		/*else
		{
			if (status & _XABORT_RETRY)
			{
				printf ("merge retry\n");
			}
			else if (status & _XABORT_EXPLICIT)
			{
				printf ("merge explicit\n");
			}
			else if (status & _XABORT_CONFLICT)
			{
				printf ("merge conflict\n");
			}
			else if (status & _XABORT_CAPACITY)
			{
				printf ("merge capacity\n");
			}
			else if (status & _XABORT_DEBUG)
			{
				printf ("merge debug\n");
			}
			else if (status & _XABORT_NESTED)
			{
				printf ("merge nested\n");
			}
			else
			{
				printf ("\n*****\nmerge status = %u\n*****\n", status);
			}
		}*/
		return;
		
	}
	
	if (left->priority > right->priority)
	{
		merge (left->right, right, left->right);
		result = left;
		return;
		
	}
	else
	{
		merge (left, right->left, right->left);
		result = right;
		return;
		
	}
}

void erase (treap& t, int key)
{
	if (t != nullptr)// return;
	{
		if (t->key == key)
		{
			//t = merge (t->left, t->right);
			merge (t->left, t->right, t);
		}
		else
		{
			if (key < t->key)
			{
				erase (t->left, key);
			}
			else
			{
				erase (t->right, key);
			}
		}
	}
}

void insert (treap& t, treap toInsert)
{
	if (t == nullptr) t = toInsert;
	else if (toInsert->priority > t->priority)
	{
		treap dupl;
		split (t, toInsert->left, toInsert->right, toInsert->key, &dupl);
		t = toInsert;
	}
	else
	{
		if (toInsert->key < t->key)
		{
			insert (t->left, toInsert);
		}
		else
		{
			insert (t->right, toInsert);
		}
	}
}

treap toTest;

void testMerge (const int volume, int threadNum)
{
	FastRandom* ran = new FastRandom (time(NULL) + threadNum);
	for (int i = 0; i < volume; i++)
	{
		int insOrDel = ran->rand()%2;
		if (insOrDel)
		{
			auto toAdd = new node (ran->rand()%volume, ran->rand ()%volume);
			insert (toTest, toAdd);
		}
		else
		{
			int data = ran->rand ()%volume;
			
			erase (toTest, data);
		}
	}
	
}

int main (int argc, char** argv)
{
	int maxThreads = 0;
	if (argc > 1)
	{
		maxThreads = atoi(argv[1]);
	}
	else
	{
		maxThreads = 1;
		//printf ("no arguments :( \n");
		//return 0;
	}
	
	toTest = new node ();
	FastRandom* ran = new FastRandom (time(NULL));
	
	for (int i = 0; i < INIT_PUSH; i++)
	{
		auto toAdd = new node (ran->rand()%INIT_PUSH, ran->rand ()%INIT_PUSH);
		insert (toTest, toAdd);
		
	}
	
	std::thread thr[maxThreads];
	
	uint64_t tick = __rdtsc ()/10000;
	
	for (int i = 0; i < maxThreads; i++)
	{
		thr[i] = std::thread (testMerge, MAX_VOLUME/maxThreads, i);
	}
	
	for (int i = 0; i < maxThreads; i++)
	{
		thr[i].join ();
	}
	
	uint64_t tick2 = __rdtsc ()/10000;
	printf ("%llu\n", tick2 - tick);
	
	return 0;
}