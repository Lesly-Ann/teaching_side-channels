#!/usr/bin/env python3
"""
Script to strip solution blocks from C files.
Removes everything between /* BEGIN_SOLUTION */ and /* END_SOLUTION */ markers.
"""

import os
import re
from pathlib import Path


def strip_solutions(content):
    """
    Remove all content between /* BEGIN_SOLUTION */ and /* END_SOLUTION */ markers.
    
    Args:
        content: String containing the file content
        
    Returns:
        String with solution blocks removed
    """
    # Pattern to match solution blocks including the markers
    pattern = r'/\*\s*BEGIN_SOLUTION\s*\*/.*?/\*\s*END_SOLUTION\s*\*/'
    
    # Remove solution blocks (DOTALL flag allows . to match newlines)
    stripped_content = re.sub(pattern, '// TODO', content, flags=re.DOTALL)
    
    return stripped_content


def process_c_files(source_dir, output_dir):
    """
    Process all C files in the source directory and save stripped versions to output directory.
    
    Args:
        source_dir: Directory containing source C files
        output_dir: Directory where stripped files will be saved
    """
    source_path = Path(source_dir)
    output_path = Path(output_dir)
    
    # Create output directory if it doesn't exist
    output_path.mkdir(exist_ok=True)
    
    # Find all C files (*.c and *.h) in the source directory
    c_files = list(source_path.glob('*.c')) + list(source_path.glob('*.h'))
    
    if not c_files:
        print(f"No C files found in {source_dir}")
        return
    
    print(f"Processing {len(c_files)} file(s)...")
    
    for c_file in c_files:
        # Skip files in subdirectories
        if c_file.parent != source_path:
            continue
            
        print(f"  Processing: {c_file.name}")
        
        # Read the file content
        with open(c_file, 'r', encoding='utf-8') as f:
            content = f.read()
        
        # Strip solution blocks
        stripped_content = strip_solutions(content)
        
        # Write to output directory
        output_file = output_path / c_file.name
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(stripped_content)
        
        print(f"    -> Saved to: {output_file}")
    
    print(f"\nDone! Processed {len(c_files)} file(s).")


if __name__ == "__main__":
    # Get the directory where the script is located
    script_dir = Path(__file__).parent
    
    # Set source and output directories
    source_dir = script_dir
    output_dir = script_dir / "stripped"
    
    print(f"Source directory: {source_dir}")
    print(f"Output directory: {output_dir}")
    print()
    
    process_c_files(source_dir, output_dir)

    # Copy Makefile and layout.ld files to stripped
    print("\nCopying additional files...")
    import shutil

    # Copy Makefile if it exists
    makefile = source_dir / "Makefile"
    if makefile.exists():
        dest_file = output_dir / "Makefile"
        shutil.copy2(makefile, dest_file)
        print(f"  Copied: Makefile")
           
    # Copy layout.ld if it exists
    layout_file = source_dir / "layout.ld"
    if layout_file.exists():
        dest_file = output_dir / "layout.ld"
        shutil.copy2(layout_file, dest_file)
        print(f"  Copied: layout.ld")
    
    # Copy the bonus directory as well
    bonus_dir = source_dir / "bonus"
    if bonus_dir.exists() and bonus_dir.is_dir():
        dest_bonus_dir = output_dir / "bonus"
        shutil.copytree(bonus_dir, dest_bonus_dir, dirs_exist_ok=True)
        print(f"  Copied: bonus directory")

    # Make a zip file with stripped
    zip_filename = script_dir / "code.zip"
    print(f"\nCreating zip file: {zip_filename}")
    shutil.make_archive(base_name=str(zip_filename).replace('.zip', ''), format='zip', root_dir=output_dir)
    print(f"Zip file created: {zip_filename}")

    # Move code.zip to ../website
    website_dir = script_dir.parent / "website"
    if website_dir.exists():
        dest_zip = website_dir / "code.zip"
        shutil.move(str(zip_filename), str(dest_zip))
        print(f"\nMoved code.zip to: {dest_zip}")
    else:
        print(f"\nWarning: Website directory not found: {website_dir}")
