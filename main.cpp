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
		//_m_prefetch (dupl);
		__builtin_prefetch (dupl, 1, 3);
		__builtin_prefetch (root, 1, 3);
		unsigned status = _xbegin ();
		/*while (status != _XBEGIN_STARTED)
		{
			_xabort (1);
			status = _xbegin ();
		}*/
		if (status == _XBEGIN_STARTED)
		{
			(*dupl) = root;
			left    = root->left;
			right   = root->right;
			_xend ();
		}
		else
		{
			switch (status)
			{
				case _XABORT_RETRY:
				{
					printf ("split retry\n");
					break;
				}
				case _XABORT_EXPLICIT:
				{
					printf ("split explicit\n");
					break;
				}
				case _XABORT_CONFLICT:
				{
					printf ("split conflict\n");
					break;
				}
				case _XABORT_CAPACITY:
				{
					printf ("split capacity\n");
					break;
				}
				case _XABORT_DEBUG:
				{
					printf ("split debug\n");
					break;
				}
				case _XABORT_NESTED:
				{
					printf ("split nested\n");
					break;
				}
			}
		}
	}
}

/*pair<treap, treap> splitt (treap root, int key, treap* dupl) //операция split разделяет дерево на два по ключу, если есть совпадение со значением ключа
// - кидает эту вершину в dupl
{
	if (root == nullptr) return make_pair (nullptr, nullptr);
	
	if (root->key < key)
	{
		(*dupl) = nullptr;
		pair<treap, treap> splitted;
		splitted.first = nullptr;
		splitted.second = nullptr;
		unsigned status = _xbegin ();
		//if (status == _XABORT_RETRY)
		{
			while (status != _XBEGIN_STARTED)
			{
				status = _xbegin ();
			}
		}
		//else if (status == _XA)
		
		if (status == _XBEGIN_STARTED)
		{
			splitted = splitt (root->right, key, dupl);
			root->right = splitted.first;
			_xend ();
		}
		
		return make_pair (root, splitted.second); //нужно ли это в RTM?
	}
	else if (root->key > key)
	{
		(*dupl) = nullptr;
		pair<treap, treap> splitted;
		splitted.first = nullptr;
		splitted.second = nullptr;
		unsigned status = _xbegin ();
		while (status != _XBEGIN_STARTED)
		{
			status = _xbegin ();
		}
		if (status == _XBEGIN_STARTED)
		{
			splitted = splitt (root->left, key, dupl);
			root->left = splitted.second;
			_xend ();
		}
		
		return make_pair (splitted.first, root);
	}
	else
	{
		//_m_prefetch (dupl);
		unsigned status = _xbegin ();
		while (status != _XBEGIN_STARTED)
		{
			status = _xbegin ();
		}
		if (status == _XBEGIN_STARTED)
		{
			(*dupl) = root;
			_xend ();
		}
		return make_pair (root->left, root->right);
		
	}
}*/

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
		
		__builtin_prefetch (left, 1, 3);
		__builtin_prefetch (right, 1, 3);
		unsigned status = _xbegin ();
		/*while (status != _XBEGIN_STARTED)
		{
			_xabort (1);
			status = _xbegin ();
		}*/
		if (status == _XBEGIN_STARTED)
		{
			std::swap (left, right);
			_xend ();
			
		}
		else
		{
			switch (status)
			{
				case _XABORT_RETRY:
				{
					printf ("merge retry\n");
					break;
				}
				case _XABORT_EXPLICIT:
				{
					printf ("merge explicit\n");
					break;
				}
				case _XABORT_CONFLICT:
				{
					printf ("merge conflict\n");
					break;
				}
				case _XABORT_CAPACITY:
				{
					printf ("merge capacity\n");
					break;
				}
				case _XABORT_DEBUG:
				{
					printf ("merge debug\n");
					break;
				}
				case _XABORT_NESTED:
				{
					printf ("merge nested\n");
					break;
				}
			}
		}
		return;
		
	}
	
	if (left->priority > right->priority)
	{
		merge (left->right, right, left->right);
		result = left;
		return;
		/*unsigned status = _xbegin ();
		while (status != _XBEGIN_STARTED)
		{
			_xabort (1);
			status = _xbegin ();
		}
		if (status == _XBEGIN_STARTED)
		{
			result = left;
			_xend ();
		}
		else
		{
			switch (status)
			{
				case _XABORT_RETRY:
				{
					printf ("merge retry\n");
					break;
				}
				case _XABORT_EXPLICIT:
				{
					printf ("merge explicit\n");
					break;
				}
				case _XABORT_CONFLICT:
				{
					printf ("merge conflict\n");
					break;
				}
				case _XABORT_CAPACITY:
				{
					printf ("merge capacity\n");
					break;
				}
				case _XABORT_DEBUG:
				{
					printf ("merge debug\n");
					break;
				}
				case _XABORT_NESTED:
				{
					printf ("merge nested\n");
					break;
				}
			}
		}*/
	}
	else
	{
		merge (left, right->left, right->left);
		result = right;
		return;
		/*unsigned status = _xbegin ();
		while (status != _XBEGIN_STARTED)
		{
			_xabort (1);
			status = _xbegin ();
		}
		if (status == _XBEGIN_STARTED)
		{
			result = right;
			_xend ();
		}
		else
		{
			switch (status)
			{
				case _XABORT_RETRY:
				{
					printf ("merge retry\n");
					break;
				}
				case _XABORT_EXPLICIT:
				{
					printf ("merge explicit\n");
					break;
				}
				case _XABORT_CONFLICT:
				{
					printf ("merge conflict\n");
					break;
				}
				case _XABORT_CAPACITY:
				{
					printf ("merge capacity\n");
					break;
				}
				case _XABORT_DEBUG:
				{
					printf ("merge debug\n");
					break;
				}
				case _XABORT_NESTED:
				{
					printf ("merge nested\n");
					break;
				}
			}
		}*/
	}
}

/*treap mmerge(treap left, treap right) //операция merge - сливает два дерева
{
	if (left == nullptr || right == nullptr) return right == nullptr ? left : right;
	
	if (left->key > right->key)
	{
		treap tmp = nullptr;
		unsigned status = _xbegin ();
		while (status != _XBEGIN_STARTED)
		{
			status = _xbegin ();
		}
		if (status == _XBEGIN_STARTED)
		{
			std::swap (left, right);
			_xend ();
		}
		else
		{
			switch (status)
			{
				case _XABORT_RETRY:
				{
					printf ("merge retry\n");
					break;
				}
				case _XABORT_EXPLICIT:
				{
					printf ("merge explicit\n");
					break;
				}
				case _XABORT_CONFLICT:
				{
					printf ("merge conflict\n");
					break;
				}
				case _XABORT_CAPACITY:
				{
					printf ("merge capacity\n");
					break;
				}
				case _XABORT_DEBUG:
				{
					printf ("merge debug\n");
					break;
				}
				case _XABORT_NESTED:
				{
					printf ("merge nested\n");
					break;
				}
			}
		}
	}
	
	if (left->priority > right->priority)
	{
		unsigned status = _xbegin ();
		while (status != _XBEGIN_STARTED)
		{
			status = _xbegin ();
		}
		if (status == _XBEGIN_STARTED)
		{
			left->right = mmerge (left->right, right);
			_xend ();
		}
		else
		{
			switch (status)
			{
				case _XABORT_RETRY:
				{
					printf ("merge retry\n");
					break;
				}
				case _XABORT_EXPLICIT:
				{
					printf ("merge explicit\n");
					break;
				}
				case _XABORT_CONFLICT:
				{
					printf ("merge conflict\n");
					break;
				}
				case _XABORT_CAPACITY:
				{
					printf ("merge capacity\n");
					break;
				}
				case _XABORT_DEBUG:
				{
					printf ("merge debug\n");
					break;
				}
				case _XABORT_NESTED:
				{
					printf ("merge nested\n");
					break;
				}
			}
		}
		return left;
	}
	else
	{
		unsigned status = _xbegin ();
		while (status != _XBEGIN_STARTED)
		{
			status = _xbegin ();
		}
		if (status == _XBEGIN_STARTED)
		{
			right->left = mmerge (left, right->left);
			_xend ();
		}
		else
		{
			switch (status)
			{
				case _XABORT_RETRY:
				{
					printf ("merge retry\n");
					break;
				}
				case _XABORT_EXPLICIT:
				{
					printf ("merge explicit\n");
					break;
				}
				case _XABORT_CONFLICT:
				{
					printf ("merge conflict\n");
					break;
				}
				case _XABORT_CAPACITY:
				{
					printf ("merge capacity\n");
					break;
				}
				case _XABORT_DEBUG:
				{
					printf ("merge debug\n");
					break;
				}
				case _XABORT_NESTED:
				{
					printf ("merge nested\n");
					break;
				}
			}
		}
		return right;
	}
}*/

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
		/*auto tmp = split (t, toInsert->key, &dupl);
		toInsert->left = tmp.first;
		toInsert->right = tmp.second;*/
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
	//queue <int> testedValues;
	
	FastRandom* ran = new FastRandom (time(NULL) + threadNum);
	for (int i = 0; i < volume; i++)
	{
		int insOrDel = ran->rand()%2;
		if (insOrDel)
		{
			/*int toInsert = ran->rand()%volume; // testedValues.front ();
			testedValues.pop ();*/
			auto toAdd = new node (ran->rand()%volume, ran->rand ()%volume);
			insert (toTest, toAdd);
		}
		else
		{
			int data = ran->rand ()%volume;
			
			erase (toTest, data);
			//testedValues.push (data);
		}
	}
	
}
/*
void testMerge (treap result, treap toMerge)
{
	result = merge (result, toMerge);
}*/

int main ()
{
	/*srand (time (NULL));
	
	auto t1 = new node (13, 3);
	auto t2 = new node (9, 7);
	auto t3 = new node (14, 4);
	auto t4 = new node (13, 8);
	
	t2 = merge (t1, t2);
	t4 = merge (t3, t4);
	
	dumpTreap (t4);
	printf ("*****************************\n");
	
	dumpTreap (t2);
	printf ("*****************************\n");
	
	auto t5 = merge (t2, t4);
	
	dumpTreap (t5);
	printf ("*****************************\n");
	
	treap t [MAXTHREADNUM];
	for (int i = 0; i < MAXTHREADNUM; i++)
	{
		t[i] = new node (i, rand()%100);
	}
	
	for (int i = 0; i < MAXTHREADNUM; i++)
	{
		dumpTreap (t[i]);
		printf ("@@@@@@@@@@@@@@@@@@@@@@\n");
	}*/
	int maxThreads = 2;
	toTest = new node ();
	FastRandom* ran = new FastRandom (time(NULL));
	
	for (int i = 0; i < INIT_PUSH; i++)
	{
		/*auto toAdd = new node (testedValues.front (), ran->rand()%(INIT_PUSH));
		testedValues.pop ();
		insert (toTest, toAdd);
		*/
		auto toAdd = new node (ran->rand()%INIT_PUSH, ran->rand ()%INIT_PUSH);
		insert (toTest, toAdd);
		
	}
	
	std::thread thr[maxThreads];
	
	uint64_t tick = __rdtsc ()/100000;
	
	for (int i = 0; i < maxThreads; i++)
	{
		//toTest.push (i);
		thr[i] = std::thread (testMerge, MAX_VOLUME/maxThreads, i); //testPush, &toTest, i);
	}
	
	for (int i = 0; i < maxThreads; i++)
	{
		thr[i].join ();
	}
	
	uint64_t tick2 = __rdtsc ()/100000;
	printf ("%llu\n", tick2 - tick);
	
	
	return 0;
	/*
	std::thread thr [MAXTHREADNUM];
	auto result = new node (101, 101);
	for (int i = 0; i < MAXTHREADNUM; i++)
	{
		thr[i] = std::thread (testMerge, result, t[i]);
	}
	
	for (int i = 0; i < MAXTHREADNUM; i++)
	{
		thr[i].join ();
	}
	
	dumpTreap (result);
	
	return 0;*/
}