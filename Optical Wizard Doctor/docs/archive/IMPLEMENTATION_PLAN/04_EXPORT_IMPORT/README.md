# Phase 4: Export/Import - Implementation Steps

## Objective

Enable users to export coordinates in multiple formats and import saved calibrations.

## Prerequisites

- Phase 3 complete (all region operations working)
- DocumentState with complete data
- File I/O ready

## Sub-Projects

1. **Export** - Export coordinates in JSON, CSV, TXT, XML, YAML
2. **Import** - Import coordinates from JSON
3. **Mask** - Generate mask images for OCR

## Order of Implementation

1. JSON Export (primary format)
2. CSV Export (simple format)
3. JSON Import (needs export first)
4. Other Export Formats (TXT, XML, YAML)
5. Mask Generation

## Success Criteria

- Users can export coordinates in all formats
- Users can import saved coordinates
- Exports use normalized coordinates only
- Imports validate and restore state correctly
- Mask images are generated correctly

## Estimated Time

2-3 weeks
