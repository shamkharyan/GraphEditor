#ifndef DISJOINT_SET_H
#define DISJOINT_SET_H

#include <unordered_map>
#include <vector>
#include <stdexcept>

template <typename T>
class DisjointSet
{
public:
    int makeSet(const T& value);
    bool unionSets(const T& val1, const T& val2);
    bool isSameSet(const T& val1, const T& val2);
    int sizeOfSet(const T& value);
    int findSet(const T& value);
    void clear();

private:
    int findId(int id);
    bool uniteId(int id1, int id2);

private:
    std::unordered_map<T, int> m_ids;
    std::vector<int> m_parent;
};

template <typename T>
int DisjointSet<T>::makeSet(const T& value)
{
    auto it = m_ids.find(value);
    if (it != m_ids.end())
        return it->second;

    m_ids[value] = static_cast<int>(m_parent.size());
    m_parent.push_back(-1);
    return m_ids[value];
}

template <typename T>
int DisjointSet<T>::findSet(const T& value)
{
    auto it = m_ids.find(value);
    if (it == m_ids.end())
        throw std::out_of_range("Disjoint Set: Element not found");

    return findId(it->second);
}

template <typename T>
bool DisjointSet<T>::unionSets(const T& val1, const T& val2)
{
    auto it1 = m_ids.find(val1);
    auto it2 = m_ids.find(val2);

    if (it1 == m_ids.end() || it2 == m_ids.end())
        throw std::out_of_range("Disjoint Set: Element not found");

    return uniteId(it1->second, it2->second);
}

template <typename T>
bool DisjointSet<T>::isSameSet(const T& val1, const T& val2)
{
    auto it1 = m_ids.find(val1);
    auto it2 = m_ids.find(val2);

    if (it1 == m_ids.end() || it2 == m_ids.end())
        return false;

    return findId(it1->second) == findId(it2->second);
}

template <typename T>
int DisjointSet<T>::sizeOfSet(const T& value)
{
    auto it = m_ids.find(value);
    if (it == m_ids.end())
        throw std::out_of_range("Disjoint Set: Element not found");

    int root = findId(it->second);
    return -m_parent[root];
}

template <typename T>
void DisjointSet<T>::clear()
{
    m_parent.clear();
    m_ids.clear();
}

template <typename T>
int DisjointSet<T>::findId(int id)
{
    if (id < 0 || id >= static_cast<int>(m_parent.size()))
        throw std::out_of_range("Disjoint Set: Index out of range");

    if (m_parent[id] < 0)
        return id;

    return m_parent[id] = findId(m_parent[id]);
}

template <typename T>
bool DisjointSet<T>::uniteId(int id1, int id2)
{
    id1 = findId(id1);
    id2 = findId(id2);

    if (id1 == id2)
        return false;

    if (m_parent[id1] > m_parent[id2])
        std::swap(id1, id2);

    m_parent[id1] += m_parent[id2];
    m_parent[id2] = id1;

    return true;
}

#endif // DISJOINT_SET_H
