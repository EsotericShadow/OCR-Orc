# CSV Export - Implementation Steps

## Objective

Export coordinates in CSV format for spreadsheet applications.

## Prerequisites

- DocumentState with regions
- JSON export working (for reference)
- File I/O working

## CSV Format

```csv
Region Name,Group,Color,X1 (%),Y1 (%),X2 (%),Y2 (%)
Last Name Cell 1,lastname,blue,0.250000,0.100000,0.350000,0.200000
Last Name Cell 2,lastname,blue,0.350000,0.100000,0.450000,0.200000
```

## Key Points

- Header row with column names
- One row per region
- All coordinates as percentages (0.0-1.0)
- 6 decimal places precision
- Sort alphabetically by region name
- Handle CSV escaping for special characters

## Acceptance Criteria

- [ ] CSV file is created correctly
- [ ] Header row is correct
- [ ] All regions are included
- [ ] Coordinates are percentages (6 decimal places)
- [ ] File opens correctly in spreadsheets

## See Also

- `IMPLEMENTATION_STEPS.md` for detailed step-by-step instructions
