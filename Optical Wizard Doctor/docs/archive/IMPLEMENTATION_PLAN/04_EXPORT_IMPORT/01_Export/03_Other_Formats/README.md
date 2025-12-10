# Other Export Formats - Implementation Steps

## Objective

Implement TXT, XML, and YAML export formats.

## Prerequisites

- JSON export working (for structure reference)
- DocumentState ready
- Qt XML support (built-in)
- yaml-cpp library (for YAML, optional)

## Formats

1. **TXT** - Human-readable plain text format
2. **XML** - Structured XML format
3. **YAML** - YAML format (requires yaml-cpp library)

## Key Points

- All formats use normalized coordinates only
- TXT is human-readable with clear formatting
- XML uses proper XML structure with QXmlStreamWriter
- YAML requires external library (optional dependency)
- All formats include same data as JSON export

## Acceptance Criteria

- [ ] TXT export works and is human-readable
- [ ] XML export works and is valid XML
- [ ] YAML export works (if library available)
- [ ] All formats contain same data
- [ ] All formats use normalized coordinates only

## See Also

- `IMPLEMENTATION_STEPS.md` for detailed step-by-step instructions
