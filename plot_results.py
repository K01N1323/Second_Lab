import argparse
import csv
import math
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.ticker import FuncFormatter

PROJECT = Path(__file__).resolve().parent
NAMES = {"tree": "AVL-дерево", "hash": "Хеш-таблица"}
COLORS = {"tree": "#276A84", "hash": "#B65E36"}


def load_results(path):
    with path.open(encoding="utf-8-sig", newline="") as file:
        rows = list(csv.DictReader(file))
    if not rows:
        raise ValueError("В CSV нет измерений.")
    aliases = {name: kind for kind, name in NAMES.items()}
    for row in rows:
        row["dictionary"] = aliases.get(row["dictionary"], row["dictionary"])
        if row["dictionary"] not in NAMES:
            raise ValueError("Неизвестный словарь в CSV.")
        row["n"] = int(row["n"])
        row["page_size"] = int(row["page_size"])
        row["outputs"] = int(row.get("outputs", 1))
        row["scenario"] = row.get("scenario", "gui")
        row["mode"] = "bytes" if row["mode"] == "chars" else row["mode"]
        if row["mode"] not in ("words", "bytes") or row["n"] < 1 or row["page_size"] < 2 or row["outputs"] < 1:
            raise ValueError("Некорректные параметры измерения в CSV.")
        for field in ("build_ms", "entries_ms", "total_ms"):
            row[field] = float(row[field])
            if not math.isfinite(row[field]) or row[field] <= 0:
                raise ValueError("Для логарифмического графика время должно быть конечным и больше нуля.")
    return rows


def save_figure(figure, folder, name):
    path = folder / f"{name}.png"
    figure.savefig(path, dpi=180, facecolor=figure.get_facecolor())
    plt.close(figure)
    print(path)


def plot_scaling(rows, folder, name, title):
    figure, axes = plt.subplots(1, 2, figsize=(11, 4.5), layout="constrained")
    figure.suptitle(title, fontsize=16, fontweight="bold")
    for axis, field, label in zip(axes, ("total_ms", "entries_ms"),
                                  ("Построение + выдача", "Выдача указателя")):
        for kind, marker in (("tree", "o"), ("hash", "s")):
            # Для повторного N оставляю последнее измерение из экспорта окна.
            points = {row["n"]: row[field] for row in rows if row["dictionary"] == kind}
            points = sorted(points.items())
            if points:
                axis.plot(*zip(*points), color=COLORS[kind], marker=marker,
                          linewidth=2, markersize=6, label=NAMES[kind])
        axis.set(title=label, xlabel="Число слов N", ylabel="Время, мс")
        axis.set_xscale("log")
        axis.set_yscale("log")
        axis.grid(axis="both", which="major", color="#DEE4E7", linewidth=0.8)
        axis.legend(frameon=False)
    figure.supxlabel("Логарифмические шкалы · медиана пяти запусков после прогрева", fontsize=10)
    save_figure(figure, folder, name)


def plot_stages(rows, folder):
    cases = [("file", 1000000, "Текст из файла\nN = 1 000 000"),
             ("shuffled", 100000, "Разные слова\nN = U = 100 000"),
             ("collisions", 4096, "Одинаковый хеш\nN = U = 4 096"),
             ("outputs10", 100000, "Десять выдач\nN = U = 100 000")]
    figure, axes = plt.subplots(1, 4, figsize=(12, 4.6), layout="constrained")
    figure.suptitle("Из чего складывается время", fontsize=16, fontweight="bold")
    for axis, (scenario, count, title) in zip(axes, cases):
        selected = {row["dictionary"]: row for row in rows
                    if row["scenario"] == scenario and row["n"] == count}
        if len(selected) != 2:
            plt.close(figure)
            return
        for position, kind in enumerate(NAMES):
            row = selected[kind]
            axis.bar(position, row["build_ms"], color=COLORS[kind], width=0.58)
            axis.bar(position, row["entries_ms"], bottom=row["build_ms"],
                     color=COLORS[kind], alpha=0.4, hatch="///", width=0.58)
            axis.annotate(f"{row['total_ms']:.2f}", (position, row["total_ms"]),
                          xytext=(0, 5), textcoords="offset points", ha="center", fontsize=10)
        axis.set(title=title, ylabel="Время, мс", xticks=[0, 1], xticklabels=["AVL", "Хеш"])
        axis.set_ylim(0, max(row["total_ms"] for row in selected.values()) * 1.23)
        axis.yaxis.set_major_formatter(FuncFormatter(lambda value, _: f"{value:g}"))
        axis.grid(axis="y", color="#DEE4E7", linewidth=0.8)
        axis.set_axisbelow(True)
    figure.supxlabel("Сплошной цвет — построение · штриховка — выдача · шкала времени своя в каждой панели", fontsize=10)
    save_figure(figure, folder, "index_stages")


def main():
    parser = argparse.ArgumentParser(description="Строю графики по CSV программы или экспорту окна.")
    parser.add_argument("csv", nargs="?", type=Path, default=PROJECT / "index_performance.csv")
    parser.add_argument("--output", type=Path, default=PROJECT / "Results")
    args = parser.parse_args()
    try:
        rows = load_results(args.csv)
    except (OSError, ValueError, KeyError, TypeError) as error:
        parser.error(str(error))
    args.output.mkdir(parents=True, exist_ok=True)
    plt.rcParams.update({"font.family": "DejaVu Sans", "font.size": 11,
                         "axes.spines.top": False, "axes.spines.right": False,
                         "axes.labelcolor": "#34444D", "text.color": "#24343D",
                         "axes.titlepad": 14, "axes.axisbelow": True})
    for scenario, title in (("file", "Алфавитный указатель · текст из файла"),
                            ("collisions", "Алфавитный указатель · одинаковый хеш")):
        selected = [row for row in rows if row["scenario"] == scenario]
        if selected:
            plot_scaling(selected, args.output, f"index_{scenario}", title)
    plot_stages(rows, args.output)
    # Разные размеры и режимы страниц показываю на отдельных графиках.
    groups = sorted({(row["page_size"], row["mode"]) for row in rows if row["scenario"] == "gui"})
    for size, mode in groups:
        selected = [row for row in rows if row["scenario"] == "gui"
                    and row["page_size"] == size and row["mode"] == mode]
        unit = "слов" if mode == "words" else "байт UTF-8"
        plot_scaling(selected, args.output, f"measurements_{mode}_{size}",
                     f"Замеры из окна · страница {size} {unit}")


if __name__ == "__main__":
    main()
