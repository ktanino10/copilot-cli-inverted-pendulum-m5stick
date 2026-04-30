#!/usr/bin/env python3
"""
CERN ROOT / matplotlib visualization for inverted-pendulum PID logs.

Usage:
  python3 tools/visualize_root.py tools/data/failure_log_backward_run.csv
  python3 tools/visualize_root.py tools/data/failure_log_*.csv

Outputs for each CSV:
  - <csv>.root        ROOT file with TGraph objects (when PyROOT works)
  - <csv>_root.png    ROOT canvas image (when PyROOT works)
  - <csv>.png         matplotlib fallback/summary image

Supported formats:
  - Detailed failure log CSV:
    t,Angle,dAngle,P,I,D,k_speed,Speed,power,powerL,powerR,accX,accY,accZ,gyroX,gyroY,gyroZ
  - Legacy serial D lines:
    D,Angle,power,powerL,powerR
  - Legacy collected CSV:
    timestamp,D,Angle,power,powerL,powerR
"""

from __future__ import annotations

import csv
import glob
import os
import sys
from pathlib import Path


FIELDS = [
    "t",
    "Angle",
    "dAngle",
    "P",
    "I",
    "D",
    "k_speed",
    "Speed",
    "power",
    "powerL",
    "powerR",
    "accX",
    "accY",
    "accZ",
    "gyroX",
    "gyroY",
    "gyroZ",
]


def main() -> int:
    if len(sys.argv) < 2:
        print("Usage: python3 tools/visualize_root.py <csv_file> [<csv_file> ...]")
        print("Example: python3 tools/visualize_root.py tools/data/failure_log_*.csv")
        return 1

    csv_files: list[str] = []
    for arg in sys.argv[1:]:
        matches = glob.glob(arg)
        csv_files.extend(matches or [arg])

    for csv_file in csv_files:
        visualize(csv_file)
    return 0


def visualize(csv_file: str) -> None:
    data = load_csv(csv_file)
    if not data["t"]:
        print(f"No data found: {csv_file}")
        return

    print(f"\n=== {csv_file} ===")
    print_summary(data)

    try:
        visualize_root(csv_file, data)
    except Exception as exc:  # ROOT is useful, but frequently fragile on macOS.
        print(f"ROOT output skipped for {csv_file}: {exc}")

    visualize_matplotlib(csv_file, data)


def load_csv(csv_file: str) -> dict[str, list[float]]:
    data: dict[str, list[float]] = {field: [] for field in FIELDS}

    with open(csv_file, newline="") as f:
        sample = f.readline()
        f.seek(0)

        if sample.startswith("t,Angle,"):
            reader = csv.DictReader(f)
            for row in reader:
                append_detailed_row(data, row)
            return normalize_time(data)

        for line_index, line in enumerate(f):
            line = line.strip()
            if not line or line.startswith("timestamp"):
                continue

            parts = line.split(",")
            try:
                if parts[0] == "D":
                    append_legacy_row(
                        data,
                        t=line_index * 0.1,
                        angle=float(parts[1]),
                        power=float(parts[2]),
                        power_l=float(parts[3]) if len(parts) > 3 else 1500.0,
                        power_r=float(parts[4]) if len(parts) > 4 else 1500.0,
                    )
                else:
                    append_legacy_row(
                        data,
                        t=float(parts[0]),
                        angle=float(parts[2]),
                        power=float(parts[3]),
                        power_l=float(parts[4]) if len(parts) > 4 else 1500.0,
                        power_r=float(parts[5]) if len(parts) > 5 else 1500.0,
                    )
            except (ValueError, IndexError):
                continue

    return normalize_time(data)


def append_detailed_row(data: dict[str, list[float]], row: dict[str, str]) -> None:
    for field in FIELDS:
        try:
            data[field].append(float(row[field]))
        except (KeyError, TypeError, ValueError):
            data[field].append(0.0)


def append_legacy_row(
    data: dict[str, list[float]],
    *,
    t: float,
    angle: float,
    power: float,
    power_l: float,
    power_r: float,
) -> None:
    row = {field: 0.0 for field in FIELDS}
    row.update({"t": t, "Angle": angle, "power": power, "powerL": power_l, "powerR": power_r})
    for field in FIELDS:
        data[field].append(row[field])


def normalize_time(data: dict[str, list[float]]) -> dict[str, list[float]]:
    if data["t"]:
        t0 = data["t"][0]
        data["t"] = [t - t0 for t in data["t"]]
    return data


def print_summary(data: dict[str, list[float]]) -> None:
    angle = data["Angle"]
    power = data["power"]
    print(f"Data points: {len(angle)}")
    print(f"Duration: {data['t'][-1]:.2f} s")
    print(f"Angle range: [{min(angle):.1f}, {max(angle):.1f}] deg")
    print(f"Power range: [{min(power):.0f}, {max(power):.0f}] us")


def visualize_root(csv_file: str, data: dict[str, list[float]]) -> None:
    import ROOT

    ROOT.gROOT.SetBatch(True)

    root_file = csv_file.replace(".csv", ".root")
    png_file = csv_file.replace(".csv", "_root.png")
    f = ROOT.TFile(root_file, "RECREATE")

    graphs = {
        "Angle": make_graph(ROOT, data, "Angle", "Angle vs Time;Time [s];Angle [deg]", ROOT.kBlue),
        "dAngle": make_graph(ROOT, data, "dAngle", "dAngle vs Time;Time [s];dAngle [deg/s]", ROOT.kCyan + 2),
        "power": make_graph(ROOT, data, "power", "Power vs Time;Time [s];Power", ROOT.kRed),
        "P": make_graph(ROOT, data, "P", "P term vs Time;Time [s];P", ROOT.kGreen + 2),
        "D": make_graph(ROOT, data, "D", "D term vs Time;Time [s];D", ROOT.kMagenta + 2),
        "accZ": make_graph(ROOT, data, "accZ", "accZ vs Time;Time [s];accZ [g]", ROOT.kOrange + 7),
        "gyroX": make_graph(ROOT, data, "gyroX", "gyroX vs Time;Time [s];gyroX [deg/s]", ROOT.kViolet),
    }

    g_phase = ROOT.TGraph(len(data["t"]))
    g_phase.SetName("g_angle_dangle")
    g_phase.SetTitle("Phase: Angle vs dAngle;Angle [deg];dAngle [deg/s]")
    for i, (angle, dangle) in enumerate(zip(data["Angle"], data["dAngle"])):
        g_phase.SetPoint(i, angle, dangle)
    g_phase.SetMarkerStyle(6)
    g_phase.SetMarkerColor(ROOT.kBlack)
    graphs["angle_dangle"] = g_phase

    c = ROOT.TCanvas("c_failure_analysis", "Failure Log Analysis", 1600, 1200)
    c.Divide(2, 3)

    c.cd(1)
    graphs["Angle"].Draw("AL")
    draw_zero_line(ROOT, data["t"], data["Angle"])

    c.cd(2)
    graphs["power"].Draw("AL")
    draw_zero_line(ROOT, data["t"], data["power"])

    c.cd(3)
    graphs["P"].Draw("AL")
    graphs["D"].Draw("L SAME")

    c.cd(4)
    graphs["angle_dangle"].Draw("AP")

    c.cd(5)
    graphs["accZ"].Draw("AL")

    c.cd(6)
    graphs["gyroX"].Draw("AL")
    draw_zero_line(ROOT, data["t"], data["gyroX"])

    c.Write()
    for graph in graphs.values():
        graph.Write()
    f.Close()
    c.SaveAs(png_file)

    print(f"ROOT file: {root_file}")
    print(f"ROOT PNG: {png_file}")


def make_graph(ROOT, data: dict[str, list[float]], field: str, title: str, color: int):
    graph = ROOT.TGraph(len(data["t"]))
    graph.SetName(f"g_{field}")
    graph.SetTitle(title)
    for i, (t, value) in enumerate(zip(data["t"], data[field])):
        graph.SetPoint(i, t, value)
    graph.SetLineColor(color)
    graph.SetMarkerStyle(1)
    return graph


def draw_zero_line(ROOT, times: list[float], values: list[float]) -> None:
    if not times:
        return
    if min(values) <= 0 <= max(values):
        line = ROOT.TLine(times[0], 0, times[-1], 0)
        line.SetLineStyle(2)
        line.SetLineColor(ROOT.kGray + 2)
        line.Draw()


def visualize_matplotlib(csv_file: str, data: dict[str, list[float]]) -> None:
    import matplotlib

    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    png_file = csv_file.replace(".csv", ".png")
    t = data["t"]

    fig, axes = plt.subplots(3, 2, figsize=(14, 11), sharex=False)
    fig.suptitle(f"Failure Log Analysis: {os.path.basename(csv_file)}")

    axes[0, 0].plot(t, data["Angle"], color="tab:blue", linewidth=1)
    axes[0, 0].axhline(0, color="gray", linestyle="--", linewidth=0.8)
    axes[0, 0].axhline(45, color="red", linestyle=":", linewidth=0.8)
    axes[0, 0].axhline(-45, color="red", linestyle=":", linewidth=0.8)
    axes[0, 0].set_title("Angle vs Time")
    axes[0, 0].set_ylabel("Angle [deg]")

    axes[0, 1].plot(t, data["power"], color="tab:red", linewidth=1, label="power")
    axes[0, 1].plot(t, [p - 1500 for p in data["powerL"]], color="tab:orange", alpha=0.7, label="L-1500")
    axes[0, 1].plot(t, [p - 1500 for p in data["powerR"]], color="tab:purple", alpha=0.7, label="R-1500")
    axes[0, 1].axhline(0, color="gray", linestyle="--", linewidth=0.8)
    axes[0, 1].set_title("Motor Output vs Time")
    axes[0, 1].set_ylabel("Power")
    axes[0, 1].legend(loc="best", fontsize=8)

    axes[1, 0].plot(t, data["P"], label="P", linewidth=1)
    axes[1, 0].plot(t, data["D"], label="D", linewidth=1)
    axes[1, 0].plot(t, data["I"], label="I", linewidth=1)
    axes[1, 0].axhline(0, color="gray", linestyle="--", linewidth=0.8)
    axes[1, 0].set_title("PID Terms")
    axes[1, 0].set_ylabel("Term value")
    axes[1, 0].legend(loc="best", fontsize=8)

    axes[1, 1].scatter(data["Angle"], data["dAngle"], s=8, alpha=0.7, c=t, cmap="viridis")
    axes[1, 1].axhline(0, color="gray", linestyle="--", linewidth=0.8)
    axes[1, 1].axvline(0, color="gray", linestyle="--", linewidth=0.8)
    axes[1, 1].set_title("Phase: Angle vs dAngle")
    axes[1, 1].set_xlabel("Angle [deg]")
    axes[1, 1].set_ylabel("dAngle [deg/s]")

    axes[2, 0].plot(t, data["accZ"], label="accZ", linewidth=1)
    axes[2, 0].plot(t, data["gyroX"], label="gyroX", linewidth=1)
    axes[2, 0].axhline(0, color="gray", linestyle="--", linewidth=0.8)
    axes[2, 0].set_title("IMU Signals")
    axes[2, 0].set_xlabel("Time [s]")
    axes[2, 0].legend(loc="best", fontsize=8)

    axes[2, 1].scatter(data["Angle"], data["power"], s=8, alpha=0.7, c=t, cmap="plasma")
    axes[2, 1].axhline(0, color="gray", linestyle="--", linewidth=0.8)
    axes[2, 1].axvline(0, color="gray", linestyle="--", linewidth=0.8)
    axes[2, 1].set_title("Angle vs Power")
    axes[2, 1].set_xlabel("Angle [deg]")
    axes[2, 1].set_ylabel("Power")

    for ax in axes.flat:
        ax.grid(True, alpha=0.25)

    fig.tight_layout()
    fig.savefig(png_file, dpi=160)
    plt.close(fig)
    print(f"PNG file: {png_file}")


if __name__ == "__main__":
    raise SystemExit(main())
