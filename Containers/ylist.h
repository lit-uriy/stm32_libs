#ifndef YLIST_H
#define YLIST_H

#include <cassert>
#include <iostream>

#ifndef Y_ASSERT
#define Y_ASSERT(cond, place, message) assert(cond);
#endif

template<typename T>
class YList
{
public:
    YList(int areserve = 10);
    ~YList();

    YList(const YList<T> &other );
// WARNING: шаг резервирования места не копируется >> уже копируется!!!
    YList<T>& operator=(const YList<T> &other);


    void append(const T &value);
    void append(const YList<T> & values);

//    void insert(int i, const T &value); //TODO: Сделать функцию
//    void prepend(const T &value); //TODO: Сделать функцию

    void clear();
    bool isEmpty() const {return !actual;}

    void removeAt(int i);
    T take(int i);

    bool contains( const T &value ) const;
    int	indexOf(const T &value, int from = 0) const;

    int	size() const {return actual;}
    int	count() const {return actual;}
    int length() const {return actual;}

    T& operator[] (int i);
    const T& operator[] (int i) const;
//    const T& operator[] (int i) const; // TODO: Сделать функцию >> уже сделана
    const T& at(int i) const;
private:
    T *d;
    int rezerve; // кол-во места под элементы контейнера
    int actual; // реальное кол-во элементов в контейнере

    const int ReserveStep;

    void addSpace(int addsize);
    void copyData(const YList<T> &other);
};

//----------------------------------------

template <typename T>
YList<T>::YList(int areserve)
    :rezerve(areserve)
    ,actual(0)
    ,ReserveStep(areserve)
{
    d = new T[rezerve]; // сразу резервируем места под areserve элементов
}


template <typename T>
YList<T>::~YList()
{
    delete[] d;
}


template <typename T>
YList<T>::YList(const YList<T> &other )
    :ReserveStep(other.ReserveStep)
{
//    std::cout << "Copy constructor\r\n";
    copyData(other);
}


template <typename T>
YList<T>& YList<T>::operator=(const YList<T> &other)
{
//    std::cout << "operator=\r\n";
    if (d != other.d){
        delete[] d;
        copyData(other);
    }
    return *this;
}



template <typename T>
void  YList<T>::copyData(const YList<T> &other)
{
    actual = other.actual;
    rezerve = other.rezerve;
    d = new T[rezerve];// массив ещё не инициализирован - создаём новый
    //скопируем элементы входного списка
    for (int i = 0; i < actual; ++i) {
        d[i] = other.d[i];
    }
}


template <typename T>
void YList<T>::append(const T &value)
{
    if (actual >= rezerve){
        // место кончилось - добавим ещё
        addSpace(ReserveStep);
    }
    d[actual++] = value;
}


template <typename T>
void YList<T>::append(const YList<T> & values)
{
    int addsize = values.actual;
    int rez = rezerve - actual;
    if (rez <= addsize){
        // места не хватает - добавим с запасиком
        addSpace(addsize - rez + ReserveStep);
    }
    //скопируем элементы входного списка
    for (int i = 0; i < addsize; ++i) {
        d[actual++] = values.d[i];
    }
}


template <typename T>
void YList<T>::clear()
{
    delete[] d;
    actual = 0;
    rezerve = ReserveStep;
    d = new T[rezerve];
}


template <typename T>
void YList<T>::removeAt(int i)
{
    T *old = d;
    d = new T[rezerve];
    int ni = 0;
    for (int oi = 0; oi < actual; ++oi) {
        if (i != oi)
            d[ni++] = old[oi];
    }
    delete[] old;
    actual--;
}

template <typename T>
T YList<T>::take(int i)
{
//    std::cout << "T& take()\r\n";
    Y_ASSERT(i >= 0 && i < actual, "YList<T>::take()", "index out of range");
    T out = d[i];
    removeAt(i);
    return out;
}


template <typename T>
bool YList<T>::contains( const T &value ) const
{
    return indexOf(value) >= 0 ? true : false;
}

template <typename T>
int	YList<T>::indexOf(const T &value, int from) const
{
    int i;
    if (from >= actual) return -1; // за пределами элементов
    for (i = from; i < actual; ++i) {
        if(d[i] == value){ return i;}
    }
    return -1;
}

template <typename T>
T& YList<T>::operator[] (int i)
{
//    std::cout << "T& operator[]\r\n";
    Y_ASSERT(i >= 0 && i < actual, "YList<T>::operator[]", "index out of range");
    T &out = d[i];
    return out;
}

template <typename T>
const T& YList<T>::operator[] (int i) const
{
//    std::cout << "const T operator[]\r\n";
    Y_ASSERT(i >= 0 && i < actual, "YList<T>::operator[]", "index out of range");
    const T &out = d[i];
    return out;
}

template <typename T>
const T& YList<T>::at(int i) const
{
//    std::cout << "const T at()\r\n";
    Y_ASSERT(i >= 0 && i < actual, "YList<T>::at()", "index out of range");
    const T &out = d[i];
    return out;
}

template <typename T>
void YList<T>::addSpace(int addsize)
{
    T *old = d;
    rezerve += addsize;
    d = new T[rezerve];
    for (int i = 0; i < actual; ++i) {
        d[i] = old[i];
    }
    delete[] old;

}

#endif // YLIST_H
