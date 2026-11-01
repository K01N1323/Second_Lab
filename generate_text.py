from pathlib import Path
import random

PROJECT = Path(__file__).resolve().parent


def make_word(number):
    word = ""
    for index in range(4):
        word = chr(ord("a") + number % 26) + word
        number //= 26
    return "word" + word


def generate_text():
    # Фиксирую начальное значение, чтобы получать один и тот же текст.
    randomizer = random.Random(1323)
    common = "дерево таблица поиск слово страница текст алгоритм данные список массив ключ значение память время тест результат".split()
    vocabulary = [make_word(number) for number in range(8192)]
    with (PROJECT / "index_text.txt").open("w", encoding="utf-8") as file:
        for line in range(10000):
            words = []
            for index in range(100):
                source = common if randomizer.randrange(4) == 0 else vocabulary
                words.append(randomizer.choice(source))
            file.write(" ".join(words) + "\n")
    print("index_text.txt: 1 000 000 слов, 8 208 разных слов")


if __name__ == "__main__":
    generate_text()
