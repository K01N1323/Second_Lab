#ifndef IENUMERATOR_H
#define IENUMERATOR_H

// Интерфейс для итератора (перечислителя) элементов
template <class T> class IEnumerator {
public:
    virtual ~IEnumerator() {}

    // Возвращает текущий элемент
    virtual const T &GetCurrent() const = 0;

    // Перемещает итератор к следующему элементу
    virtual void MoveNext() = 0;

    // Проверяет, есть ли следующий элемент
    virtual bool HasNext() const = 0;
};

#endif // IENUMERATOR_H