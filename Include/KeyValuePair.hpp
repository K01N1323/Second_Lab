#ifndef KEY_VALUE_PAIR_HPP
#define KEY_VALUE_PAIR_HPP

template <typename TKey, typename TValue>
struct KeyValuePair {
    TKey key;
    TValue value;

    KeyValuePair(const TKey& Key, const TValue& Value): key(Key), value(Value) {}
};

#endif // Key_value_pair_h