# DRC Examples

This folder contains layout examples for testing the Design Rule Check (DRC) system.

## Example Files

### `semiconductor_design.json`
**Real-world comprehensive example** with multiple rule types.

**Features:**
- 5 layers: polysilicon, metal1, metal2, contact, via
- Per-layer minWidth, minHeight, minArea constraints
- Multiple enclosure rules (inner layer must be enclosed by outer layer with margin)
- Multiple containment rules (one layer must be contained within another)
- 16 shapes total with intentional violations

**Expected Violations:**
- Enclosure violations: 7
- Containment violations: 5
- MinWidth violations: 15
- MinArea violations: 4

**Total:** 31 violations across all rule types (verified by `test_examples.sh`)

**How to use:**
```bash
./build/bin/drc_demo examples/semiconductor_design.json
./build/bin/cli_drc_gui examples/semiconductor_design.json
```

Then in GUI: File > Open > `examples/semiconductor_design.json` > Run > Execute DRC

---

### `intersection_demo.json`
**Simple demo** for testing intersection and width rules.

**Features:**
- 2 layers: metal1, metal2
- Per-layer minWidth and minArea constraints
- 8 shapes with some overlapping regions

**Expected Violations:**
When standalone: 0 violations (no rules defined)
When in GUI with demo rules: Intersection and MinSpacing violations

**How to use:**
```bash
./build/bin/drc_demo examples/intersection_demo.json
./run_gui.sh examples/intersection_demo.json
```

---

### `basic_spacing_violations.json`
**Spacing rule focused example** (designed for future spacing rule additions).

**Features:**
- 2 layers: metal1, metal2
- Per-layer minWidth and minArea constraints
- 10 shapes with various spacing patterns
- Some shapes are close together (< 30 units on metal1, < 40 on metal2)

**How to use:**
```bash
./build/bin/drc_demo examples/basic_spacing_violations.json
```

---

## JSON Format

All files follow this structure:

```json
{
  "layers": {
    "layer_name": {
      "minWidth": 100.0,
      "minHeight": 100.0,
      "minArea": 5000.0
    }
  },
  
  "rules": {
    "enclosure": [
      {
        "inner": "poly",
        "outer": "metal1",
        "margin": 50.0
      }
    ],
    "containment": [
      {
        "layerA": "poly",
        "layerB": "metal1"
      }
    ]
  },
  
  "shapes": [
    {
      "id": 1,
      "layer": "layer_name",
      "rect": {
        "x1": 0,
        "y1": 0,
        "x2": 100,
        "y2": 100
      }
    }
  ]
}
```

### Key Fields:
- **layers**: Defines available layers and their minimum constraints
- **rules**: Enclosure and containment rules (optional)
- **shapes**: Layout geometry with coordinates

---

## Testing

### Run All Examples Test
```bash
./test_examples.sh              # Verify all examples work correctly
```

This script tests each example and reports:
- File existence
- Violation counts
- Rule parsing correctness

### CLI Testing
```bash
./build/bin/rule_tests              # Unit tests
./build/bin/drc_demo <file.json>    # Detailed output with all violations
```

### GUI Testing
```bash
./run_gui.sh <file.json>            # Launch with preset layout
# or
./build/bin/cli_drc_gui             # Launch GUI, then File > Open
```


---

## Rule Types Supported

1. **MinWidth/Height**: Each shape must meet minimum width and height on its layer
2. **MinArea**: Each shape must have minimum area on its layer
3. **Enclosure**: Inner shapes must be enclosed by outer shapes with specified margin
4. **Containment**: Shapes on one layer must be contained within shapes on another layer
5. **MinSpacing** (demo only): Shapes on same layer must have minimum distance
6. **Intersection** (demo only): Shapes on different layers must not overlap

---

## Creating Your Own Examples

Use `semiconductor_design.json` as a template:

1. Define your layers
2. Set per-layer constraints (minWidth, minHeight, minArea)
3. Define rules (enclosure, containment)
4. Add shapes with coordinates

Run: `./build/bin/drc_demo yourfile.json`

Then visualize in GUI: `./run_gui.sh yourfile.json`
