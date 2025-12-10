# JSON Import - Implementation Steps

## Objective

Import coordinates from JSON files and restore application state.

## Prerequisites

- JSON export working (for format reference)
- DocumentState ready
- Qt JSON support

## Key Features

- Read and parse JSON file
- Validate JSON structure
- Load regions with normalized coordinates
- Load groups (or reconstruct from region data)
- Support both coordinate formats (dict and list)
- Calculate missing coordinates if image loaded
- Handle import errors gracefully

## Import Process

1. Read JSON file
2. Parse and validate structure
3. Load regions (handle coordinate formats)
4. Load groups (or reconstruct)
5. Calculate image/canvas coordinates if image loaded
6. Update application state
7. Update UI

## Acceptance Criteria

- [ ] JSON files can be imported
- [ ] Regions are restored correctly
- [ ] Groups are restored correctly
- [ ] Both coordinate formats supported
- [ ] Errors are handled gracefully
- [ ] Round-trip works (export → import)

## See Also

- `IMPLEMENTATION_STEPS.md` for detailed step-by-step instructions
