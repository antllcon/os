#include "BTree/BTree.h"
#include "gtest/gtest.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <numeric>
#include <random>
#include <string>
#include <vector>

namespace fs = std::filesystem;

class BTreeTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		std::string filename = "test_db_" + std::to_string(GetRandomId()) + ".bin";
		m_dbPath = fs::temp_directory_path() / filename;

		if (fs::exists(m_dbPath))
		{
			fs::remove(m_dbPath);
		}
	}

	void TearDown() override
	{
		if (fs::exists(m_dbPath))
		{
			fs::remove(m_dbPath);
		}
	}

	uint64_t GetRandomId()
	{
		static std::mt19937_64 rng(std::random_device{}());
		return rng();
	}

	fs::path m_dbPath;
};

// Вставка и получение
TEST_F(BTreeTest, BasicPutAndGet)
{
	{
		BTree tree(m_dbPath);
		tree.Put(1, "Value 1");
		tree.Put(2, "Value 2");

		auto val1 = tree.Get(1);
		auto val2 = tree.Get(2);
		auto val3 = tree.Get(3);

		ASSERT_TRUE(val1.has_value());
		EXPECT_EQ(*val1, "Value 1");

		ASSERT_TRUE(val2.has_value());
		EXPECT_EQ(*val2, "Value 2");

		EXPECT_FALSE(val3.has_value());
	}
}

// Обновление существующего ключа
TEST_F(BTreeTest, UpdateExistingKey)
{
	{
		BTree tree(m_dbPath);
		tree.Put(100, "Old Value");
		auto val = tree.Get(100);
		ASSERT_TRUE(val.has_value());
		EXPECT_EQ(*val, "Old Value");

		// Обновляем
		tree.Put(100, "New Value");
		val = tree.Get(100);
		ASSERT_TRUE(val.has_value());
		EXPECT_EQ(*val, "New Value");
	}
}

// Удаление ключей
TEST_F(BTreeTest, BasicRemove)
{
	{
		BTree tree(m_dbPath);
		tree.Put(10, "Ten");
		tree.Put(20, "Twenty");

		EXPECT_TRUE(tree.Remove(10));
		EXPECT_FALSE(tree.Get(10).has_value());

		// Удаление несуществующего ключа
		EXPECT_FALSE(tree.Remove(999));

		// Второй ключ должен остаться
		EXPECT_EQ(tree.Get(20).value_or(""), "Twenty");
	}
}

// Сохранение на диске
TEST_F(BTreeTest, DataPersistsAfterReopen)
{
	{
		BTree tree(m_dbPath);
		tree.Put(42, "Universe");
		tree.Put(1, "One");
	}

	{
		BTree tree(m_dbPath);
		auto val = tree.Get(42);
		ASSERT_TRUE(val.has_value());
		EXPECT_EQ(*val, "Universe");

		auto val2 = tree.Get(1);
		ASSERT_TRUE(val2.has_value());
		EXPECT_EQ(*val2, "One");
	}
}

// Тест на максимальную длину строки
TEST_F(BTreeTest, MaxLengthValue)
{
	std::string longStr(119, 'A');
	{
		BTree tree(m_dbPath);
		tree.Put(5, longStr);

		auto val = tree.Get(5);
		ASSERT_TRUE(val.has_value());
		EXPECT_EQ(*val, longStr);
	}
}

// 6. Тест расщепления листа (Leaf Split)
// Максимум записей в листе = 31. Вставляем 32+ элемента
TEST_F(BTreeTest, LeafSplit)
{
	const int count = 50;
	{
		BTree tree(m_dbPath);
		for (int i = 0; i < count; ++i)
		{
			tree.Put(i, "Val " + std::to_string(i));
		}

		// Проверяем все
		for (int i = 0; i < count; ++i)
		{
			auto val = tree.Get(i);
			ASSERT_TRUE(val.has_value()) << "Key " << i << " lost after split";
			EXPECT_EQ(*val, "Val " + std::to_string(i));
		}
	}
}

// 7. Тест расщепления внутренних узлов (Internal Split + Height Growth)
// Нужно вставить достаточно элементов, чтобы заполнить корневой лист,
// расщепить его, заполнить новый корень (internal node) и расщепить его.
// Internal Order ~ 250. Leaf Order ~ 31.
// 250 * 31 = 7750. Вставка 10 000 элементов должна гарантированно увеличить высоту.
TEST_F(BTreeTest, LargeScaleInsert)
{
	const int count = 10000;
	{
		BTree tree(m_dbPath);
		for (int i = 0; i < count; ++i)
		{
			tree.Put(i, std::to_string(i));
		}

		// Выборочная проверка
		EXPECT_EQ(tree.Get(0).value_or(""), "0");
		EXPECT_EQ(tree.Get(count / 2).value_or(""), std::to_string(count / 2));
		EXPECT_EQ(tree.Get(count - 1).value_or(""), std::to_string(count - 1));
	}
}

// 8. Стресс-тест: Случайная вставка и удаление
TEST_F(BTreeTest, RandomInsertDelete)
{
	const int count = 2000;
	std::vector<uint64_t> keys(count);
	std::iota(keys.begin(), keys.end(), 0);

	std::mt19937 g(std::random_device{}());
	std::shuffle(keys.begin(), keys.end(), g);

	{
		BTree tree(m_dbPath);

		// 1. Вставка в случайном порядке
		for (uint64_t key : keys)
		{
			tree.Put(key, "V" + std::to_string(key));
		}

		// 2. Удаляем первую половину ключей (из перемешанного массива)
		size_t deleteCount = count / 2;
		for (size_t i = 0; i < deleteCount; ++i)
		{
			bool removed = tree.Remove(keys[i]);
			ASSERT_TRUE(removed) << "Failed to remove key: " << keys[i];
		}

		// 3. Проверка
		for (size_t i = 0; i < count; ++i)
		{
			uint64_t key = keys[i];
			auto val = tree.Get(key);

			if (i < deleteCount)
			{
				// Должен быть удален
				EXPECT_FALSE(val.has_value()) << "Key " << key << " should be deleted";
			}
			else
			{
				// Должен существовать
				ASSERT_TRUE(val.has_value()) << "Key " << key << " should exist";
				EXPECT_EQ(*val, "V" + std::to_string(key));
			}
		}
	}
}

// 9. Переоткрытие файла и добавление данных (расширение)
TEST_F(BTreeTest, ReopenAndAppend)
{
	// Шаг 1: Создаем дерево и пишем данные
	{
		BTree tree(m_dbPath);
		for (int i = 0; i < 100; ++i)
			tree.Put(i, "Original");
	}

	// Шаг 2: Открываем и добавляем еще
	{
		BTree tree(m_dbPath);
		// Проверяем старое
		ASSERT_EQ(tree.Get(50).value_or(""), "Original");

		// Пишем новое
		for (int i = 100; i < 200; ++i)
			tree.Put(i, "New");
	}

	// Шаг 3: Проверяем всё вместе
	{
		BTree tree(m_dbPath);
		EXPECT_EQ(tree.Get(0).value_or(""), "Original");
		EXPECT_EQ(tree.Get(199).value_or(""), "New");
	}
}