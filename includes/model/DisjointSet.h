#ifndef DISJOINT_SET_H
#define DISJOINT_SET_H

#include <vector>
#include <numeric>
#include <algorithm>

class DisjointSet
{
public:
    // Инициализируем сразу на N элементов
    explicit DisjointSet(size_t n) {
        m_parent.assign(n, -1); 
    }

    // Максимально быстрый поиск с рекурсивным сжатием пути
    int find(int i) {
        if (m_parent[i] < 0)
            return i;
        return m_parent[i] = find(m_parent[i]);
    }

    // Объединение по размеру (union by size)
    bool unite(int i, int j) {
        int root_i = find(i);
        int root_j = find(j);

        if (root_i != root_j) {
            // Подвешиваем меньшее дерево к большему
            if (m_parent[root_i] > m_parent[root_j]) 
                std::swap(root_i, root_j);

            m_parent[root_i] += m_parent[root_j]; // Увеличиваем размер корня i
            m_parent[root_j] = root_i;           // Делаем i родителем j
            return true;
        }
        return false;
    }

private:
    // Отрицательное число: корень, значение - размер множества
    // Положительное число: индекс родителя
    std::vector<int> m_parent;
};

#endif
