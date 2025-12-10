# JSON Export - Implementation Steps

## Objective

Export coordinates in JSON format with complete structure and metadata.

## Prerequisites

- DocumentState with regions and groups
- Qt JSON support (QJsonDocument)
- File I/O working

## JSON Structure

```json
{
  "pdf_path": "string",
  "image_size": [width, height],
  "aspect_ratio": 0.6667,
  "regions": {
    "Region Name": {
      "normalized_coords": {"x1": 0.25, "y1": 0.10, "x2": 0.35, "y2": 0.20},
      "color": "blue",
      "group": "lastname"
    }
  },
  "groups": {
    "lastname": ["Region Name 1", "Region Name 2"]
  }
}
```

## Key Points

- Use normalized coordinates only (0.0-1.0)
- Include all metadata (PDF path, image size, aspect ratio)
- Pretty-print with indentation
- Validate JSON before writing

## Acceptance Criteria

- [ ] JSON file is created correctly
- [ ] JSON structure matches specification
- [ ] Only normalized coordinates are exported
- [ ] All metadata is included
- [ ] File is readable and valid JSON

## See Also

- `IMPLEMENTATION_STEPS.md` for detailed step-by-step instructions
