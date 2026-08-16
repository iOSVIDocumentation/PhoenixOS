#!/usr/bin/env python3
import struct
import os

# Палитры для каждой темы (из theme.c)
palettes = {
    'phoenix': {
        'desktop': 0x0010,
        'K': 0x0000, 'W': 0xFFFF, 'G': 0xF7DE, 'D': 0x8410,
        'B': 0x3A9F, 'N': 0x0013, 'Y': 0xFFE0, 'y': 0xCD00,
        'P': 0x780F, 'R': 0xF800, 'E': 0x07E0
    },
    'xp': {
        'desktop': 0x3B74,
        'K': 0x0000, 'W': 0xFFFF, 'G': 0xEF5B, 'D': 0xAD53,
        'B': 0x02BC, 'N': 0x0012, 'Y': 0xFFE0, 'y': 0xCD00,
        'P': 0x780F, 'R': 0xF800, 'E': 0x2C45
    },
    'macos': {
        'desktop': 0x8410,
        'K': 0x0000, 'W': 0xFFFF, 'G': 0xC618, 'D': 0x8410,
        'B': 0x0010, 'N': 0x0010, 'Y': 0xFFE0, 'y': 0xCD00,
        'P': 0x780F, 'R': 0xF800, 'E': 0x07E0
    }
}

# Пиксель-арт из ui.c (16x16 символов)
art_my_pc = [
    "................",
    ".KKKKKKKKKKKKKK.",
    ".KGGGGGGGGGGGGK.",
    ".KGBBBBBBBBBBGK.",
    ".KGBWWBBBBBBBGK.",
    ".KGBWWBBBBBBBGK.",
    ".KGBBBBBBBBBBGK.",
    ".KGBBWWWWWWBBGK.",
    ".KGBBBBBBBBBBGK.",
    ".KGGGGGGGGGGGGK.",
    ".KKKKKKKKKKKKKK.",
    "......KDDK......",
    "......KDDK......",
    "....KDDDDDDK....",
    "...KDDDDDDDDK...",
    "................"
]

art_files = [
    "................",
    ".KKKK...........",
    ".KyyKKKKKKKKKKK.",
    ".KYYYYYYYYYYYYK.",
    ".KYWWWWWWWWWWYK.",
    ".KYWWWWWWWWWWYK.",
    ".KYYYYYYYYYYYYK.",
    ".KYYYYYYYYYYYYK.",
    ".KYYYYYYYYYYYYK.",
    ".KYYYYYYYYYYYYK.",
    ".KyyyyyyyyyyyyK.",
    ".KKKKKKKKKKKKKK.",
    "................",
    "................",
    "................",
    "................"
]

art_settings = [
    "................",
    ".....KGGGGK.....",
    ".....KGGGGK.....",
    "..KKKGGGGGGKKK..",
    "..KGGGGGGGGGGK..",
    ".KGGGGKKKKGGGGK.",
    ".KGGGK....KGGGK.",
    ".KGGGK....KGGGK.",
    ".KGGGK....KGGGK.",
    ".KGGGK....KGGGK.",
    ".KGGGGKKKKGGGGK.",
    "..KGGGGGGGGGGK..",
    "..KKKGGGGGGKKK..",
    ".....KGGGGK.....",
    ".....KGGGGK.....",
    "................"
]

art_games = [
    "................",
    "..KKKKKKKKKKKK..",
    ".KDDDDDDDDDDDDK.",
    ".KDDDWDDDDDRDDK.",
    ".KDWWWDDDDRRDDK.",
    ".KDDDWDDDDERDDK.",
    ".KDDDDDDDDDDDDK.",
    ".KDDDDDDDDDDDDK.",
    ".KDDKDDDDDDKDDK.",
    ".KKKDDDDDDDDKKK.",
    "..KKKKKKKKKKKK..",
    "................",
    "................",
    "................",
    "................",
    "................"
]

art_media = [
    "................",
    ".KKKKKKKKKKKKKK.",
    ".KWKPKWKPKWKPWK.",
    ".KKKKKKKKKKKKKK.",
    ".KPPPPPPPPPPPPK.",
    ".KPPWPPPPPPPPPK.",
    ".KPPWWPPPPPPPPK.",
    ".KPPWWWPPPPPPPK.",
    ".KPPWWPPPPPPPPK.",
    ".KPPWPPPPPPPPPK.",
    ".KPPPPPPPPPPPPK.",
    ".KKKKKKKKKKKKKK.",
    "................",
    "................",
    "................",
    "................"
]

all_arts = [art_my_pc, art_files, art_settings, art_games, art_media]

def render_icon(art, palette):
    """Рендерит иконку 16x16 в 32x32 пикселя RGB565 big-endian"""
    pixels = []
    for y in range(16):
        for py in range(2):  # каждый "пиксель" арта -> 2 пикселя по Y
            for x in range(16):
                ch = art[y][x]
                if ch == '.':
                    col = palette['desktop']
                else:
                    col = palette.get(ch, palette['desktop'])
                for px in range(2):  # каждый "пиксель" арта -> 2 пикселя по X
                    pixels.append(struct.pack('>H', col))
    return b''.join(pixels)

def generate_theme_icons(theme_name, palette):
    """Генерирует icons.rgb для одной темы"""
    icon_data = []
    for art in all_arts:
        icon_data.append(render_icon(art, palette))
    return b''.join(icon_data)

# Генерация для всех тем
for theme_name, palette in palettes.items():
    out_dir = f'themes/{theme_name}'
    os.makedirs(out_dir, exist_ok=True)
    out_path = f'{out_dir}/icons.rgb'
    
    data = generate_theme_icons(theme_name, palette)
    with open(out_path, 'wb') as f:
        f.write(data)
    
    print(f"Создан: {out_path} ({len(data)} байт)")

print("\nГотово! Теперь скопируй файлы на карту памяти:")
print("  cp -r themes/* /run/media/$USER/<LABEL_SD>/")
print("Или перетащи папки phoenix/, xp/, macos/ на SD карту через файловый менеджер.")
