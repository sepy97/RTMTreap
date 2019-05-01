#include <cstdio>
#include <cstdlib>
#include <utility>
#include <thread>
#include <mutex>
#include <immintrin.h>

#define MAXTHREADNUM 10

using namespace std;
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

pair<treap, treap> split (treap root, int key, treap* dupl) //операция split разделяет дерево на два по ключу, если есть совпадение со значением ключа
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
		while (status != _XBEGIN_STARTED)
		{
			status = _xbegin ();
		}
		if (status == _XBEGIN_STARTED)
		{
			splitted = split (root->right, key, dupl);
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
			splitted = split (root->left, key, dupl);
			root->left = splitted.second;
			_xend ();
		}
		
		return make_pair (splitted.first, root);
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
			(*dupl) = root;
			_xend ();
		}
		return make_pair (root->left, root->right);
		
	}
}

treap merge(treap left, treap right) //операция merge - сливает два дерева
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
			tmp = right;
			right = left;
			left = tmp;
			_xend ();
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
			left->right = merge (left->right, right);
			_xend ();
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
			right->left = merge (left, right->left);
			_xend ();
		}
		return right;
	}
}

void erase (treap& t, int key)
{
	if (t->key == key)
	{
		t = merge (t->left, t->right);
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

void insert (treap& t, treap toInsert)
{
	if (t == nullptr) t = toInsert;
	else if (toInsert->priority > t->priority)
	{
		treap dupl;
		auto tmp = split (t, toInsert->key, &dupl);
		toInsert->left = tmp.first;
		toInsert->right = tmp.second;
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

void testMerge (treap result, treap toMerge)
{
	result = merge (result, toMerge);
}

int main ()
{
	srand (time (NULL));
	
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
	}
	
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
	
	return 0;
}