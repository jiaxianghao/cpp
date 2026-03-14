#!/bin/bash
# IDL Syntax Validation Tool
# Validates IDL files before building

set -e

echo "=================================="
echo "FastDDS IDL Validation Tool"
echo "=================================="
echo ""

# Check if fastddsgen is available
if ! command -v fastddsgen &> /dev/null; then
    echo "ERROR: fastddsgen not found"
    echo "Please install FastDDS-Gen from:"
    echo "https://github.com/eProsima/Fast-DDS-Gen"
    exit 1
fi

echo "✓ fastddsgen found"
echo ""

# Find IDL directory
IDL_DIR="${1:-../idl}"

if [ ! -d "$IDL_DIR" ]; then
    echo "ERROR: IDL directory not found: $IDL_DIR"
    exit 1
fi

echo "Scanning IDL directory: $IDL_DIR"
echo ""

# Find all IDL files
IDL_FILES=$(find "$IDL_DIR" -name "*.idl" -type f)

if [ -z "$IDL_FILES" ]; then
    echo "No IDL files found"
    exit 0
fi

# Count files
FILE_COUNT=$(echo "$IDL_FILES" | wc -l)
echo "Found $FILE_COUNT IDL file(s)"
echo ""

# Validate each file
ERRORS=0
SUCCESS=0

for IDL_FILE in $IDL_FILES; do
    echo "Validating: $(basename $IDL_FILE)"
    
    # Try to generate code in a temp directory
    TEMP_DIR=$(mktemp -d)
    
    if fastddsgen -replace -d "$TEMP_DIR" "$IDL_FILE" > /dev/null 2>&1; then
        echo "  ✓ Valid"
        SUCCESS=$((SUCCESS + 1))
    else
        echo "  ✗ Syntax error"
        echo "    Running detailed check:"
        fastddsgen -replace -d "$TEMP_DIR" "$IDL_FILE" 2>&1 | sed 's/^/    /'
        ERRORS=$((ERRORS + 1))
    fi
    
    # Cleanup
    rm -rf "$TEMP_DIR"
    echo ""
done

echo "=================================="
echo "Validation Summary"
echo "=================================="
echo "Total files:  $FILE_COUNT"
echo "Valid:        $SUCCESS"
echo "Errors:       $ERRORS"
echo ""

if [ $ERRORS -eq 0 ]; then
    echo "✓ All IDL files are valid!"
    exit 0
else
    echo "✗ Some IDL files have errors"
    exit 1
fi
