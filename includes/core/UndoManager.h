#ifndef UNDO_MANAGER_H
#define UNDO_MANAGER_H

#include "core/actions/AAction.h"

#include <deque>
#include <memory>

class UndoManager
{
public:
    void undo();
    void redo();
    void append(std::unique_ptr<AAction> action);
private:
    std::deque<std::unique_ptr<AAction>> m_undo;
    std::deque<std::unique_ptr<AAction>> m_redo;
};

#endif // UNDO_MANAGER_H
