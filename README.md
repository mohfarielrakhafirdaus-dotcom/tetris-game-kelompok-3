# Tetris Console (Windows)

Game Tetris berbasis teks (console) yang ditulis dengan C++. Dilengkapi sistem slot simpanan, leaderboard, pencarian pemain, dan hapus save.

> **Catatan:** Hanya berjalan di **Windows**, karena memakai `conio.h` dan `windows.h`.
# Kelompok 3
- Moh. Arya F5512620078
- Moh. Fariel Rakha Firdaus F5512620059
- Amalia Sam F5512620057
- Thariq ziyad Usamah F5512620085
- Jourinco Rantererung F5512620070
- Andi Arya Dwi Putra F5512620048

## Pembagian Tugas
- Moh Arya bertangung jawab dalam sistem logika game tetris.
- Andi Arya bertanggung jawab dalam sistem slot penyimpanan file fame dan membuat fitur new game.
- Fariel Rakha bertanggung jawab dalam sistem menyortir skor tertinggi di leaderboard dan fitur cari nama.
- Thariq bertanggung jawab dalam pembuatan fitur hapus file data game dan fitur cara bermain.
- Jourinco bertanggung jawab dalam pembuatan sistem manajemen file game dan fitur load game.

## Fitur
- Permainan Tetris di terminal
- Simpan & muat game lewat slot (default 10 slot)
- Leaderboard skor tertinggi
- Cari data pemain berdasarkan nama
- Hapus data simpanan

## Kontrol
| Tombol | Aksi |
|--------|------|
| `A` | Geser kiri |
| `D` | Geser kanan |
| `W` | Putar blok |
| `S` | Turun |
| `Q` | Keluar & simpan |

## Cara Kompilasi
Butuh compiler C++ untuk Windows (misalnya [MinGW-w64](https://www.mingw-w64.org/) atau MSVC).

**Dengan g++:**
```bash
g++ -std=c++11 -O2 tetris.cpp -o tetris.exe
```

**Atau dengan make:**
```bash
make
```

**Dengan MSVC (Developer Command Prompt):**
```bash
cl /EHsc /std:c++14 tetris.cpp
```

Lalu jalankan:
```bash
tetris.exe
```

## Konfigurasi
Jumlah slot simpanan dapat diubah di `tetris.cpp`:
```cpp
const int JUMLAH_SLOT = 10; // ubah ke 100 jika perlu
```

## Lisensi
MIT — lihat file [LICENSE](LICENSE).
