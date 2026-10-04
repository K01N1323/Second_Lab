#ifndef ALPHABETICAL_INDEX_HPP
#define ALPHABETICAL_INDEX_HPP

#include <string>
#include <limits>

#include "TreeDictionary.hpp"
#include "HashDictionary.hpp"
#include "IDictionary.hpp"
#include "Sequences/MutableArraySequence.hpp"
#include "Sequences/MutableListSequence.hpp"

enum class PageSizeMode {
    Words,
    Characters
};

enum class DictionaryType {
    Tree,
    Hash
};

class AlphabeticalIndex {
public:
    using PageList = MutableArraySequence<int>;
    using Entry = KeyValuePair<std::string, PageList>;

private:
    IDictionary<std::string, PageList>* dictionary;
    DictionaryType dictionaryType;

    // Сортирую адреса записей, чтобы не копировать списки страниц при перестановках.
    static void SiftDown(DynamicArray<const AlphabeticalIndex::Entry*>& entries, int RootIndex, int Count) {
        while (RootIndex < Count / 2) {
            int ChildIndex = RootIndex * 2 + 1;

            if (ChildIndex + 1 < Count && entries.Get(ChildIndex)->key < entries.Get(ChildIndex + 1)->key) {
                ChildIndex++;
            }

            if (!(entries.Get(RootIndex)->key < entries.Get(ChildIndex)->key)) {return;}

            const AlphabeticalIndex::Entry* TempEntry = entries.Get(RootIndex);
            entries.Set(RootIndex, entries.Get(ChildIndex));
            entries.Set(ChildIndex, TempEntry);
            RootIndex = ChildIndex;
        }
    }

    static void SortEntries(DynamicArray<const AlphabeticalIndex::Entry*>& entries) {
        int Count = entries.GetSize();

        for (int RootIndex = Count / 2 - 1; RootIndex >= 0; RootIndex--) {
            SiftDown(entries, RootIndex, Count);
        }

        for (int LastIndex = Count - 1; LastIndex > 0; LastIndex--) {
            const AlphabeticalIndex::Entry* TempEntry = entries.Get(0);
            entries.Set(0, entries.Get(LastIndex));
            entries.Set(LastIndex, TempEntry);
            SiftDown(entries, 0, LastIndex);
        }
    }

    static bool CompareEntries(const Entry& first, const Entry& second) {
        return first.key < second.key;
    }

    static int CompareKey(const std::string& key, const Entry& entry) {
        if (key < entry.key) {return -1;}
        if (entry.key < key) {return 1;}
        return 0;
    }

    static bool EqualWords(const std::string& first, const std::string& second) {
        return first == second;
    }

    static unsigned int HashWord(const std::string& word) {
        unsigned int HashCode = 0;

        for (unsigned char Symbol : word) {
            HashCode = HashCode * 31u + Symbol;
        }

        return HashCode;
    }

    static int GetPageLimit(int PageSize, int PageNumber) {
        if (PageNumber == 1) {return PageSize / 2;}
        if (PageNumber % 10 == 0) {return static_cast<int>(3LL * PageSize / 4);}
        return PageSize;
    }

    static IDictionary<std::string, PageList>* CreateDictionary(DictionaryType Type) {
        if (Type == DictionaryType::Tree) {
            return new TreeDictionary<std::string, PageList>(CompareEntries, CompareKey);
        }

        if (Type == DictionaryType::Hash) {
            return new HashDictionary<std::string, PageList>(HashWord, EqualWords);
        }

        throw std::invalid_argument("Неизвестный тип словаря");
    }

public:
    explicit AlphabeticalIndex(DictionaryType Type = DictionaryType::Tree): dictionary(CreateDictionary(Type)), dictionaryType(Type) {}

    AlphabeticalIndex(const AlphabeticalIndex& other) = delete;
    AlphabeticalIndex& operator=(const AlphabeticalIndex& other) = delete;

    // После перемещения исходный объект пуст, его можно снова заполнить через Build.
    AlphabeticalIndex(AlphabeticalIndex&& other) noexcept: dictionary(other.dictionary), dictionaryType(other.dictionaryType) {
        other.dictionary = nullptr;
    }

    AlphabeticalIndex& operator=(AlphabeticalIndex&& other) noexcept {
        if (this == &other) {return *this;}

        delete dictionary;
        dictionary = other.dictionary;
        dictionaryType = other.dictionaryType;
        other.dictionary = nullptr;

        return *this;
    }

    ~AlphabeticalIndex() {
        delete dictionary;
    }

    // В режиме Characters считаю байты std::string и один пробел между словами.
    void Build(const std::string& Text, int PageSize, PageSizeMode Mode = PageSizeMode::Characters) {
        if (PageSize <= 0) {throw std::invalid_argument("Размер страницы должен быть положительным");}
        if (PageSize / 2 == 0) {throw std::invalid_argument("Предел первой страницы равен нулю");}
        if (Mode != PageSizeMode::Words && Mode != PageSizeMode::Characters) {
            throw std::invalid_argument("Неизвестный режим размера страницы");
        }
        if (Text.length() > static_cast<unsigned int>(std::numeric_limits<int>::max())) {
            throw std::length_error("Текст слишком длинный");
        }

        // Сначала собираю новый результат, чтобы ошибка не испортила старый указатель.
        IDictionary<std::string, PageList>* NewDictionary = CreateDictionary(dictionaryType);
        int TextLength = static_cast<int>(Text.length());
        int Position = 0;
        int PageNumber = 1;
        int UsedSize = 0;
        int PageLimit = GetPageLimit(PageSize, PageNumber);

        try {
            while (Position < TextLength) {
                while (Position < TextLength && Text[Position] == ' ') {Position++;}
                if (Position == TextLength) {break;}

                int WordStart = Position;
                while (Position < TextLength && Text[Position] != ' ') {Position++;}

                std::string word = Text.substr(WordStart, Position - WordStart);
                int WordSize = Mode == PageSizeMode::Words ? 1 : Position - WordStart;
                int SpaceSize = Mode == PageSizeMode::Characters && UsedSize != 0 ? 1 : 0;

                if (static_cast<long long>(UsedSize) + SpaceSize + WordSize > PageLimit && UsedSize != 0) {
                    if (PageNumber == std::numeric_limits<int>::max()) {
                        throw std::length_error("Превышено максимальное число страниц");
                    }

                    PageNumber++;
                    UsedSize = 0;
                    SpaceSize = 0;
                    PageLimit = GetPageLimit(PageSize, PageNumber);
                }

                if (WordSize > PageLimit) {
                    throw std::invalid_argument("Слово не помещается целиком на странице " + std::to_string(PageNumber));
                }

                UsedSize += SpaceSize + WordSize;
                PageList* pages = NewDictionary->Find(word);

                if (pages == nullptr) {
                    PageList NewPages;
                    NewPages.Append(PageNumber);
                    NewDictionary->Add(word, NewPages);
                } else if (pages->GetLast() != PageNumber) {
                    // Не добавляю страницу повторно, если слово уже встречалось на ней.
                    pages->Append(PageNumber);
                }
            }
        } catch (...) {
            delete NewDictionary;
            throw;
        }

        delete dictionary;
        dictionary = NewDictionary;
    }

    int GetCount() const {
        return dictionary == nullptr ? 0 : dictionary->GetCount();
    }

    // Ссылка действительна до успешного Build, Clear или уничтожения владельца.
    const PageList& GetPages(const std::string& word) const {
        const PageList* pages = dictionary == nullptr ? nullptr : dictionary->Find(word);
        if (pages == nullptr) {throw std::out_of_range("Слово не найдено");}
        return *pages;
    }

    // Возвращаю независимый список. Порядок лексикографический, как у std::string.
    MutableListSequence<Entry> GetEntries() const {
        MutableListSequence<Entry> result;
        if (dictionary == nullptr) {return result;}

        IEnumerator<Entry>* it = dictionary->GetEnumerator();

        try {
            if (dictionaryType == DictionaryType::Tree) {
                while (it->HasNext()) {
                    result.Append(it->GetCurrent());
                    it->MoveNext();
                }
            } else {
                DynamicArray<const Entry*> entries(dictionary->GetCount());
                int Index = 0;

                while (it->HasNext()) {
                    entries.Set(Index, &(it->GetCurrent()));
                    it->MoveNext();
                    Index++;
                }

                SortEntries(entries);

                for (int EntryIndex = 0; EntryIndex < entries.GetSize(); EntryIndex++) {
                    result.Append(*entries.Get(EntryIndex));
                }
            }
        } catch (...) {
            delete it;
            throw;
        }

        delete it;
        return result;
    }

    void Clear() noexcept {
        if (dictionary != nullptr) {dictionary->Clear();}
    }
};

#endif // ALPHABETICAL_INDEX_HPP
