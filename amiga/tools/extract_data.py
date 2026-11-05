#!/usr/bin/env python3
"""Write Mr.Boom's sound effects and music out of upstream's sdl2/sdl2_data.h.

The SDL 2 build embeds a zip (dataRom, a C string of \\xNN escapes) and
unpacks it to $HOME at start-up; the Amiga build reads the files from
PROGDIR:data/ instead, so they are unpacked once, here, at build time.
mrboom.dat is left out: the Amiga build uses the heap already in
mrboom_data.c, as the libretro core does.

    python3 amiga/tools/extract_data.py sdl2/sdl2_data.h OUTDIR
"""
import io
import os
import re
import sys
import zipfile


def main():
    src, out = sys.argv[1], sys.argv[2]
    text = open(src, encoding="ascii").read()
    body = text[text.index("=") + 1:text.rindex(";")]
    literal = "".join(re.findall(r'"((?:[^"\\]|\\.)*)"', body))
    if re.sub(r"\\x[0-9A-Fa-f]{2}", "", literal):
        sys.exit("extract_data: unexpected characters in dataRom")
    blob = bytes(int(h, 16) for h in re.findall(r"\\x([0-9A-Fa-f]{2})", literal))
    os.makedirs(out, exist_ok=True)
    count = 0
    with zipfile.ZipFile(io.BytesIO(blob)) as z:
        for name in z.namelist():
            if os.path.splitext(name)[1].upper() not in (".WAV", ".MOD", ".XM"):
                continue
            with open(os.path.join(out, os.path.basename(name)), "wb") as f:
                f.write(z.read(name))
            count += 1
    print("extract_data: %d files in %s" % (count, out))


if __name__ == "__main__":
    main()
