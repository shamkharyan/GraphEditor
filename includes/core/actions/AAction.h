#ifndef AACTION_H
#define AACTION_H

class AAction
{
public:
    virtual bool doAction() = 0;
    virtual bool undoAction() = 0;
    virtual ~AAction() = default;
protected:
    bool m_completed = false;
};

#endif // AACTION_H
