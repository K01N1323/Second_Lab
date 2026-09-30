#ifndef KEY_VALUE_PAIR_H
#define KEY_VALUE_PAIR_H

template <class TKey, class TValue>
struct KeyValuePair {
    TKey key;
    TValue value;

    KeyValuePair(const TKey& Key, const TValue& Value): key(Key), value(Value) {}
};

#endif // Key_value_pair_h