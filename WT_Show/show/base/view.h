#ifndef VIEW_H
#define VIEW_H

#include <functional>
#include <QStringList>
#include <QMap>

#include "module.h"
#include "updateuievt.h"

typedef std::function<void (UpdateUIEvt&)> UIUpdateCallback;

class ParamNameList: public QStringList
{
public:
    ParamNameList();

    ParamNameList(const QString& rcStr)
    {
        this->append(rcStr);
    }

    ParamNameList(const QStringList& rcStrList)
    {
        foreach(const QString& str, rcStrList)
            this->append(str);
    }

    bool operator < (const ParamNameList& cOther) const
    {
        if(this->size() < cOther.size())
            return true;
        if(this->size() > cOther.size())
            return false;
        int iSize = this->size();

        for(int i = 0; i < iSize; i++)
        {
            if(this->at(i) < cOther.at(i))
                return true;
        }

        return false;
    }
};

class View : public Module
{
public:
    View(EventBus* pEventBus = nullptr);
    virtual ~View() {}

    bool fire(Event& evt);

    bool listenToParams(const QString& params, const UIUpdateCallback& callback);
    bool listenToParams(const QStringList& params, const UIUpdateCallback& callback);
    bool unlistenToParams(const QStringList& params);
    bool isListenToParams(const QStringList& params) const;

    ADD_CLASS_FIELD_PRIVATE( CONCATE(QMap<ParamNameList,UIUpdateCallback>), pCallbacks )
};

#endif // VIEW_H
