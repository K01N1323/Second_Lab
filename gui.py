

from concurrent.futures import ThreadPoolExecutor
from itertools import islice
import csv
import json
from pathlib import Path
import subprocess
import tempfile
import tkinter as tk
from tkinter import filedialog, messagebox, ttk

PROJECT = Path(__file__).resolve().parent

CPP = r'''
#include <chrono>
#include <iostream>
#include <iterator>
#include <string>
#include "Include/IndexMeasurement.hpp"

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
        int size = std::stoi(argv[2]);
        IndexMeasurement measurements[2][5];
        for (int run = -1; run < 5; run++) {
            for (int order = 0; order < 2; order++) {
                int number = (order + (run + 1) % 2) % 2;
                AlphabeticalIndex index(number == 0 ? DictionaryType::Tree : DictionaryType::Hash);
                auto measured = MeasureIndex(index, text, size, mode);
                if (run >= 0) {measurements[number][run] = measured;}
            }
        }
        std::cout << "{\"timings\":[";
        for (int number = 0; number < 2; number++) {
            auto measured = MedianMeasurement(measurements[number], 5);
            if (number != 0) {std::cout << ',';}
            std::cout << "{\"build\":" << measured.BuildTime
                      << ",\"entries\":" << measured.EntriesTime
                      << ",\"total\":" << measured.GetTotal() << '}';
        }
        // Готовлю выбранный результат для окна вне измеряемых участков.
        AlphabeticalIndex index(type);
        index.Build(text, size, mode);
        auto entries = index.GetEntries();
        std::cout << "],\"entries\":{";
        IEnumerator<AlphabeticalIndex::Entry>* it = entries.GetEnumerator();
        for (int i = 0; it->HasNext(); i++, it->MoveNext()) {
            if (i != 0) {std::cout << ',';}
            const auto& entry = it->GetCurrent();
            WriteString(entry.key);
            std::cout << ":[";
            for (int j = 0; j < entry.value.GetLength(); j++) {
                if (j != 0) {std::cout << ',';}
                std::cout << entry.value.Get(j);
            }
            std::cout << ']';
        }
        delete it;
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
                            capture_output=True, text=True, encoding="utf-8", check=True, timeout=300)
    data = json.loads(result.stdout)
    if "error" in data:
        raise ValueError(data["error"])
    return data


def read_words(count):
    if not 1 <= count <= 2147483647:
        raise ValueError("N — целое число от 1 до 2147483647.")
    with (PROJECT / "index_text.txt").open(encoding="utf-8") as file:
        words = list(islice((word for line in file for word in line.split()), count))
    if len(words) != count:
        raise ValueError(f"В index_text.txt только {len(words)} слов. Уменьшите N.")
    return " ".join(words)


def prepare_index(program, count, size, mode, kind="tree"):
    text = read_words(count)
    data = run_index(program, text, size, mode, kind)
    return text, data, make_pages(text, size, mode)


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
        self.geometry("1080x780")
        self.minsize(900, 650)
        self.folder = tempfile.TemporaryDirectory(prefix="index-ui-")
        self.program = Path(self.folder.name) / "adapter"
        self.entries, self.pages, self.words = {}, [], []
        self.worker = ThreadPoolExecutor(max_workers=1)
        self.task = None
        self.measurements = []
        self.page_start = 0
        self.unit = "байт UTF-8"
        self.mode_choice = tk.StringVar(value="байт UTF-8")
        self.size = tk.StringVar(value="1000")
        self.count = tk.StringVar(value="10000")
        self.search = tk.StringVar()
        self.status = tk.StringVar(value="Задайте число слов и нажмите «Сравнить».")
        self.result = tk.StringVar(value="Время появится после сравнения")
        self.page_status = tk.StringVar(value="Нет страниц")
        self.configure_styles()

        main = ttk.Frame(self, padding=24)
        main.pack(fill="both", expand=True)
        main.columnconfigure(0, weight=1)
        main.rowconfigure(5, weight=1)
        header = ttk.Frame(main)
        header.grid(row=0, column=0, sticky="ew", pady=(0, 22))
        ttk.Label(header, text="Алфавитный указатель", style="Title.TLabel").pack(anchor="w")
        ttk.Label(header, text="Сравнение AVL-дерева и хеш-таблицы", style="Muted.TLabel").pack(anchor="w", pady=(4, 0))

        controls = ttk.Frame(main)
        controls.grid(row=1, column=0, sticky="ew")
        controls.columnconfigure(3, weight=1)
        for column, title in enumerate(("Слов из файла", "Размер страницы", "Единица размера")):
            ttk.Label(controls, text=title, style="Muted.TLabel").grid(row=0, column=column, sticky="w", pady=(0, 6))
        ttk.Entry(controls, textvariable=self.count, width=14).grid(row=1, column=0, sticky="w", padx=(0, 16))
        ttk.Entry(controls, textvariable=self.size, width=14).grid(row=1, column=1, sticky="w", padx=(0, 16))
        ttk.Combobox(controls, textvariable=self.mode_choice, values=["байт UTF-8", "слов"],
                     state="readonly", width=14).grid(row=1, column=2, sticky="w")
        self.build_button = ttk.Button(controls, text="Сравнить", command=self.build, style="Accent.TButton")
        self.build_button.grid(row=1, column=4, sticky="e")
        ttk.Label(controls, text="index_text.txt  ·  Первая страница — ½, каждая десятая — ¾ размера.",
                  style="Muted.TLabel").grid(row=2, column=0, columnspan=5, sticky="w", pady=(10, 0))

        timing_header = ttk.Frame(main)
        timing_header.grid(row=2, column=0, sticky="ew", pady=(24, 10))
        ttk.Label(timing_header, textvariable=self.result, style="Section.TLabel").pack(side="left")
        self.save_button = ttk.Button(timing_header, text="Экспорт CSV", command=self.save_measurements, state="disabled")
        self.save_button.pack(side="right")
        table = ttk.Frame(main)
        table.grid(row=3, column=0, sticky="ew")
        self.timings = ttk.Treeview(table, columns=("kind", "build", "entries", "total"),
                                    show="headings", height=2, selectmode="none", takefocus=False)
        for column, title in zip(("kind", "build", "entries", "total"),
                                 ("Структура данных", "Построение, мс", "Выдача, мс", "Итого, мс")):
            self.timings.heading(column, text=title, anchor="w" if column == "kind" else "e")
            self.timings.column(column, width=190, anchor="w" if column == "kind" else "e")
        for name in ("AVL-дерево", "Хеш-таблица"):
            self.timings.insert("", "end", values=(name, "—", "—", "—"))
        self.timings.tag_configure("faster", foreground="#9DE0CC")
        self.timings.pack(fill="x")
        ttk.Label(table, text="Медиана 5 запусков. Чтение файла и отрисовка не учитываются.",
                  style="Muted.TLabel").pack(anchor="w", pady=(8, 0))

        ttk.Separator(main).grid(row=4, column=0, sticky="ew", pady=20)
        area = ttk.Frame(main)
        area.grid(row=5, column=0, sticky="nsew")
        area.columnconfigure(0, weight=3)
        area.columnconfigure(1, weight=2)
        area.rowconfigure(1, weight=1)
        page_header = ttk.Frame(area)
        page_header.grid(row=0, column=0, sticky="ew", padx=(0, 20), pady=(0, 10))
        ttk.Label(page_header, text="Текст по страницам", style="Section.TLabel").pack(side="left")
        self.previous_button = ttk.Button(page_header, text="Назад", width=6,
                                          command=lambda: self.change_pages(-10), state="disabled")
        self.next_button = ttk.Button(page_header, text="Далее", width=6,
                                      command=lambda: self.change_pages(10), state="disabled")
        self.next_button.pack(side="right")
        self.previous_button.pack(side="right", padx=(8, 4))
        ttk.Label(page_header, textvariable=self.page_status, style="Muted.TLabel").pack(side="right")
        preview_frame = ttk.Frame(area)
        preview_frame.grid(row=1, column=0, sticky="nsew", padx=(0, 20))
        preview_frame.columnconfigure(0, weight=1)
        preview_frame.rowconfigure(0, weight=1)
        self.preview = tk.Text(preview_frame, height=10, width=40, wrap="word", font=("Helvetica", 13),
                                   background="#182022", foreground="#EAF1EE", borderwidth=0, highlightthickness=0,
                                   padx=16, pady=14, spacing1=2, spacing3=2, state="disabled", takefocus=False)
        self.preview.grid(row=0, column=0, sticky="nsew")
        page_scrollbar = ttk.Scrollbar(preview_frame, orient="vertical", command=self.preview.yview)
        page_scrollbar.grid(row=0, column=1, sticky="ns")
        self.preview.configure(yscrollcommand=page_scrollbar.set)
        self.preview.tag_configure("selected", background="#91D8C5", foreground="#152A24")
        self.preview.tag_configure("page", foreground="#9DE0CC", font=("Helvetica", 12, "bold"), spacing3=8)
        self.preview.configure(state="normal")
        self.preview.insert("end", "После сравнения здесь появится текст с разбиением на страницы.")
        self.preview.configure(state="disabled")

        ttk.Label(area, text="Слова и страницы", style="Section.TLabel").grid(row=0, column=1, sticky="w", pady=(0, 10))
        side = ttk.Frame(area)
        side.grid(row=1, column=1, sticky="nsew")
        search_row = ttk.Frame(side)
        search_row.pack(fill="x", pady=(0, 10))
        entry = ttk.Entry(search_row, textvariable=self.search)
        entry.pack(side="left", fill="x", expand=True)
        entry.bind("<Return>", lambda event: self.find_word())
        ttk.Button(search_row, text="Найти", command=self.find_word).pack(side="right", padx=(8, 0))
        list_frame = ttk.Frame(side)
        list_frame.pack(fill="both", expand=True)
        list_frame.columnconfigure(0, weight=1)
        list_frame.rowconfigure(0, weight=1)
        self.index = tk.Listbox(list_frame, width=26, font=("Helvetica", 13), exportselection=False,
                                background="#182022", foreground="#EAF1EE", borderwidth=0, highlightthickness=1,
                                highlightbackground="#182022", highlightcolor="#91D8C5",
                                selectbackground="#36584F", selectforeground="#FFFFFF", activestyle="none")
        horizontal = ttk.Scrollbar(list_frame, orient="horizontal", command=self.index.xview)
        vertical = ttk.Scrollbar(list_frame, orient="vertical", command=self.index.yview)
        self.index.configure(xscrollcommand=horizontal.set, yscrollcommand=vertical.set)
        self.index.grid(row=0, column=0, sticky="nsew")
        vertical.grid(row=0, column=1, sticky="ns")
        horizontal.grid(row=1, column=0, sticky="ew")
        self.index.bind("<<ListboxSelect>>", self.select_word)
        ttk.Label(main, textvariable=self.status, style="Muted.TLabel", wraplength=820).grid(row=6, column=0, sticky="w", pady=(16, 0))
        self.protocol("WM_DELETE_WINDOW", self.close)

    def configure_styles(self):
        self.configure(background="#202729")
        style = ttk.Style(self)
        style.theme_use("clam")
        style.configure(".", font=("Helvetica", 12), background="#202729", foreground="#EAF1EE")
        style.configure("TFrame", background="#202729")
        style.configure("TLabel", background="#202729")
        style.configure("Muted.TLabel", foreground="#B5C4BF", font=("Helvetica", 11))
        style.configure("Title.TLabel", font=("Helvetica", 24, "bold"))
        style.configure("Section.TLabel", font=("Helvetica", 13, "bold"))
        style.configure("TEntry", fieldbackground="#182022", foreground="#EAF1EE",
                        insertcolor="#EAF1EE", bordercolor="#455550", lightcolor="#455550", darkcolor="#455550", padding=8)
        style.map("TEntry", bordercolor=[("focus", "#91D8C5")])
        style.configure("TCombobox", fieldbackground="#182022", background="#35433F", foreground="#EAF1EE",
                        arrowcolor="#EAF1EE", bordercolor="#455550", lightcolor="#455550", darkcolor="#455550", padding=8)
        style.map("TCombobox", fieldbackground=[("readonly", "#182022")],
                  foreground=[("readonly", "#EAF1EE")], selectbackground=[("readonly", "#182022")],
                  selectforeground=[("readonly", "#EAF1EE")])
        self.option_add("*TCombobox*Listbox.background", "#182022")
        self.option_add("*TCombobox*Listbox.foreground", "#EAF1EE")
        style.configure("TButton", background="#35433F", foreground="#EAF1EE", borderwidth=0, padding=(12, 9))
        style.map("TButton", background=[("disabled", "#293431"), ("active", "#455A53")],
                  foreground=[("disabled", "#879B93")], bordercolor=[("focus", "#91D8C5")])
        style.configure("Accent.TButton", background="#91D8C5", foreground="#152A24", font=("Helvetica", 12, "bold"), padding=(24, 9))
        style.map("Accent.TButton", background=[("disabled", "#405B52"), ("active", "#ADE8D8")],
                  foreground=[("disabled", "#CCDAD4")])
        style.layout("Treeview", [("Treeview.treearea", {"sticky": "nswe"})])
        style.configure("Treeview", background="#202729", fieldbackground="#202729", foreground="#EAF1EE",
                        borderwidth=0, rowheight=32)
        style.configure("Treeview.Heading", background="#2C3733", foreground="#C9D8D2", borderwidth=0,
                        font=("Helvetica", 11), padding=(10, 8))
        style.map("Treeview", background=[("selected", "#36584F")])
        style.configure("TSeparator", background="#3B4944")
        style.configure("TScrollbar", background="#455A53", troughcolor="#202729", borderwidth=0,
                        arrowcolor="#B5C4BF", arrowsize=12, bordercolor="#202729", lightcolor="#455A53", darkcolor="#455A53")
        style.map("TScrollbar", background=[("active", "#60796F"), ("!active", "#455A53")])

    def build(self):
        if self.task is not None:
            return
        try:
            size = int(self.size.get())
            count = int(self.count.get())
            if not 2 <= size <= 2147483647:
                raise ValueError("Размер страницы — целое число от 2 до 2147483647.")
            if not 1 <= count <= 2147483647:
                raise ValueError("N — целое число от 1 до 2147483647.")
            mode = "words" if self.mode_choice.get() == "слов" else "chars"
            self.parameters = count, size, mode
            self.status.set("Чтение файла и сравнение двух словарей…")
            self.build_button.configure(state="disabled", text="Сравнение…")
            self.task = self.worker.submit(prepare_index, self.program, count, size, mode)
            self.after(100, self.finish_build)
        except Exception as error:
            self.show_error(error)

    def finish_build(self):
        if not self.task.done():
            self.after(100, self.finish_build)
            return
        try:
            text, data, pages = self.task.result()
            count, size, mode = self.parameters
            self.entries, self.pages = data["entries"], pages
            self.words = list(self.entries)
            self.unit = "слов" if mode == "words" else "байт UTF-8"
            self.search.set("")
            self.index.delete(0, "end")
            rows = [f"{word} → {', '.join(map(str, numbers))}" for word, numbers in self.entries.items()]
            if rows:
                self.index.insert("end", *rows)
            self.timings.delete(*self.timings.get_children())
            times = [measured["total"] for measured in data["timings"]]
            faster = times.index(min(times))
            close = max(times) <= min(times) * 1.05
            for number, (name, measured) in enumerate(zip(("AVL-дерево", "Хеш-таблица"), data["timings"])):
                self.timings.insert("", "end", values=(name, f"{measured['build']:.3f}",
                                    f"{measured['entries']:.3f}", f"{measured['total']:.3f}"),
                                    tags=("faster",) if number == faster and not close else ())
                self.measurements.append((count, len(self.entries), size, mode, name,
                                          measured["build"], measured["entries"], measured["total"]))
            self.page_start = 0
            self.draw_pages()
            if close or min(times) <= 0:
                self.result.set("Время работы близко")
            else:
                name = "AVL-дерево" if faster == 0 else "Хеш-таблица"
                self.result.set(f"{name} быстрее в {max(times) / min(times):.1f} раза")
            self.save_button.configure(state="normal")
            self.status.set(f"{count:,} слов · {len(self.entries):,} разных · {len(pages):,} страниц".replace(",", " "))
        except Exception as error:
            self.show_error(error)
        finally:
            self.task = None
            self.build_button.configure(state="normal", text="Сравнить")

    def show_error(self, error):
        self.status.set("Построение не выполнено. Предыдущий результат сохранён.")
        messagebox.showerror("Ошибка", getattr(error, "stderr", None) or str(error), parent=self)

    def save_measurements(self):
        if not self.measurements:
            messagebox.showinfo("CSV", "Сначала выполните построение.", parent=self)
            return
        filename = filedialog.asksaveasfilename(parent=self, defaultextension=".csv",
                                               initialfile="index_measurements.csv", filetypes=[("CSV", "*.csv")])
        if filename:
            try:
                with open(filename, "w", encoding="utf-8-sig", newline="") as file:
                    writer = csv.writer(file)
                    writer.writerow(("n", "unique", "page_size", "mode", "dictionary", "build_ms", "entries_ms", "total_ms"))
                    writer.writerows(self.measurements)
            except OSError as error:
                messagebox.showerror("Ошибка сохранения", str(error), parent=self)

    def change_pages(self, offset):
        if self.pages:
            self.page_start = max(0, min(self.page_start + offset, (len(self.pages) - 1) // 10 * 10))
            self.draw_pages(self.search.get())

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
            self.page_start = (numbers[0] - 1) // 10 * 10
            self.draw_pages(word)
            self.preview.see(self.page_lines[numbers[0]])
            shown = ", ".join(map(str, numbers[:20]))
            remaining = f" … ещё {len(numbers) - 20}" if len(numbers) > 20 else ""
            self.status.set(f"{word} → {shown}{remaining}")
        else:
            self.draw_pages()
            self.status.set("Слово не найдено.")

    def draw_pages(self, selected=None):
        self.preview.configure(state="normal")
        self.preview.delete("1.0", "end")
        self.page_lines = {}
        for number, (words, used, limit) in enumerate(self.pages[self.page_start:self.page_start + 10], self.page_start + 1):
            self.page_lines[number] = self.preview.index("end-1c")
            self.preview.insert("end", f"Страница {number}   ·   {used}/{limit} {self.unit}\n", ("page",))
            for word in words:
                tags = ("selected",) if word == selected else ()
                self.preview.insert("end", word + " ", tags)
            self.preview.insert("end", "\n\n")
        if not self.pages:
            self.preview.insert("end", "Нет страниц.")
        self.preview.configure(state="disabled")
        self.page_status.set(f"{self.page_start + 1}–{min(self.page_start + 10, len(self.pages))} из {len(self.pages)}" if self.pages else "Нет страниц")
        self.previous_button.configure(state="normal" if self.page_start > 0 else "disabled")
        self.next_button.configure(state="normal" if self.page_start + 10 < len(self.pages) else "disabled")

    def close(self):
        if self.task is not None:
            self.status.set("Дождитесь завершения сравнения перед закрытием окна.")
            return
        self.worker.shutdown(wait=False)
        self.folder.cleanup()
        self.destroy()


if __name__ == "__main__":
    App().mainloop()
