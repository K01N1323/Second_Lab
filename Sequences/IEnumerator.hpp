#ifndef IENUMERATOR_HPP
#define IENUMERATOR_HPP

// Интерфейс для итератора (перечислителя) элементов
template <typename T>
class IEnumerator {
public:
    virtual ~IEnumerator() {}

    // Возвращает текущий элемент
    virtual const T& GetCurrent() const = 0;

    // Перемещает итератор к следующему элементу
    virtual void MoveNext() = 0;

    // Проверяет, есть ли следующий элемент
    virtual bool HasNext() const = 0;

    const T& GetCurrentItem() const {return GetCurrent();}

    bool Next() {
        if (!HasNext()) {return false;}
        MoveNext();
        return HasNext();
    }

    bool TryGetCurrentItem(T& item) const {
        if (!HasNext()) {return false;}
        item = GetCurrent();
        return true;
    }
};

#endif // IENUMERATOR_HPP
