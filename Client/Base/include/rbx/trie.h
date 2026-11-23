// Base/include/rbx/trie.h: October 2013 reconstruction (Project OGRE).
// The character trie behind reflection's member lookup (MemberDescriptorContainer's
// descriptorLookup is rbx::trie<Descriptor*, 50>, reflection/member.h).
// Evidence: PLAYER holds the strings "trie depth exceeded" and "key out of range"; the node
// constructor at 0x00614f60 throws depth_exceeded_exception once depth passes 0x32, and the
// exception's vftable is rbx::trie<PropertyDescriptor*,50>::depth_exceeded_exception.
// Node layout (tail string, 96 child pointers, value pointer, depth) and every inline body are
// kept as the tree had them: they are instantiated into reflection code.
//
// Keys are 7-bit ASCII from 32 to 127. A node whose subtree holds a single key keeps the rest of
// that key in `tail` instead of a chain of nodes; it is split into real children when a second
// key arrives (removeLeaf). Lookups are lock-free reads; operator[] (insertion) is not thread safe.
#pragma once

#include <boost/noncopyable.hpp>
#include <boost/pool/object_pool.hpp>

namespace rbx {

// V is stored by value in its own pool (reflection stores descriptor pointers). maxDepth bounds
// the node chain: a key longer than that throws depth_exceeded_exception when it is inserted.
// Memory: every node carries a full 96-pointer table (384 bytes on x86) plus its tail string, so
// a few hundred member names cost on the order of a hundred kilobytes; compute_memory_usage
// reports the node part.
template<class V, unsigned int maxDepth>
class trie : public boost::noncopyable
{
public:
	class depth_exceeded_exception : public std::exception
	{
	public:
		virtual const char* what() const throw() { return "trie depth exceeded"; };
	};

	// A key character below 32.
	class bad_key : public std::exception
	{
	public:
		const char key;
		bad_key(char key) : key(key) {}
		virtual const char* what() const throw() { return "key out of range"; };
	};

private:
	class Node;
	// Nodes and values come from pools the trie owns; destroy() hands them back one by one.
	typedef boost::object_pool<Node> Pool;
	typedef boost::object_pool<V> ValuePool;

	class Node : public boost::noncopyable
	{
		friend class trie;

		std::string tail; // remaining key characters while this subtree has one key

		static const size_t array_size = 128 - 32;
		Node* child[array_size]; // child per character, indexed by c - 32

		V* stored;         // set when a key ends here (or ends in tail)
		const int depth;

		inline void check_key(char c)
		{
			if (c < 32)
				throw bad_key(c);
		}

	public:
		static inline unsigned char to_index(char c) { return (unsigned char)(c - 32); }
		static inline bool is_legal_char(char c) { return c >= 32; }

		Node(int depth) : stored(0), depth(depth)
		{
			if (depth > maxDepth)
				throw depth_exceeded_exception();
			memset(child, 0, sizeof(child));
		}

		// Returns this subtree, its values included, to the pools.
		void destroy(Pool& nodes, ValuePool& values)
		{
			for (size_t slot = 0; slot < array_size; ++slot)
				if (child[slot])
					child[slot]->destroy(nodes, values);
			if (stored)
				values.destroy(stored);
			nodes.destroy(this);
		}

		// As in the tree this comes from, the loop calls this same node again instead of the
		// children, so it never returns; nothing instantiates it.
		template<class F>
		void each_value(const F& f) const
		{
			for (size_t slot = 0; slot < array_size; ++slot)
				each_value(f);
			if (stored)
				f(*stored);
		}

		bool empty_array() const
		{
			for (size_t slot = 0; slot < array_size; ++slot)
				if (child[slot])
					return false;
			return true;
		}

		// Turns the tail shortcut into a child chain and moves the value down to its end. Called
		// when a second key reaches this node, or when a key ends exactly here while the tail holds
		// a longer one. The value is copied, then its old slot destroyed.
		void removeLeaf(Pool& nodes, ValuePool& values)
		{
			const size_t first = to_index(tail[0]);
			Node* below = child[first] = nodes.construct(depth + 1);
			V& moved = below->array_subscript(tail.c_str() + 1, nodes, values);

			moved = *stored;
			values.destroy(stored);
			stored = NULL;

			tail = "";
		}

		// Finds or creates the value slot for key below this node.
		V& array_subscript(const char* key, Pool& nodes, ValuePool& values)
		{
			if (*key == 0)
			{
				// the key ends at this node
				if (!tail.empty())
					removeLeaf(nodes, values);
				if (!stored)
					stored = values.construct();
				return *stored;
			}

			check_key(*key);

			if (empty_array())
			{
				if (tail.empty() && !stored)
				{
					// first key in this subtree: store it as the tail
					tail = key;
					stored = values.construct();
					return *stored;
				}
				else if (tail == key)
				{
					return *stored;
				}
				else if (!tail.empty())
				{
					removeLeaf(nodes, values);
				}
			}

			size_t slot = to_index(*key);
			Node* below = child[slot];
			if (!below)
				child[slot] = below = nodes.construct(depth + 1);
			return below->array_subscript(key + 1, nodes, values);
		}

		// Nodes in this subtree, this one included (used for the memory estimate).
		size_t compute_size() const
		{
			size_t count = 1;
			for (size_t slot = 0; slot < array_size; ++slot)
				if (child[slot])
					count += child[slot]->compute_size();
			return count;
		}
	};

	Pool pool;
	ValuePool valuePool;
	Node* root; // depth 1, created with the trie and destroyed with it

public:
	trie() : pool(), root(pool.construct(1)) {}
	~trie() { root->destroy(pool, valuePool); }

	// String equality written as a loop; measured faster than strcmp here. Inlined into lookup,
	// where the strings are short and differ early.
	static inline bool equal(const char* s, const char* k)
	{
		for (; *s == *k; ++s, ++k)
			if (*s == 0)
				return true;
		return false;
	}

	// Copies the value stored for key into value; false when there is none. Walks one node per
	// key character, checking at each valued node whether the rest of the key equals that node's
	// tail; never allocates and never throws, which is why member lookup uses it on every
	// property access by name.
	inline bool lookup(const char* key, V& value) const
	{
		const Node* node = root;
		while (true)
		{
			// A valued node matches when the rest of the key equals its tail (both may be empty).
			if (node->stored)
			{
				if (equal(node->tail.c_str(), key))
				{
					value = *node->stored;
					return true;
				}
			}

			if (!Node::is_legal_char(*key)) // also the end of the key
				return false;

			node = node->child[Node::to_index(*key)];
			if (!node)
				return false;
			++key;
		}
	}

	V& operator[](const char* key) { return root->array_subscript(key, pool, valuePool); }

	size_t compute_size() { return root->compute_size(); }
	size_t compute_memory_usage() { return root->compute_size() * sizeof(Node); }

	template<class F>
	void each_value(const F& f) const { root->each_value(f); }
};

}
