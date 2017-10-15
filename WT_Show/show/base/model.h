#ifndef MODEL_H
#define MODEL_H

#include <QMutex>

template <class T>
class Model
{
public:
    static T* getInstance()
    {
        QMutexLocker cLocker(&m_mutex);
        if(m_instance == nullptr)
            m_instance = new T();
        return m_instance;
    }

protected:
    Model() {}
    static QMutex m_mutex;
    static T* m_instance;
};

template <class T>
T* Model<T>::m_instance = nullptr;

template <class T>
QMutex Model<T>::m_mutex;

#endif // MODEL_H
