#!/usr/bin/env python3
"""
Create macOS app icon from OCR-Orc logo
Generates an .icns file with light grey-white diagonal gradient background,
rounded corners, and slightly flared planes
"""

import os
import sys
from PIL import Image, ImageDraw, ImageFilter
import subprocess

def create_icon_with_background(logo_path, output_path, size=1024):
    """Create app icon with styled background"""
    
    # Load the logo
    logo = Image.open(logo_path).convert("RGBA")
    
    # Resize logo to fit within the icon (with padding)
    logo_size = int(size * 0.75)  # Logo takes 75% of icon size
    logo = logo.resize((logo_size, logo_size), Image.Resampling.LANCZOS)
    
    # Create base image with gradient background (FAST VERSION - no numpy needed)
    corner_radius = int(size * 0.15)  # 15% corner radius for rounded corners
    
    # Create gradient background efficiently using ImageDraw
    rounded_icon = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(rounded_icon)
    
    # Draw diagonal gradient using lines (much faster than pixel-by-pixel)
    # Draw diagonal stripes for gradient effect
    num_stripes = 200  # More stripes = smoother gradient
    for i in range(num_stripes):
        # Calculate position along diagonal
        t = i / num_stripes
        # Interpolate color from grey (224) to white (255)
        grey_val = int(224 + (255 - 224) * t)
        color = (grey_val, grey_val, grey_val, 255)
        
        # Draw diagonal line
        x1 = int(size * t * 0.7)
        y1 = int(size * t * 0.7)
        x2 = int(size * (1 - (1-t) * 0.7))
        y2 = int(size * (1 - (1-t) * 0.7))
        
        # Draw thick line for gradient effect
        line_width = max(1, size // num_stripes)
        for offset in range(-line_width//2, line_width//2 + 1):
            draw.line([(x1 + offset, y1 + offset), (x2 + offset, y2 + offset)], fill=color, width=line_width)
    
    # Apply rounded corners using mask
    mask = Image.new("L", (size, size), 0)
    mask_draw = ImageDraw.Draw(mask)
    mask_draw.rounded_rectangle(
        [(0, 0), (size, size)],
        corner_radius,
        fill=255
    )
    
    # Apply mask to create rounded corners
    rounded_icon.putalpha(mask)
    
    # Add slight bevel/flare effect (subtle shadow and highlight)
    # Top-left highlight
    highlight = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    highlight_draw = ImageDraw.Draw(highlight)
    highlight_draw.rounded_rectangle(
        [(0, 0), (size, size)],
        corner_radius,
        fill=(255, 255, 255, 30)  # Subtle white overlay
    )
    
    # Bottom-right shadow
    shadow = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    shadow_draw = ImageDraw.Draw(shadow)
    shadow_draw.rounded_rectangle(
        [(2, 2), (size, size)],  # Slight offset
        corner_radius,
        fill=(0, 0, 0, 20)  # Subtle black shadow
    )
    
    # Composite: shadow, base, highlight
    final_icon = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    final_icon = Image.alpha_composite(final_icon, shadow)
    final_icon = Image.alpha_composite(final_icon, rounded_icon)
    final_icon = Image.alpha_composite(final_icon, highlight)
    
    # Center the logo on the background
    logo_x = (size - logo_size) // 2
    logo_y = (size - logo_size) // 2
    
    # Paste logo onto background
    final_icon.paste(logo, (logo_x, logo_y), logo)
    
    # Save as PNG
    final_icon.save(output_path, "PNG")
    print(f"✓ Created icon: {output_path}")
    
    return final_icon

def create_iconset(logo_path, iconset_dir):
    """Create .iconset directory with all required sizes"""
    
    sizes = [
        (16, "icon_16x16.png"),
        (32, "icon_16x16@2x.png"),
        (32, "icon_32x32.png"),
        (64, "icon_32x32@2x.png"),
        (128, "icon_128x128.png"),
        (256, "icon_128x128@2x.png"),
        (256, "icon_256x256.png"),
        (512, "icon_256x256@2x.png"),
        (512, "icon_512x512.png"),
        (1024, "icon_512x512@2x.png"),
    ]
    
    os.makedirs(iconset_dir, exist_ok=True)
    
    for size, filename in sizes:
        output_path = os.path.join(iconset_dir, filename)
        create_icon_with_background(logo_path, output_path, size)
    
    print(f"✓ Created iconset: {iconset_dir}")

def create_icns(iconset_dir, icns_path):
    """Convert .iconset to .icns using iconutil"""
    
    if not os.path.exists("/usr/bin/iconutil"):
        print("ERROR: iconutil not found. Cannot create .icns file.")
        print("Please run on macOS or use an alternative method.")
        return False
    
    try:
        subprocess.run(
            ["/usr/bin/iconutil", "-c", "icns", iconset_dir, "-o", icns_path],
            check=True,
            capture_output=True
        )
        print(f"✓ Created .icns file: {icns_path}")
        return True
    except subprocess.CalledProcessError as e:
        print(f"ERROR: Failed to create .icns: {e}")
        return False

def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_root = os.path.dirname(script_dir)
    
    logo_path = os.path.join(project_root, "OCR-Orc-LOGO.png")
    if not os.path.exists(logo_path):
        # Try parent directory
        logo_path = os.path.join(os.path.dirname(project_root), "OCR-Orc-LOGO.png")
    if not os.path.exists(logo_path):
        print(f"ERROR: Logo not found at {logo_path}")
        sys.exit(1)
    
    # Create iconset directory
    iconset_dir = os.path.join(script_dir, "AppIcon.iconset")
    icns_path = os.path.join(script_dir, "AppIcon.icns")
    
    print("Creating app icon...")
    print(f"Logo: {logo_path}")
    print(f"Iconset: {iconset_dir}")
    print(f"ICNS: {icns_path}")
    print()
    
    # Create iconset
    create_iconset(logo_path, iconset_dir)
    print()
    
    # Create .icns file
    if create_icns(iconset_dir, icns_path):
        print()
        print("✓ App icon created successfully!")
        print(f"  Location: {icns_path}")
    else:
        print()
        print("⚠ .icns creation failed, but iconset is available")
        print(f"  You can manually create .icns using:")
        print(f"  iconutil -c icns {iconset_dir} -o {icns_path}")

if __name__ == "__main__":
    main()

