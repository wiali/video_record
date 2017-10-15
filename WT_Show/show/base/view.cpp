#include "view.h"
#include <QDebug>

View::View(EventBus *pEventBus) : Module(pEventBus)
{
    subscribeToEvtByName( _UPDATE_UI_REQUEST_EVENT, MAKE_CALLBACK(View::fire) );
}

bool View::fire(Event& evt)
{
    UpdateUIEvt& rcUpdateUIEvt = static_cast<UpdateUIEvt&>(evt);

    QMap<ParamNameList,UIUpdateCallback>::iterator iter= m_pCallbacks.begin();
    /// check every callback functions
    while(iter != m_pCallbacks.end())
    {
        /// if the event has all the parameters that this callback function listens to, call it.
        bool findAllParams = true;
        const ParamNameList& cList = iter.key();
        for(int i = 0; i < cList.size(); i++)
        {
            if(evt.getParameters().hasParameter(cList.at(i)) == false)
            {
                findAllParams = false;
                break;
            }
        }

        if(findAllParams == true)
            (iter.value())(rcUpdateUIEvt);      /// call

        iter++;     /// check next
    }

    return true;
}

bool View::listenToParams(const QString& params, const UIUpdateCallback& callback)
{
    m_pCallbacks[params] = callback;
    return true;
}

bool View::listenToParams(const QStringList& params, const UIUpdateCallback& callback)
{
    m_pCallbacks[params] = callback;
    return true;
}

bool View::unlistenToParams(const QStringList& params)
{
    return m_pCallbacks.remove(params);
}

bool View::isListenToParams(const QStringList& params) const
{
    return m_pCallbacks.contains(params);
}
