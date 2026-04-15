#!/usr/bin/env python3
"""
DRC Dataset Analyzer

This script analyzes the generated JSON layout datasets and provides statistics
useful for performance benchmarking and thesis work.
"""

import json
import sys
from typing import Dict, List, Any
from collections import defaultdict
import math

def load_layout(filename: str) -> Dict[str, Any]:
    """Load a layout from JSON file"""
    with open(filename, 'r') as f:
        return json.load(f)

def analyze_layout(layout: Dict[str, Any]) -> Dict[str, Any]:
    """Analyze a layout and return statistics"""
    objects = layout["objects"]
    stats = {
        "total_objects": len(objects),
        "shape_counts": defaultdict(int),
        "layer_counts": defaultdict(int),
        "violations": {
            "too_small_width": 0,
            "too_small_height": 0,
            "too_small_area": 0
        },
        "spatial_stats": {
            "min_x": float('inf'),
            "max_x": float('-inf'),
            "min_y": float('inf'),
            "max_y": float('-inf'),
            "total_area": 0
        }
    }

    for obj in objects:
        # Shape and layer counts
        stats["shape_counts"][obj["type"]] += 1
        stats["layer_counts"][obj["layer"]] += 1

        # Spatial bounds
        if obj["type"] == "rectangle":
            x, y, w, h = obj["x"], obj["y"], obj["width"], obj["height"]
            stats["spatial_stats"]["min_x"] = min(stats["spatial_stats"]["min_x"], x)
            stats["spatial_stats"]["max_x"] = max(stats["spatial_stats"]["max_x"], x + w)
            stats["spatial_stats"]["min_y"] = min(stats["spatial_stats"]["min_y"], y)
            stats["spatial_stats"]["max_y"] = max(stats["spatial_stats"]["max_y"], y + h)
            area = w * h
            stats["spatial_stats"]["total_area"] += area

            # Check for violations (assuming min width/height = 5, min area = 25)
            if w < 5:
                stats["violations"]["too_small_width"] += 1
            if h < 5:
                stats["violations"]["too_small_height"] += 1
            if area < 25:
                stats["violations"]["too_small_area"] += 1

        elif obj["type"] in ["trapezoid", "polygon"]:
            points = obj["points"]
            xs = [p["x"] for p in points]
            ys = [p["y"] for p in points]

            stats["spatial_stats"]["min_x"] = min(stats["spatial_stats"]["min_x"], min(xs))
            stats["spatial_stats"]["max_x"] = max(stats["spatial_stats"]["max_x"], max(xs))
            stats["spatial_stats"]["min_y"] = min(stats["spatial_stats"]["min_y"], min(ys))
            stats["spatial_stats"]["max_y"] = max(stats["spatial_stats"]["max_y"], max(ys))

            # Approximate area for polygons (shoelace formula)
            if len(points) >= 3:
                area = 0
                n = len(points)
                for i in range(n):
                    j = (i + 1) % n
                    area += xs[i] * ys[j]
                    area -= xs[j] * ys[i]
                area = abs(area) / 2
                stats["spatial_stats"]["total_area"] += area

    # Calculate derived statistics
    width = stats["spatial_stats"]["max_x"] - stats["spatial_stats"]["min_x"]
    height = stats["spatial_stats"]["max_y"] - stats["spatial_stats"]["min_y"]
    stats["spatial_stats"]["bounding_box_area"] = width * height
    stats["spatial_stats"]["density"] = stats["spatial_stats"]["total_area"] / stats["spatial_stats"]["bounding_box_area"]

    return stats

def print_analysis(filename: str, stats: Dict[str, Any]):
    """Print analysis results"""
    print(f"\n=== Analysis of {filename} ===")
    print(f"Total objects: {stats['total_objects']}")

    print("\nShape distribution:")
    for shape, count in stats["shape_counts"].items():
        pct = (count / stats["total_objects"]) * 100
        print(".1f")

    print("\nLayer distribution:")
    for layer, count in stats["layer_counts"].items():
        pct = (count / stats["total_objects"]) * 100
        print(".1f")

    print("\nSpatial statistics:")
    spatial = stats["spatial_stats"]
    print(f"  Bounding box: ({spatial['min_x']:.0f}, {spatial['min_y']:.0f}) to ({spatial['max_x']:.0f}, {spatial['max_y']:.0f})")
    print(f"  Dimensions: {spatial['max_x'] - spatial['min_x']:.0f} x {spatial['max_y'] - spatial['min_y']:.0f}")
    print(".2f")
    print(".4f")

    print("\nPotential violations:")
    violations = stats["violations"]
    print(f"  Too small width: {violations['too_small_width']}")
    print(f"  Too small height: {violations['too_small_height']}")
    print(f"  Too small area: {violations['too_small_area']}")

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 analyze_datasets.py <dataset_file1.json> [dataset_file2.json] ...")
        sys.exit(1)

    for filename in sys.argv[1:]:
        try:
            layout = load_layout(filename)
            stats = analyze_layout(layout)
            print_analysis(filename, stats)
        except Exception as e:
            print(f"Error analyzing {filename}: {e}")

if __name__ == "__main__":
    main()