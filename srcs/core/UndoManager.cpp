#include "core/UndoManager.h"

void UndoManager::append(std::unique_ptr<AAction> action)
{
    m_undo.push_back(std::move(action));
    m_redo.clear();
}

void UndoManager::undo()
{
    if (!m_undo.empty())
    {
        m_undo.back()->undoAction();
        m_redo.push_back(std::move(m_undo.back()));
        m_undo.pop_back();
    }
}

void UndoManager::redo()
{
    if (!m_redo.empty())
    {
        m_redo.back()->doAction();
        m_undo.push_back(std::move(m_redo.back()));
        m_redo.pop_back();
    }
}
