#!/usr/bin/env python3
"""
JSON Layout Generator for DRC Performance Testing

This script generates realistic JSON layout files with mixed geometric shapes
for testing DRC (Design Rule Check) engine performance and correctness.

Features:
- Generates rectangles, trapezoids, and polygons
- Distributes objects across multiple layers
- Includes intentional rule violations for testing
- Configurable dataset sizes
- Random but realistic placement
"""

import json
import random
import math
from typing import List, Dict, Any, Tuple

class LayoutGenerator:
    def __init__(self, seed: int = 42):
        random.seed(seed)
        self.layers = ["metal1", "metal2", "via"]
        self.shape_types = ["rectangle", "trapezoid", "polygon"]
        self.weights = [0.5, 0.3, 0.2]  # 50% rectangles, 30% trapezoids, 20% polygons

    def generate_rectangle(self, layer: str, x: int, y: int, width: int, height: int) -> Dict[str, Any]:
        """Generate a rectangle object"""
        return {
            "type": "rectangle",
            "layer": layer,
            "x": x,
            "y": y,
            "width": width,
            "height": height
        }

    def generate_trapezoid(self, layer: str, x: int, y: int, base_width: int, top_width: int, height: int) -> Dict[str, Any]:
        """Generate a trapezoid object"""
        # Ensure top_width <= base_width for a valid trapezoid
        if top_width > base_width:
            base_width, top_width = top_width, base_width

        offset = (base_width - top_width) // 2
        return {
            "type": "trapezoid",
            "layer": layer,
            "points": [
                {"x": x, "y": y},
                {"x": x + base_width, "y": y},
                {"x": x + base_width - offset, "y": y + height},
                {"x": x + offset, "y": y + height}
            ]
        }

    def generate_polygon(self, layer: str, x: int, y: int, width: int, height: int, sides: int = 5) -> Dict[str, Any]:
        """Generate a polygon object"""
        center_x = x + width // 2
        center_y = y + height // 2
        radius = min(width, height) // 2

        points = []
        for i in range(sides):
            angle = 2 * math.pi * i / sides
            px = int(center_x + radius * math.cos(angle))
            py = int(center_y + radius * math.sin(angle))
            points.append({"x": px, "y": py})

        return {
            "type": "polygon",
            "layer": layer,
            "points": points
        }

    def get_random_dimensions(self, min_size: int = 5, max_size: int = 30) -> Tuple[int, int]:
        """Get random width and height"""
        width = random.randint(min_size, max_size)
        height = random.randint(min_size, max_size)
        return width, height

    def get_random_position(self, max_x: int = 1000, max_y: int = 1000) -> Tuple[int, int]:
        """Get random position within bounds"""
        x = random.randint(0, max_x)
        y = random.randint(0, max_y)
        return x, y

    def generate_objects(self, count: int, max_area: int = 1000000) -> List[Dict[str, Any]]:
        """Generate a list of objects"""
        objects = []
        max_x = int(math.sqrt(max_area))
        max_y = max_x

        for i in range(count):
            # Choose shape type based on weights
            shape_type = random.choices(self.shape_types, weights=self.weights)[0]
            layer = random.choice(self.layers)

            x, y = self.get_random_position(max_x, max_y)

            if shape_type == "rectangle":
                width, height = self.get_random_dimensions()
                # Occasionally create violations (too small)
                if random.random() < 0.1:  # 10% chance of violation
                    width = random.randint(1, 4)  # Too narrow
                if random.random() < 0.1:
                    height = random.randint(1, 4)  # Too short
                obj = self.generate_rectangle(layer, x, y, width, height)

            elif shape_type == "trapezoid":
                base_width, height = self.get_random_dimensions()
                top_width = random.randint(max(1, base_width - 10), base_width)
                obj = self.generate_trapezoid(layer, x, y, base_width, top_width, height)

            else:  # polygon
                width, height = self.get_random_dimensions()
                sides = random.randint(5, 8)
                obj = self.generate_polygon(layer, x, y, width, height, sides)

            objects.append(obj)

        return objects

    def generate_layout(self, name: str, object_count: int) -> Dict[str, Any]:
        """Generate a complete layout"""
        objects = self.generate_objects(object_count)
        
        # Add enclosure rules for semiconductor layout
        rules = {
            "enclosure": [
                {
                    "inner": "via",
                    "outer": "metal1", 
                    "margin": 2.0
                },
                {
                    "inner": "via",
                    "outer": "metal2",
                    "margin": 2.0
                }
            ]
        }
        
        return {
            "layout_name": name,
            "objects": objects,
            "rules": rules
        }

    def save_layout(self, layout: Dict[str, Any], filename: str):
        """Save layout to JSON file"""
        with open(filename, 'w') as f:
            json.dump(layout, f, indent=2)

def main():
    generator = LayoutGenerator(seed=42)

    # Generate dataset_100.json (already created manually, but can regenerate)
    print("Generating dataset_100.json...")
    layout_100 = generator.generate_layout("dataset_100", 100)
    generator.save_layout(layout_100, "/home/arshak/Desktop/Myfile/Synopsys/Thesis/CLI_DRC/examples/dataset_100.json")

    # Generate dataset_1000.json
    print("Generating dataset_1000.json...")
    layout_1000 = generator.generate_layout("dataset_1000", 1000)
    generator.save_layout(layout_1000, "/home/arshak/Desktop/Myfile/Synopsys/Thesis/CLI_DRC/examples/dataset_1000.json")

    # Generate dataset_10000.json
    print("Generating dataset_10000.json...")
    layout_10000 = generator.generate_layout("dataset_10000", 10000)
    generator.save_layout(layout_10000, "/home/arshak/Desktop/Myfile/Synopsys/Thesis/CLI_DRC/examples/dataset_10000.json")

    print("All datasets generated successfully!")

    # Print statistics
    for name, layout in [("100", layout_100), ("1000", layout_1000), ("10000", layout_10000)]:
        objects = layout["objects"]
        shape_counts = {}
        layer_counts = {}

        for obj in objects:
            shape_counts[obj["type"]] = shape_counts.get(obj["type"], 0) + 1
            layer_counts[obj["layer"]] = layer_counts.get(obj["layer"], 0) + 1

        print(f"\nDataset {name}:")
        print(f"  Total objects: {len(objects)}")
        print(f"  Shape distribution: {shape_counts}")
        print(f"  Layer distribution: {layer_counts}")

if __name__ == "__main__":
    main()