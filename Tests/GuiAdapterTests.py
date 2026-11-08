from pathlib import Path
import sys
import json
import subprocess
import tempfile

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
import gui


def run_gui_adapter_tests():
    # Проверяю передачу данных и страницы без запуска окна.
    with tempfile.TemporaryDirectory(prefix="index-test-") as folder:
        program = Path(folder) / "adapter"
        for count, size, mode in [(1, 1000, "chars"), (10000, 1000, "chars"),
                                  (1000000, 1000, "chars"), (100, 2, "words")]:
            text, data, pages = gui.prepare_index(program, count, size, mode, "tree")
            hashed = gui.run_index(program, text, size, mode, "hash")
            assert data["entries"] == hashed["entries"], "Словари дали разные результаты"
            reference = {}
            for number, (words, used, limit) in enumerate(pages, 1):
                assert used <= limit, "Превышен размер страницы"
                for word in words:
                    numbers = reference.setdefault(word, [])
                    if not numbers or numbers[-1] != number:
                        numbers.append(number)
            assert data["entries"] == reference, "Страницы Python и C++ различаются"
            assert sum(len(page[0]) for page in pages) == count, "Обработано не N слов"
            assert len(data["timings"]) == 2, "Нет замеров обоих словарей"
            print(f"  Успешно: адаптер, N={count}, режим {mode}")

        for count in [0, -1, 1000001]:
            try:
                gui.read_words(count)
            except ValueError:
                pass
            else:
                raise AssertionError("Принято некорректное N")
        try:
            gui.run_index(program, "toolong", 2, "chars", "tree")
        except ValueError:
            pass
        else:
            raise AssertionError("Слишком длинное слово принято")
        assert gui.read_words(2).split() == gui.read_words(10).split()[:2], "Читается не префикс файла"
        for size, mode in [(0, "words"), (1, "chars"), (2147483648, "chars"), (10, "unknown")]:
            for action in (lambda: gui.make_pages("", size, mode),
                           lambda: gui.run_index(program, "a", size, mode, "tree")):
                try:
                    action()
                except ValueError:
                    pass
                else:
                    raise AssertionError("Приняты некорректные параметры страницы")
        try:
            gui.run_index(program, "a", 10, "words", "unknown")
        except ValueError:
            pass
        else:
            raise AssertionError("Принят неизвестный словарь")

        # Проверяю и прямой запуск адаптера, минуя проверки Python.
        for arguments in [("unknown", "10", "words"), ("tree", "10", "unknown"),
                          ("tree", "10abc", "words")]:
            result = subprocess.run([str(program), *arguments], input="a", text=True,
                                    capture_output=True, check=True, timeout=30)
            assert "error" in json.loads(result.stdout), "Адаптер принял некорректные параметры"
        text = 'ёж "кавычки" путь\\файл нуль\x00слово ёж'
        escaped = gui.run_index(program, text, 2, "words", "hash")
        assert set(escaped["entries"]) == set(text.split()), "Повреждены ключи при передаче JSON"
        assert escaped["entries"]["ёж"] == [1, 3], "Неверные страницы UTF-8"
    print("  Успешно: границы N и передача ошибок C++")


if __name__ == "__main__":
    run_gui_adapter_tests()
