#pragma once

#include "PageManager.h"
#include <filesystem>
#include <optional>
#include <string>

class BaseNode;
class LeafNode;
class BTree
{
public:
	explicit BTree(const std::filesystem::path& path);

	BTree(const BTree&) = delete;
	BTree& operator=(const BTree&) = delete;

	std::optional<std::string> Get(uint64_t key);
	void Put(uint64_t key, const std::string& value);
	bool Remove(uint64_t key);

	void PrintStats() const;

private:
	void CreateFirstRoot(uint64_t key, const std::string& value);
	uint64_t FindLeaf(uint64_t key) const;
	void InsertParent(uint64_t leftChild, uint64_t key, uint64_t rightChild);
	void SplitLeafAndInsert(uint64_t leafId, uint64_t key, const std::string_view& value);
	void SplitInternalAndInsert(uint64_t nodeId, uint64_t key, uint64_t rightChildId);

	static bool TryUpdateInLeaf(LeafNode& leaf, uint64_t key, const std::string& value);
	static void InsertIntoLeafNoSplit(LeafNode& leaf, uint64_t key, const std::string& value);

	std::vector<LeafRecord> CollectLeafEntries(uint64_t leafId, uint64_t newKey, const std::string_view& newValue);
	void DistributeLeafEntries(const std::vector<LeafRecord>& buffer, uint64_t oldLeafId, uint64_t newLeafId);
	void LinkLeafNodes(uint64_t oldLeafId, uint64_t newLeafId);

	struct InternalSplitData
	{
		std::vector<uint64_t> keys;
		std::vector<uint64_t> children;
	};

	InternalSplitData PrepareInternalSplit(uint64_t nodeId, uint64_t key, uint64_t rightChildId);
	void DistributeInternalData(uint64_t oldNodeId, uint64_t newNodeId, const InternalSplitData& data, size_t midIndex);
	void CreateNewRootFromSplit(uint64_t leftChildId, uint64_t key, uint64_t rightChildId);

	bool RemoveFromLeaf(uint64_t leafId, uint64_t key);
	void HandleLeafUnderflow(uint64_t leafId);
	void HandleInternalUnderflow(uint64_t nodeId);
	void RemoveFromInternal(uint64_t nodeId, uint16_t index);

	struct SiblingInfo
	{
		uint64_t parentId;
		uint16_t indexInParent;
		uint64_t leftSibling;
		uint64_t rightSibling;
	};

	SiblingInfo GetSiblingInfo(uint64_t nodeId, const BaseNode& node);

	void BorrowFromLeftLeaf(uint64_t leafId, uint64_t leftId, uint64_t parentId, uint16_t idxInParent);
	void BorrowFromRightLeaf(uint64_t leafId, uint64_t rightId, uint64_t parentId, uint16_t idxInParent);
	void MergeLeaves(uint64_t leftId, uint64_t rightId, uint64_t parentId, uint16_t idxInParent);

	void BorrowFromLeftInternal(uint64_t nodeId, uint64_t leftId, uint64_t parentId, uint16_t idxInParent);
	void BorrowFromRightInternal(uint64_t nodeId, uint64_t rightId, uint64_t parentId, uint16_t idxInParent);
	void MergeInternalNodes(uint64_t leftId, uint64_t rightId, uint64_t parentId, uint16_t idxInParent);

	void AdjustRoot();

	PageManager m_manager;
};