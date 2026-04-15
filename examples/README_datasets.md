# DRC Layout Datasets for Performance Testing

This directory contains realistic JSON layout files designed for testing and benchmarking the DRC (Design Rule Check) engine performance and correctness.

## Dataset Overview

### dataset_100.json
- **Size**: ~100 objects
- **Purpose**: Small dataset for quick testing and correctness validation
- **Characteristics**:
  - Mixed geometric shapes (rectangles, trapezoids, polygons)
  - Multiple layers (metal1, metal2, via)
  - Intentional rule violations for testing
  - Compact size for fast processing

### dataset_1000.json
- **Size**: ~1000 objects
- **Purpose**: Medium dataset for performance benchmarking
- **Characteristics**:
  - Balanced distribution of shapes and layers
  - Realistic spatial distribution
  - Suitable for comparing different spatial indexing algorithms

### dataset_10000.json
- **Size**: ~10,000 objects
- **Purpose**: Large dataset for stress testing and performance analysis
- **Characteristics**:
  - High object density
  - Complex spatial relationships
  - Ideal for benchmarking brute-force vs. optimized approaches

## Shape Distribution

All datasets follow this distribution:
- **Rectangles**: 50% (most common geometric shape)
- **Trapezoids**: 30% (common in semiconductor layouts)
- **Polygons**: 20% (complex shapes for advanced testing)

## Layer Distribution

Objects are distributed across three layers:
- **metal1**: Lower metal layer
- **metal2**: Upper metal layer
- **via**: Connection layer between metal layers

## Intentional Violations

Each dataset includes intentional rule violations to test DRC correctness:
- **Width violations**: Objects narrower than 5 units (minWidth)
- **Height violations**: Objects shorter than 5 units (minHeight)
- **Area violations**: Objects smaller than 25 square units (minArea)
- **Spacing violations**: Objects placed closer than minimum spacing requirements
- **Overlapping violations**: Objects that intersect inappropriately

## Rule Configuration

The datasets use default DRC rule thresholds:
- **Minimum Width**: 5 units
- **Minimum Height**: 5 units
- **Minimum Area**: 25 square units
- **Minimum Spacing**: 10 units (layer 0), 5 units (layer 1)
- **Enclosure Rules**:
  - Via shapes must be enclosed by metal1 with 2.0 unit margin
  - Via shapes must be enclosed by metal2 with 2.0 unit margin

## Validation Results

Testing with `dataset_100.json` shows violations across all rule types:
- **Enclosure**: 68 violations (vias not properly enclosed by metal layers)
- **IntersectionRule**: 6 violations (overlapping shapes)
- **MinArea**: 6 violations (area < 25 units²)
- **MinSpacing**: 2 violations (insufficient spacing)
- **MinWidth**: 9 violations (width/height < 5 units)
- **Total**: 91 violations detected

## Usage for Benchmarking

### Performance Comparison
Use these datasets to compare different DRC engine implementations:

1. **Brute-force approach**: Check all pairs of objects (O(n²) complexity)
2. **R-tree spatial indexing**: Hierarchical spatial indexing (O(n log n) query time)
3. **Quadtree spatial indexing**: Grid-based spatial partitioning

### Expected Performance Scaling
- **dataset_100**: Fast processing with all approaches
- **dataset_1000**: Noticeable difference between algorithms
- **dataset_10000**: Clear performance separation between optimized and brute-force methods

### Benchmarking Commands
```bash
# Test with different datasets
./cli_drc examples/dataset_100.json
./cli_drc examples/dataset_1000.json
./cli_drc examples/dataset_10000.json

# Performance comparison script
python3 performance_comparison.py
```

## File Format

Each JSON file follows this structure:
```json
{
  "layout_name": "dataset_100",
  "objects": [
    {
      "type": "rectangle",
      "layer": "metal1",
      "x": 100,
      "y": 200,
      "width": 20,
      "height": 10
    },
    {
      "type": "trapezoid",
      "layer": "metal1",
      "points": [
        {"x": 10, "y": 10},
        {"x": 30, "y": 10},
        {"x": 25, "y": 20},
        {"x": 15, "y": 20}
      ]
    },
    {
      "type": "polygon",
      "layer": "metal2",
      "points": [
        {"x": 50, "y": 50},
        {"x": 70, "y": 55},
        {"x": 65, "y": 70},
        {"x": 45, "y": 65}
      ]
    }
  ]
}
```

## Generation Script

The datasets were generated using `generate_layouts.py` which:
- Uses deterministic seeding for reproducible results
- Ensures realistic spatial distribution
- Includes controlled violation injection
- Maintains consistent shape and layer distributions

## Thesis Applications

These datasets are specifically designed for thesis work comparing:
- **Algorithm efficiency**: Brute-force vs. spatial indexing
- **Scalability analysis**: Performance degradation with dataset size
- **Correctness validation**: Detection of various DRC violations
- **GUI visualization**: Rendering performance with different object counts

## Validation

To validate the datasets work with your DRC engine:
```bash
# Load and check for parsing errors
./cli_drc examples/dataset_100.json --validate-only

# Run full DRC check
./cli_drc examples/dataset_100.json --rules rules.json

# GUI visualization
./cli_drc_gui examples/dataset_100.json
```