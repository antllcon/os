#include "BTree.h"

#include "InternalNode.h"
#include "LeafNode.h"
#include "NodeWrappers.h"
#include "PageManager.h"
#include <algorithm>
#include <iostream>
#include <vector>

struct InternalEntry
{
	uint64_t ptr;
	uint64_t key;
};

BTree::BTree(const std::filesystem::path& path)
	: m_manager(path, !std::filesystem::exists(path))
{
}

std::optional<std::string> BTree::Get(uint64_t key)
{
	if (m_manager.GetHeader()->rootPage == NULL_PAGE)
	{
		return std::nullopt;
	}

	uint64_t leafId = FindLeaf(key);
	if (leafId == NULL_PAGE)
	{
		return std::nullopt;
	}

	LeafNode leaf(m_manager.GetPage(leafId));

	uint16_t count = leaf.GetCount();
	for (uint16_t i = 0; i < count; ++i)
	{
		const auto& record = leaf.GetRecord(i);

		if (record.key == key)
		{
			return std::string(record.value, record.len);
		}

		if (record.key > key)
		{
			break;
		}
	}

	return std::nullopt;
}

void BTree::Put(uint64_t key, const std::string& value)
{
	if (m_manager.GetHeader()->rootPage == NULL_PAGE)
	{
		CreateFirstRoot(key, value);
		return;
	}

	uint64_t leafId = FindLeaf(key);
	{
		LeafNode leaf(m_manager.GetPage<NodeHeader>(leafId));
		if (TryUpdateInLeaf(leaf, key, value))
		{
			return;
		}

		if (leaf.GetCount() < MAX_LEAF_KEYS)
		{
			InsertIntoLeafNoSplit(leaf, key, value);
			m_manager.GetHeader()->keysCount++;
			return;
		}
	}

	SplitLeafAndInsert(leafId, key, value);
	m_manager.GetHeader()->keysCount++;
}

void BTree::PrintStats() const
{
	TreeHeader* th = m_manager.GetHeader();
	std::cout << "--- B+ Tree Stats ---" << std::endl;
	std::cout << "Page Size: " << th->pageSize << std::endl;
	std::cout << "Height:    " << th->height << std::endl;
	std::cout << "Nodes:     " << th->nodesCount << std::endl;
	std::cout << "Keys:      " << th->keysCount << std::endl;
	std::cout << "Root Page: " << th->rootPage << std::endl;
	std::cout << "Next PID:  " << th->nextPid << std::endl;
}

void BTree::PrintStructure() const
{
	TreeHeader* header = m_manager.GetHeader();
	if (header->rootPage == NULL_PAGE)
	{
		std::cout << "Empty Tree" << std::endl;
		return;
	}

	std::cout << "ROOT [" << header->rootPage << "] (H=" << header->height << ")" << std::endl;
	PrintNodeRecursive(header->rootPage, "", true);
}

void BTree::PrintNodeRecursive(uint64_t nodeId, const std::string& prefix, bool isLast) const
{
	const NodeHeader* header = m_manager.GetPage<NodeHeader>(nodeId);
	BaseNode node(const_cast<NodeHeader*>(header));

	std::cout << prefix;
	std::cout << (isLast ? "L-" : "|-");

	if (node.IsInternal())
	{
		InternalNode inode(const_cast<NodeHeader*>(header));
		std::cout << "[INT " << nodeId << "] P:" << header->parent << " Keys: " << inode.GetCount() << std::endl;

		std::string childPrefix = prefix + (isLast ? "   " : "|  ");

		uint16_t count = inode.GetCount();
		for (uint16_t i = 0; i <= count; ++i)
		{
			uint64_t childId = inode.GetChild(i);
			PrintNodeRecursive(childId, childPrefix, i == count);
		}
	}
	else
	{
		LeafNode leaf(const_cast<NodeHeader*>(header));
		std::cout << "[LEAF " << nodeId << "] P:" << header->parent
				  << " Next:" << leaf.GetNext() << " Prev:" << leaf.GetPrev() << std::endl;

		std::string childPrefix = prefix + (isLast ? "   " : "|  ");
		uint16_t count = leaf.GetCount();

		for (uint16_t i = 0; i < count; ++i)
		{
			const auto& rec = leaf.GetRecord(i);

			std::string val(rec.value, rec.len);
			if (val.length() > MAX_PRINT_VAL_LEN)
			{
				val = val.substr(0, MAX_PRINT_VAL_LEN) + "..";
			}

			std::cout << childPrefix;
			std::cout << (i == count - 1 ? "L-" : "|-");
			std::cout << "[" << rec.key << "] " << val << std::endl;
		}
	}
}

void BTree::CreateFirstRoot(uint64_t key, const std::string& value)
{
	uint64_t rootId = m_manager.AllocatePage();

	NodeHeader* header = m_manager.GetPage<NodeHeader>(rootId);
	header->type = NodeType::Leaf;
	header->numKeys = 0;
	header->parent = NULL_PAGE;
	header->nextLeaf = NULL_PAGE;
	header->prevLeaf = NULL_PAGE;

	LeafNode leaf(header);
	leaf.InsertAt(0, key, value);

	TreeHeader* treeHeader = m_manager.GetHeader();
	treeHeader->rootPage = rootId;
	treeHeader->height = 1;
	treeHeader->keysCount = 1;
}

uint64_t BTree::FindLeaf(uint64_t key) const
{
	uint64_t currentPageId = m_manager.GetHeader()->rootPage;

	if (currentPageId == NULL_PAGE)
	{
		return NULL_PAGE;
	}

	while (true)
	{
		const NodeHeader* header = m_manager.GetPage<NodeHeader>(currentPageId);
		BaseNode node(const_cast<NodeHeader*>(header));

		if (node.IsLeaf())
		{
			return currentPageId;
		}

		InternalNode internal(const_cast<NodeHeader*>(header));
		uint16_t count = internal.GetCount();
		uint16_t index = 0;

		while (index < count && key >= internal.GetKey(index))
		{
			index++;
		}

		currentPageId = internal.GetChild(index);
	}
}

bool BTree::TryUpdateInLeaf(LeafNode& leaf, uint64_t key, const std::string& value)
{
	uint16_t count = leaf.GetCount();
	for (uint16_t i = 0; i < count; ++i)
	{
		if (leaf.GetRecord(i).key == key)
		{
			leaf.RemoveAt(i);
			leaf.InsertAt(i, key, value);
			return true;
		}
	}

	return false;
}

void BTree::InsertIntoLeafNoSplit(LeafNode& leaf, uint64_t key, const std::string& value)
{
	uint16_t count = leaf.GetCount();
	uint16_t pos = 0;

	while (pos < count && leaf.GetRecord(pos).key < key)
	{
		pos++;
	}

	leaf.InsertAt(pos, key, value);
}

void BTree::SplitLeafAndInsert(uint64_t leafId, uint64_t key, const std::string_view& value)
{
	auto buffer = CollectLeafEntries(leafId, key, value);

	uint64_t newLeafId = m_manager.AllocatePage();
	m_manager.GetHeader()->nodesCount++;

	DistributeLeafEntries(buffer, leafId, newLeafId);

	LinkLeafNodes(leafId, newLeafId);

	size_t splitPoint = buffer.size() / 2;
	uint64_t separatorKey = buffer[splitPoint].key;

	InsertParent(leafId, separatorKey, newLeafId);
}

std::vector<LeafRecord> BTree::CollectLeafEntries(uint64_t leafId, uint64_t newKey, const std::string_view& newValue)
{
	std::vector<LeafRecord> buffer;
	buffer.reserve(MAX_LEAF_KEYS + 1);

	NodeHeader* header = m_manager.GetPage<NodeHeader>(leafId);
	LeafNode leaf(header);
	uint16_t count = leaf.GetCount();

	for (uint16_t i = 0; i < count; ++i)
	{
		buffer.push_back(leaf.GetRecord(i));
	}

	LeafRecord newRecord{};
	newRecord.key = newKey;
	newRecord.len = static_cast<uint8_t>(std::min(newValue.size(), MAX_VALUE_LENGTH));
	std::memcpy(newRecord.value, newValue.data(), newRecord.len);

	if (newRecord.len < MAX_VALUE_LENGTH)
	{
		std::memset(newRecord.value + newRecord.len, 0, MAX_VALUE_LENGTH - newRecord.len);
	}

	auto it = std::lower_bound(buffer.begin(), buffer.end(), newKey, [](const LeafRecord& r, uint64_t k) { return r.key < k; });
	buffer.insert(it, newRecord);
	return buffer;
}

void BTree::DistributeLeafEntries(const std::vector<LeafRecord>& buffer, uint64_t oldLeafId, uint64_t newLeafId)
{
	LeafNode oldLeaf(m_manager.GetPage<NodeHeader>(oldLeafId));

	NodeHeader* newHeaderRaw = m_manager.GetPage<NodeHeader>(newLeafId);
	newHeaderRaw->type = NodeType::Leaf;
	newHeaderRaw->numKeys = 0;
	newHeaderRaw->parent = oldLeaf.GetParent();
	LeafNode newLeaf(newHeaderRaw);

	size_t total = buffer.size();
	size_t splitPoint = total / 2;

	oldLeaf.SetCount(0);
	for (size_t i = 0; i < splitPoint; ++i)
	{
		std::string_view val(buffer[i].value, buffer[i].len);
		oldLeaf.InsertAt(static_cast<uint16_t>(i), buffer[i].key, val);
	}

	for (size_t i = splitPoint; i < total; ++i)
	{
		std::string_view val(buffer[i].value, buffer[i].len);
		newLeaf.InsertAt(static_cast<uint16_t>(i - splitPoint), buffer[i].key, val);
	}
}

void BTree::LinkLeafNodes(uint64_t oldLeafId, uint64_t newLeafId)
{
	LeafNode oldLeaf(m_manager.GetPage<NodeHeader>(oldLeafId));
	LeafNode newLeaf(m_manager.GetPage<NodeHeader>(newLeafId));

	uint64_t nextLeafId = oldLeaf.GetNext();

	oldLeaf.SetNext(newLeafId);
	newLeaf.SetPrev(oldLeafId);
	newLeaf.SetNext(nextLeafId);

	if (nextLeafId != NULL_PAGE)
	{
		LeafNode nextNode(m_manager.GetPage<NodeHeader>(nextLeafId));
		nextNode.SetPrev(newLeafId);
	}
}

void BTree::InsertParent(uint64_t leftChildId, uint64_t key, uint64_t rightChildId)
{
	uint64_t parentId;
	{
		BaseNode leftNode(m_manager.GetPage<NodeHeader>(leftChildId));
		parentId = leftNode.GetParent();
	}

	if (parentId == NULL_PAGE)
	{
		CreateNewRootFromSplit(leftChildId, key, rightChildId);
		return;
	}

	bool needSplit = false;
	{
		InternalNode parent(m_manager.GetPage<NodeHeader>(parentId));
		if (parent.GetCount() < MAX_INTERNAL_KEYS)
		{
			uint16_t pos = 0;
			uint16_t count = parent.GetCount();
			bool found = false;

			if (parent.GetChild(0) == leftChildId)
			{
				pos = 0;
				found = true;
			}
			else
			{
				for (uint16_t i = 1; i <= count; ++i)
				{
					if (parent.GetChild(i) == leftChildId)
					{
						pos = i;
						found = true;
						break;
					}
				}
			}

			if (!found)
			{
				while (pos < count && parent.GetKey(pos) < key)
				{
					pos++;
				}
			}

			parent.InsertAt(pos, key, rightChildId);
			BaseNode child(m_manager.GetPage<NodeHeader>(rightChildId));
			child.SetParent(parentId);
		}
		else
		{
			needSplit = true;
		}
	}

	if (needSplit)
	{
		SplitInternalAndInsert(parentId, key, rightChildId);
	}
}

void BTree::CreateNewRootFromSplit(uint64_t leftChildId, uint64_t key, uint64_t rightChildId)
{
	uint64_t newRootId = m_manager.AllocatePage();
	TreeHeader* header = m_manager.GetHeader();

	header->rootPage = newRootId;
	header->nodesCount++;
	header->height++;

	NodeHeader* rootRaw = m_manager.GetPage<NodeHeader>(newRootId);
	rootRaw->type = NodeType::Internal;
	rootRaw->numKeys = 0;
	rootRaw->parent = NULL_PAGE;

	InternalNode root(rootRaw);
	root.SetChild(0, leftChildId);
	root.InsertAt(0, key, rightChildId);

	BaseNode left(m_manager.GetPage<NodeHeader>(leftChildId));
	left.SetParent(newRootId);

	BaseNode right(m_manager.GetPage<NodeHeader>(rightChildId));
	right.SetParent(newRootId);
}

void BTree::SplitInternalAndInsert(uint64_t nodeId, uint64_t key, uint64_t rightChildId)
{
	InternalSplitData data = PrepareInternalSplit(nodeId, key, rightChildId);
	uint64_t newNodeId = m_manager.AllocatePage();
	m_manager.GetHeader()->nodesCount++;

	size_t totalKeys = data.keys.size();
	size_t midIndex = totalKeys / 2;
	uint64_t promotedKey = data.keys[midIndex];

	DistributeInternalData(nodeId, newNodeId, data, midIndex);

	InsertParent(nodeId, promotedKey, newNodeId);
}

BTree::InternalSplitData BTree::PrepareInternalSplit(uint64_t nodeId, uint64_t key, uint64_t rightChildId)
{
	InternalSplitData data;

	{
		InternalNode node(m_manager.GetPage<NodeHeader>(nodeId));
		uint16_t count = node.GetCount();

		for (uint16_t i = 0; i < count; ++i)
		{
			data.keys.push_back(node.GetKey(i));
		}

		for (uint16_t i = 0; i <= count; ++i)
		{
			data.children.push_back(node.GetChild(i));
		}
	}

	auto it = std::ranges::lower_bound(data.keys, key);
	size_t pos = std::distance(data.keys.begin(), it);

	data.keys.insert(data.keys.begin() + pos, key);
	data.children.insert(data.children.begin() + pos + 1, rightChildId);

	return data;
}

void BTree::DistributeInternalData(uint64_t oldNodeId, uint64_t newNodeId, const InternalSplitData& data, size_t midIndex)
{
	InternalNode oldNode(m_manager.GetPage<NodeHeader>(oldNodeId));


	NodeHeader* newHeaderRaw = m_manager.GetPage<NodeHeader>(newNodeId);
	newHeaderRaw->type = NodeType::Internal;
	newHeaderRaw->numKeys = 0;
	newHeaderRaw->parent = oldNode.GetParent();
	InternalNode newNode(newHeaderRaw);

	oldNode.SetCount(0);
	oldNode.SetChild(0, data.children[0]);
	BaseNode(m_manager.GetPage<NodeHeader>(data.children[0])).SetParent(oldNodeId);

	for (size_t i = 0; i < midIndex; ++i)
	{
		oldNode.InsertAt(static_cast<uint16_t>(i), data.keys[i], data.children[i + 1]);
		BaseNode(m_manager.GetPage<NodeHeader>(data.children[i + 1])).SetParent(oldNodeId);
	}

	newNode.SetChild(0, data.children[midIndex + 1]);
	BaseNode(m_manager.GetPage<NodeHeader>(data.children[midIndex + 1])).SetParent(newNodeId);

	size_t totalKeys = data.keys.size();
	for (size_t i = midIndex + 1; i < totalKeys; ++i)
	{
		uint16_t newIndex = static_cast<uint16_t>(i - (midIndex + 1));
		newNode.InsertAt(newIndex, data.keys[i], data.children[i + 1]);
		BaseNode(m_manager.GetPage<NodeHeader>(data.children[i + 1])).SetParent(newNodeId);
	}
}

bool BTree::Remove(uint64_t key)
{
	TreeHeader* header = m_manager.GetHeader();
	if (header->rootPage == NULL_PAGE)
	{
		return false;
	}

	uint64_t leafId = FindLeaf(key);
	return RemoveFromLeaf(leafId, key);
}

bool BTree::RemoveFromLeaf(uint64_t leafId, uint64_t key)
{
	LeafNode leaf(m_manager.GetPage<NodeHeader>(leafId));
	uint16_t count = leaf.GetCount();
	bool found = false;
	uint16_t index = 0;

	for (uint16_t i = 0; i < count; ++i)
	{
		if (leaf.GetRecord(i).key == key)
		{
			index = i;
			found = true;
			break;
		}
	}

	if (!found)
	{
		return false;
	}

	leaf.RemoveAt(index);
	m_manager.GetHeader()->keysCount--;

	if (leafId == m_manager.GetHeader()->rootPage)
	{
		if (leaf.GetCount() == 0)
		{
			m_manager.FreePage(leafId);
			TreeHeader* th = m_manager.GetHeader();
			th->rootPage = NULL_PAGE;
			th->height = 0;
			th->nodesCount = 0;
		}
		return true;
	}

	if (leaf.GetCount() < MIN_LEAF_KEYS)
	{
		HandleLeafUnderflow(leafId);
	}

	return true;
}

void BTree::HandleLeafUnderflow(uint64_t leafId)
{
	LeafNode leaf(m_manager.GetPage<NodeHeader>(leafId));
	SiblingInfo info = GetSiblingInfo(leafId, leaf);

	if (info.leftSibling != NULL_PAGE)
	{
		LeafNode left(m_manager.GetPage<NodeHeader>(info.leftSibling));
		if (left.GetCount() > MIN_LEAF_KEYS)
		{
			BorrowFromLeftLeaf(leafId, info.leftSibling, info.parentId, info.indexInParent);
			return;
		}
	}

	if (info.rightSibling != NULL_PAGE)
	{
		LeafNode right(m_manager.GetPage<NodeHeader>(info.rightSibling));
		if (right.GetCount() > MIN_LEAF_KEYS)
		{
			BorrowFromRightLeaf(leafId, info.rightSibling, info.parentId, info.indexInParent);
			return;
		}
	}

	if (info.leftSibling != NULL_PAGE)
	{
		MergeLeaves(info.leftSibling, leafId, info.parentId, info.indexInParent - 1);
	}
	else if (info.rightSibling != NULL_PAGE)
	{
		MergeLeaves(leafId, info.rightSibling, info.parentId, info.indexInParent);
	}
}

void BTree::BorrowFromLeftLeaf(uint64_t leafId, uint64_t leftId, uint64_t parentId, uint16_t idxInParent)
{
	LeafNode leaf(m_manager.GetPage<NodeHeader>(leafId));
	LeafNode left(m_manager.GetPage<NodeHeader>(leftId));
	InternalNode parent(m_manager.GetPage<NodeHeader>(parentId));

	uint16_t leftCount = left.GetCount();
	LeafRecord borrowed = left.GetRecord(leftCount - 1);
	left.RemoveAt(leftCount - 1);

	std::string_view val(borrowed.value, borrowed.len);
	leaf.InsertAt(0, borrowed.key, val);

	parent.SetKey(idxInParent - 1, borrowed.key);
}

void BTree::BorrowFromRightLeaf(uint64_t leafId, uint64_t rightId, uint64_t parentId, uint16_t idxInParent)
{
	LeafNode leaf(m_manager.GetPage<NodeHeader>(leafId));
	LeafNode right(m_manager.GetPage<NodeHeader>(rightId));
	InternalNode parent(m_manager.GetPage<NodeHeader>(parentId));

	LeafRecord borrowed = right.GetRecord(0);
	right.RemoveAt(0);

	std::string_view val(borrowed.value, borrowed.len);
	leaf.InsertAt(leaf.GetCount(), borrowed.key, val);

	uint64_t newSeparator = right.GetRecord(0).key;
	parent.SetKey(idxInParent, newSeparator);
}

void BTree::MergeLeaves(uint64_t leftId, uint64_t rightId, uint64_t parentId, uint16_t idxInParent)
{
	LeafNode left(m_manager.GetPage<NodeHeader>(leftId));
	LeafNode right(m_manager.GetPage<NodeHeader>(rightId));

	uint16_t rightCount = right.GetCount();
	for (uint16_t i = 0; i < rightCount; ++i)
	{
		const auto& rec = right.GetRecord(i);
		std::string_view val(rec.value, rec.len);
		left.InsertAt(left.GetCount(), rec.key, val);
	}

	left.SetNext(right.GetNext());
	if (right.GetNext() != NULL_PAGE)
	{
		LeafNode(m_manager.GetPage<NodeHeader>(right.GetNext())).SetPrev(leftId);
	}

	m_manager.FreePage(rightId);
	m_manager.GetHeader()->nodesCount--;

	RemoveFromInternal(parentId, idxInParent);
}

void BTree::RemoveFromInternal(uint64_t nodeId, uint16_t index)
{
	InternalNode node(m_manager.GetPage<NodeHeader>(nodeId));

	node.RemoveAt(index);

	if (nodeId == m_manager.GetHeader()->rootPage)
	{
		AdjustRoot();
		return;
	}

	if (node.GetCount() < MIN_INTERNAL_KEYS)
	{
		HandleInternalUnderflow(nodeId);
	}
}

void BTree::HandleInternalUnderflow(uint64_t nodeId)
{
	InternalNode node(m_manager.GetPage<NodeHeader>(nodeId));
	SiblingInfo info = GetSiblingInfo(nodeId, node);

	if (info.leftSibling != NULL_PAGE)
	{
		InternalNode left(m_manager.GetPage<NodeHeader>(info.leftSibling));
		if (left.GetCount() > MIN_INTERNAL_KEYS)
		{
			BorrowFromLeftInternal(nodeId, info.leftSibling, info.parentId, info.indexInParent);
			return;
		}
	}

	if (info.rightSibling != NULL_PAGE)
	{
		InternalNode right(m_manager.GetPage<NodeHeader>(info.rightSibling));
		if (right.GetCount() > MIN_INTERNAL_KEYS)
		{
			BorrowFromRightInternal(nodeId, info.rightSibling, info.parentId, info.indexInParent);
			return;
		}
	}

	if (info.leftSibling != NULL_PAGE)
	{
		MergeInternalNodes(info.leftSibling, nodeId, info.parentId, info.indexInParent - 1);
	}
	else if (info.rightSibling != NULL_PAGE)
	{
		MergeInternalNodes(nodeId, info.rightSibling, info.parentId, info.indexInParent);
	}
}

void BTree::BorrowFromLeftInternal(uint64_t nodeId, uint64_t leftId, uint64_t parentId, uint16_t idxInParent)
{
	InternalNode cur(m_manager.GetPage<NodeHeader>(nodeId));
	InternalNode left(m_manager.GetPage<NodeHeader>(leftId));
	InternalNode parent(m_manager.GetPage<NodeHeader>(parentId));

	uint64_t parentKeyIdx = idxInParent - 1;
	uint64_t keyFromParent = parent.GetKey(parentKeyIdx);

	uint16_t leftLastIdx = left.GetCount() - 1;
	uint64_t keyFromLeft = left.GetKey(leftLastIdx);
	uint64_t childFromLeft = left.GetChild(leftLastIdx + 1);

	left.RemoveAt(leftLastIdx);

	uint64_t oldP0 = cur.GetChild(0);
	cur.InsertAt(0, keyFromParent, oldP0);
	cur.SetChild(0, childFromLeft);
	BaseNode(m_manager.GetPage<NodeHeader>(childFromLeft)).SetParent(nodeId);

	parent.SetKey(parentKeyIdx, keyFromLeft);
}

void BTree::BorrowFromRightInternal(uint64_t nodeId, uint64_t rightId, uint64_t parentId, uint16_t idxInParent)
{
	InternalNode cur(m_manager.GetPage<NodeHeader>(nodeId));
	InternalNode right(m_manager.GetPage<NodeHeader>(rightId));
	InternalNode parent(m_manager.GetPage<NodeHeader>(parentId));

	uint64_t parentKeyIdx = idxInParent;
	uint64_t keyFromParent = parent.GetKey(parentKeyIdx);

	uint64_t childFromRight = right.GetChild(0);
	uint64_t keyFromRight = right.GetKey(0);

	cur.InsertAt(cur.GetCount(), keyFromParent, childFromRight);
	BaseNode(m_manager.GetPage<NodeHeader>(childFromRight)).SetParent(nodeId);

	uint64_t p1Right = right.GetChild(1);

	parent.SetKey(parentKeyIdx, keyFromRight);

	right.RemoveAt(0);
	right.SetChild(0, p1Right);
}

void BTree::MergeInternalNodes(uint64_t leftId, uint64_t rightId, uint64_t parentId, uint16_t idxInParent)
{
	InternalNode left(m_manager.GetPage<NodeHeader>(leftId));
	InternalNode right(m_manager.GetPage<NodeHeader>(rightId));
	InternalNode parent(m_manager.GetPage<NodeHeader>(parentId));

	uint64_t keyFromParent = parent.GetKey(idxInParent);

	uint64_t rightP0 = right.GetChild(0);
	left.InsertAt(left.GetCount(), keyFromParent, rightP0);
	BaseNode(m_manager.GetPage<NodeHeader>(rightP0)).SetParent(leftId);

	uint16_t rightCount = right.GetCount();
	for (uint16_t i = 0; i < rightCount; ++i)
	{
		left.InsertAt(left.GetCount(), right.GetKey(i), right.GetChild(i + 1));
		BaseNode(m_manager.GetPage<NodeHeader>(right.GetChild(i + 1))).SetParent(leftId);
	}

	m_manager.FreePage(rightId);
	m_manager.GetHeader()->nodesCount--;

	RemoveFromInternal(parentId, idxInParent);
}

void BTree::AdjustRoot()
{
	TreeHeader* header = m_manager.GetHeader();
	uint64_t rootId = header->rootPage;

	InternalNode root(m_manager.GetPage<NodeHeader>(rootId));

	if (root.GetCount() == 0)
	{
		uint64_t newRootId = root.GetChild(0);

		header->rootPage = newRootId;
		header->height--;

		BaseNode(m_manager.GetPage<NodeHeader>(newRootId)).SetParent(NULL_PAGE);
		m_manager.FreePage(rootId);
		header->nodesCount--;
	}
}

BTree::SiblingInfo BTree::GetSiblingInfo(uint64_t nodeId, const BaseNode& node)
{
	SiblingInfo info{NULL_PAGE, 0, NULL_PAGE, NULL_PAGE};
	info.parentId = node.GetParent();

	if (info.parentId == NULL_PAGE) return info;

	InternalNode parent(m_manager.GetPage<NodeHeader>(info.parentId));
	uint16_t pCount = parent.GetCount();

	bool found = false;
	for (uint16_t i = 0; i <= pCount; ++i)
	{
		if (parent.GetChild(i) == nodeId)
		{
			info.indexInParent = i;
			found = true;
			break;
		}
	}

	if (!found)
	{
		return info;
	}

	if (info.indexInParent > 0)
	{
		info.leftSibling = parent.GetChild(info.indexInParent - 1);
	}

	if (info.indexInParent < pCount)
	{
		info.rightSibling = parent.GetChild(info.indexInParent + 1);
	}

	return info;
}