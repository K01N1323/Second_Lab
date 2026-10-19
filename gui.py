

import json
from pathlib import Path
import subprocess
import tempfile
import tkinter as tk
from tkinter import messagebox, ttk
from tkinter.scrolledtext import ScrolledText

PROJECT = Path(__file__).resolve().parent

CPP = r'''
#include <chrono>
#include <iostream>
#include <iterator>
#include <string>
#include "Include/AlphabeticalIndex.hpp"

void WriteString(const std::string& text) {
    std::cout << '"';
    for (unsigned char c : text) {
        if (c == '"' || c == '\\') {std::cout << '\\' << c;}
        else if (c < 32) {
            std::cout << "\\u00" << "0123456789abcdef"[c >> 4]
                      << "0123456789abcdef"[c & 15];
        } else {std::cout << c;}
    }
    std::cout << '"';
}

int main(int argc, char** argv) {
    try {
        if (argc != 4) {throw std::invalid_argument("Неверные параметры адаптера");}
        auto type = std::string(argv[1]) == "tree" ? DictionaryType::Tree : DictionaryType::Hash;
        auto mode = std::string(argv[3]) == "words" ? PageSizeMode::Words : PageSizeMode::Characters;
        std::string text((std::istreambuf_iterator<char>(std::cin)), {});
        AlphabeticalIndex index(type);
        auto start = std::chrono::steady_clock::now();
        index.Build(text, std::stoi(argv[2]), mode);
        double ms = std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - start).count();
        auto entries = index.GetEntries();
        std::cout << "{\"ms\":" << ms << ",\"entries\":{";
        for (int i = 0; i < entries.GetLength(); i++) {
            if (i != 0) {std::cout << ',';}
            const auto& entry = entries.Get(i);
            WriteString(entry.key);
            std::cout << ":[";
            for (int j = 0; j < entry.value.GetLength(); j++) {
                if (j != 0) {std::cout << ',';}
                std::cout << entry.value.Get(j);
            }
            std::cout << ']';
        }
        std::cout << "}}";
    } catch (const std::exception& error) {
        std::cout << "{\"error\":";
        WriteString(error.what());
        std::cout << '}';
    }
}
'''


def run_index(program, text, size, mode, kind):
    # Компилирую адаптер только при первом построении в этом запуске окна.
    if not program.exists():
        source = program.with_suffix(".cpp")
        source.write_text(CPP, encoding="utf-8")
        subprocess.run(["c++", "-std=c++17", "-O2", "-I", str(PROJECT),
                        str(source), "-o", str(program)],
                       capture_output=True, text=True, encoding="utf-8", check=True, timeout=90)
    result = subprocess.run([str(program), kind, str(size), mode], input=text,
                            capture_output=True, text=True, encoding="utf-8", check=True, timeout=30)
    data = json.loads(result.stdout)
    if "error" in data:
        raise ValueError(data["error"])
    return data


def page_limit(size, number):
    if number == 1:
        return size // 2
    return size * 3 // 4 if number % 10 == 0 else size


def make_pages(text, size, mode):
    pages, words = [], []
    used, number = 0, 1
    for word in text.split():
        length = 1 if mode == "words" else len(word.encode("utf-8"))
        space = int(mode == "chars" and bool(words))
        if words and used + space + length > page_limit(size, number):
            pages.append((words, used, page_limit(size, number)))
            words, used, space = [], 0, 0
            number += 1
        if length > page_limit(size, number):
            raise ValueError(f"Слово «{word}» не помещается на странице {number}.")
        words.append(word)
        used += length + space
    if words:
        pages.append((words, used, page_limit(size, number)))
    return pages


class App(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Алфавитный указатель")
        self.geometry("1000x700")
        self.minsize(800, 550)
        self.folder = tempfile.TemporaryDirectory(prefix="index-ui-")
        self.program = Path(self.folder.name) / "adapter"
        self.entries, self.pages, self.words = {}, [], []
        self.unit = "байт UTF-8"
        self.kind = tk.StringVar(value="Дерево")
        self.mode_choice = tk.StringVar(value="Символы")
        self.size = tk.StringVar(value="30")
        self.search = tk.StringVar()
        self.status = tk.StringVar(value="Введите текст и нажмите «Построить».")

        ttk.Label(self, text="Алфавитный указатель", font=("Arial", 20)).pack(anchor="w", padx=12, pady=10)
        self.source = ScrolledText(self, height=5, wrap="word", font=("Arial", 14))
        self.source.pack(fill="x", padx=12)
        self.source.insert("1.0", "кот дом кот сад дом лес кот дом сад лес")
        row = ttk.Frame(self)
        row.pack(fill="x", padx=12, pady=10)
        ttk.Label(row, text="Словарь:").pack(side="left")
        ttk.Combobox(row, textvariable=self.kind, values=["Дерево", "Хеш-таблица"],
                     state="readonly", width=15).pack(side="left", padx=(5, 15))
        ttk.Label(row, text="Размер в:").pack(side="left")
        ttk.Combobox(row, textvariable=self.mode_choice, values=["Символы", "Слова"],
                     state="readonly", width=10).pack(side="left", padx=(5, 15))
        ttk.Label(row, text="На страницу:").pack(side="left")
        ttk.Entry(row, textvariable=self.size, width=8).pack(side="left", padx=5)
        ttk.Button(row, text="Построить", command=self.build).pack(side="right")
        ttk.Label(self, text="Первая страница — половина, каждая десятая — три четверти. Символы считаются в байтах UTF-8.").pack(anchor="w", padx=12)

        area = ttk.Frame(self)
        area.pack(fill="both", expand=True, padx=12, pady=10)
        area.columnconfigure(0, weight=2)
        area.columnconfigure(1, weight=1)
        area.rowconfigure(1, weight=1)
        ttk.Label(area, text="Страницы").grid(row=0, column=0, sticky="w", pady=5)
        ttk.Label(area, text="Слово → страницы").grid(row=0, column=1, sticky="w", padx=10, pady=5)
        self.preview = ScrolledText(area, wrap="word", font=("Arial", 14), state="disabled")
        self.preview.grid(row=1, column=0, sticky="nsew")
        self.preview.tag_configure("selected", background="#CDECE8")
        side = ttk.Frame(area)
        side.grid(row=1, column=1, sticky="nsew", padx=(10, 0))
        search_row = ttk.Frame(side)
        search_row.pack(fill="x", pady=(0, 5))
        entry = ttk.Entry(search_row, textvariable=self.search)
        entry.pack(side="left", fill="x", expand=True)
        entry.bind("<Return>", lambda event: self.find_word())
        ttk.Button(search_row, text="Найти", command=self.find_word).pack(side="right", padx=(5, 0))
        self.index = tk.Listbox(side, width=30, font=("Arial", 13), exportselection=False)
        self.index.pack(fill="both", expand=True)
        self.index.bind("<<ListboxSelect>>", self.select_word)
        ttk.Label(self, textvariable=self.status, wraplength=950).pack(fill="x", padx=12, pady=(0, 10))
        self.protocol("WM_DELETE_WINDOW", self.close)

    def build(self):
        try:
            size = int(self.size.get())
            if not 2 <= size <= 2147483647:
                raise ValueError("Размер страницы — целое число от 2 до 2147483647.")
            text = " ".join(self.source.get("1.0", "end-1c").split())
            if len(text.encode("utf-8")) > 200000:
                raise ValueError("Текст должен быть не больше 200 000 байтов.")
            mode = "words" if self.mode_choice.get() == "Слова" else "chars"
            kind = "tree" if self.kind.get() == "Дерево" else "hash"
            self.status.set("Построение…")
            self.update_idletasks()
            data = run_index(self.program, text, size, mode, kind)
            pages = make_pages(text, size, mode)
            self.entries, self.pages = data["entries"], pages
            self.words = list(self.entries)
            self.unit = "слов" if mode == "words" else "байт UTF-8"
            self.search.set("")
            self.index.delete(0, "end")
            for word, numbers in self.entries.items():
                self.index.insert("end", f"{word} → {', '.join(map(str, numbers))}")
            self.draw_pages()
            self.status.set(f"{self.kind.get()} · {len(self.entries)} разных слов · {len(pages)} страниц · Build: {data['ms']:.3f} мс")
        except Exception as error:
            self.status.set("Построение не выполнено. Предыдущий результат сохранён.")
            messagebox.showerror("Ошибка", getattr(error, "stderr", None) or str(error), parent=self)

    def select_word(self, event):
        selection = self.index.curselection()
        if selection:
            self.find_word(self.words[selection[0]])

    def find_word(self, word=None):
        word = self.search.get() if word is None else word
        self.search.set(word)
        numbers = self.entries.get(word)
        self.index.selection_clear(0, "end")
        if numbers:
            position = self.words.index(word)
            self.index.selection_set(position)
            self.index.see(position)
            self.draw_pages(word)
            self.preview.see(self.page_lines[numbers[0]])
            self.status.set(f"{word} → {', '.join(map(str, numbers))}")
        else:
            self.draw_pages()
            self.status.set("Слово не найдено.")

    def draw_pages(self, selected=None):
        self.preview.configure(state="normal")
        self.preview.delete("1.0", "end")
        self.page_lines = {}
        for number, (words, used, limit) in enumerate(self.pages, 1):
            self.page_lines[number] = self.preview.index("end-1c")
            self.preview.insert("end", f"Страница {number} — {used}/{limit} {self.unit}\n")
            for word in words:
                tags = ("selected",) if word == selected else ()
                self.preview.insert("end", word + " ", tags)
            self.preview.insert("end", "\n\n")
        if not self.pages:
            self.preview.insert("end", "Нет страниц.")
        self.preview.configure(state="disabled")

    def close(self):
        self.folder.cleanup()
        self.destroy()


if __name__ == "__main__":
    App().mainloop()
