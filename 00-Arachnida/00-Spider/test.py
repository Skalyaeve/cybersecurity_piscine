from PIL import Image
import py3exiv2
import os

path = "data/..."

print(f"Filename: {os.path.basename(path)}")
print(f"Size: {os.path.getsize(path)} bytes")

img = Image.open(path)
exif_data = img._getexif()
if exif_data is not None:
    for tag_id, value in exif_data.items():
        tag_name = Image.TAGS.get(tag_id, tag_id)
        print(f"EXIF {tag_name} ({tag_id}): {value}")
else:
    print("No EXIF data.")

metadata = py3exiv2.ImageMetadata(path)
metadata.read()

print("\nIPTC data:")
for key in metadata.iptc_keys:
    print(f"IPTC {key}: {metadata[key].raw_value}")

print("\XMP data:")
for key in metadata.xmp_keys:
    print(f"XMP {key}: {metadata[key].raw_value}")
