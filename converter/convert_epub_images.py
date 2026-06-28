"""

Guy Turcotte
2026-05-31

This script processes an EPUB file, converting all embedded images to 4-bit 
grayscale BMP format and resizing them to fit within specified maximum dimensions. 
It also updates the EPUB's XHTML/HTML files to reference the new BMP images. 
The modified EPUB is then saved to a new file.

The following libraries are required:

- ebooklib: For reading and writing EPUB files.
- BeautifulSoup: For parsing and modifying XHTML/HTML content within the EPUB.
- Pillow (PIL): For image processing tasks such as resizing and format conversion.

They can be installed using pip:
    pip install ebooklib beautifulsoup4 Pillow

Usage:
    python convert_epub_images.py <input.epub> <max_width> <max_height> <output.epub>

Example:
    python convert_epub_images.py my_book.epub 600 800 my_book_converted.epub

"""

import os
import sys
import io
from ebooklib import epub
from bs4 import BeautifulSoup
from PIL import Image

def convert_and_resize(img_bytes, max_width, max_height):
    """
    Resizes the image and converts it to a 4-bit (16 levels) grayscale BMP.
    """
    try:
        orig_img = Image.open(io.BytesIO(img_bytes))
    except Exception:
        return None

    # Resize (maintaining aspect ratio using thumbnail)
    orig_img.thumbnail((max_width, max_height), Image.Resampling.LANCZOS)
    
    # Convert to 8-bit Grayscale ('L')
    gray_img = orig_img.convert('L')
    
    # Quantize to 4-bit (16 levels of gray)
    quantized_img = gray_img.quantize(colors=16, method=Image.Quantization.MEDIANCUT)
    
    # Save as BMP to memory
    bmp_io = io.BytesIO()
    quantized_img.save(bmp_io, format='BMP')
    return bmp_io.getvalue()

def process_epub(epub_path, max_width, max_height, output_path):
    # Load EPUB
    try:
        book = epub.read_epub(epub_path)
    except Exception as e:
        print(f"Error reading EPUB: {e}")
        return

    # Track modified items to prevent duplicate processing
    processed_items = {}

    # Process all items in the book
    for item in book.get_items():
        if item.get_type() == ebooklib.ITEM_IMAGE:
            old_content = item.get_content()
            new_content = convert_and_resize(old_content, max_width, max_height)
            
            if new_content:
                old_ext = os.path.splitext(item.get_name())[1].lower()
                new_name = item.get_name().replace(old_ext, '.bmp')
                
                # Update item properties
                processed_items[item.get_name()] = new_name
                item.set_content(new_content)
                item.set_file_name(new_name)
                item.media_type = 'image/bmp'

    # Update XHTML/HTML files in the EPUB to point to the renamed BMP files
    for item in book.get_items_of_type(ebooklib.ITEM_DOCUMENT):
        content = item.get_content()
        soup = BeautifulSoup(content, 'html.parser')
        
        # Update <img> tags
        for img in soup.find_all('img'):
            src = img.get('src')
            if src and src in processed_items:
                img['src'] = processed_items[src]
                
        # Update <image> tags (common in EPUB SVGs)
        for svg_img in soup.find_all('image'):
            xlink_href = svg_img.get('xlink:href') or svg_img.get('href')
            if xlink_href and xlink_href in processed_items:
                if svg_img.get('xlink:href'):
                    svg_img['xlink:href'] = processed_items[xlink_href]
                else:
                    svg_img['href'] = processed_items[xlink_href]
                    
        item.set_content(str(soup).encode('utf-8'))

    # Save the modified EPUB
    epub.write_epub(book, output_path)
    print(f"Successfully processed EPUB. Output saved to: {output_path}")

if __name__ == '__main__':
    if len(sys.argv) < 5:
        print("Usage: python convert_epub_images.py <input.epub> <max_width> <max_height> <output.epub>")
        sys.exit(1)

    input_epub = sys.argv[1]
    width = int(sys.argv[2])
    height = int(sys.argv[3])
    output_epub = sys.argv[4]

    process_epub(input_epub, width, height, output_epub)
