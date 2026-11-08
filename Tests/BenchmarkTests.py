from pathlib import Path
import subprocess
import sys
import tempfile


def run_benchmark_tests(program):
    # Проверяю ошибки до первого замера и после частичного выполнения.
    cases = [(None, "Не найден index_text.txt"),
             ("a " * 150, "меньше миллиона слов"),
             ("x" * 1000 + " a" * 99, "Слово не помещается")]
    for text, message in cases:
        with tempfile.TemporaryDirectory(prefix="index-benchmark-test-") as folder:
            directory = Path(folder)
            output = directory / "index_performance.csv"
            previous = b"previous measurements\n"
            output.write_bytes(previous)
            if text is not None:
                (directory / "index_text.txt").write_text(text, encoding="utf-8")
            result = subprocess.run([str(program), "--index-performance"],
                                    cwd=directory, capture_output=True, text=True,
                                    encoding="utf-8", timeout=30)
            assert result.returncode != 0, "Ошибка входного текста не обнаружена"
            assert message in result.stderr, f"Неожиданная ошибка: {result.stderr}"
            assert output.read_bytes() == previous, "Ошибка замера затёрла предыдущий CSV"
            assert not list(directory.glob("*.tmp")), "Остался временный CSV"
    print("  Успешно: ошибки эксперимента сохраняют предыдущий CSV")


if __name__ == "__main__":
    run_benchmark_tests(Path(sys.argv[1]).resolve())
