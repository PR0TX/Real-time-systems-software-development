import argparse
import csv
from pathlib import Path
import subprocess

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt


TARGET = 0.05
DURATION_MS = 20_000

BASELINE = ("Без затримок", 5.0, 50, 0, 0)
NOMINAL = ("Затримка 150 мс", 5.0, 50, 100, 50)
VARIANTS = [
    ("Kp = 12", 12.0, 50, 100, 50),
    ("T = 25 мс", 5.0, 25, 100, 50),
    ("T = 100 мс\nЗатримка 300 мс", 5.0, 100, 200, 100),
    ("T = 50 мс\nЗатримка 300 мс", 5.0, 50, 200, 100),
]


def simulate(folder, name, case, compensate):
    label, kp, period, actuation, feedback = case
    path = folder / f"{name}.csv"
    params = {
        "kp": kp,
        "period_ms": period,
        "actuation_ms": actuation,
        "feedback_ms": feedback,
        "target": TARGET,
        "duration_ms": DURATION_MS,
        "compensate": str(compensate).lower(),
        "output": path,
    }
    command = [str(Path(__file__).with_name("simulate_joint")), "--ros-args"]
    for key, value in params.items():
        command.extend(["-p", f"{key}:={value}"])
    subprocess.run(command, check=True)
    with path.open(newline="") as source:
        rows = list(csv.DictReader(source))
    return label, {
        key: [float(row[key]) for row in rows]
        for key in ("time_s", "position_rad", "velocity_rad_s")
    }


def decorate(axes):
    for ax in axes:
        ax[0].axhline(TARGET, color="0.4", linestyle=":", linewidth=1, label="Ціль")
        ax[0].set_ylabel("Положення, рад")
        ax[1].set_ylabel("Швидкість, рад/с")
        ax[1].set_xlabel("Час, с")
        for panel in ax:
            panel.grid(alpha=0.25)
            panel.legend(fontsize=8)


def plot_pair(axes, data, label, xlim=None):
    for ax, key in zip(axes, ("position_rad", "velocity_rad_s")):
        ax.plot(data["time_s"], data[key], label=label)
        if xlim is not None:
            ax.set_xlim(0, xlim)


def write_metrics(writer, scenario, case, data, compensate):
    positions = data["position_rad"]
    times = data["time_s"]
    tolerance = 0.02 * abs(TARGET)
    outside = [i for i, x in enumerate(positions) if abs(x - TARGET) > tolerance]
    first_stable = outside[-1] + 1 if outside else 0
    settle = times[first_stable] if first_stable < len(times) else ""
    tail = [x for t, x in zip(times, positions) if t >= times[-1] - 5.0]
    writer.writerow([
        scenario, *case[1:], int(compensate), max(0.0, max(positions) - TARGET),
        settle, max(tail) - min(tail),
    ])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output_dir", type=Path)
    folder = parser.parse_args().output_dir.resolve()
    folder.mkdir(parents=True, exist_ok=True)
    with (folder / "metrics.csv").open("w", newline="") as output:
        writer = csv.writer(output)
        writer.writerow([
            "scenario", "kp", "period_ms", "actuation_ms", "feedback_ms", "compensate",
            "overshoot_rad", "settling_time_2pct_s", "last_5s_peak_to_peak_rad",
        ])

        fig, axes = plt.subplots(2, 1, sharex=True, figsize=(9, 6), layout="constrained")
        for name, case, compensate in [
            ("no_delay", BASELINE, False),
            ("nominal", NOMINAL, False),
            ("nominal_predicted", NOMINAL, True),
        ]:
            label, data = simulate(folder, name, case, compensate)
            plot_pair(axes, data, f"{label} з предиктором" if compensate else label, xlim=3)
            write_metrics(writer, name, case, data, compensate)
        decorate([axes])
        fig.savefig(folder / "nominal.png", dpi=160)
        plt.close(fig)

        fig, axes = plt.subplots(2, 4, sharex=True, figsize=(16, 6), layout="constrained")
        for name, case, pair in zip(
            ("high_kp", "fast_sampling", "slow_sampling", "long_delay"), VARIANTS, axes.T
        ):
            pair[0].set_title(case[0])
            for suffix, compensate in [("", False), ("_predicted", True)]:
                scenario = name + suffix
                _, data = simulate(folder, scenario, case, compensate)
                plot_pair(pair, data, "З предиктором" if compensate else "П-регулятор")
                write_metrics(writer, scenario, case, data, compensate)
        decorate(axes.T)
        fig.savefig(folder / "parameter_sweep.png", dpi=160)
        plt.close(fig)
    print(folder)


if __name__ == "__main__":
    main()
