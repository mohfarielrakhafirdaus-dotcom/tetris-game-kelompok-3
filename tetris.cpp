#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cstdlib>
#include <conio.h>   // Khusus Windows untuk _kbhit() dan _getch()
#include <windows.h> // Khusus Windows untuk Sleep() dan penanganan kursor
#include <cctype>
#include <chrono>
#include <algorithm>

using namespace std;

// ======================= KONSTANTA & STRUKTUR =======================
const int TINGGI_PAPAN = 20;
const int LEBAR_PAPAN = 10;
const int JUMLAH_SLOT = 10; // Ubah ke 100 jika ingin kapasitas 100 slot

struct StatusGameTetris {
    string namaPemain;
    int skor;
    int level;
    int totalBarisHancur;
    int waktuBermainDetik;
};

// ======================= VARIABEL GLOBAL =======================
StatusGameTetris statusAktif;
int slotAktifDigunakan = -1;

// ======================= FUNGSI SISTEM & FILE =======================
void aturUlangKursor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD koordinatPos = {0, 0};
    SetConsoleCursorPosition(hConsole, koordinatPos);
}

void sembunyikanKursor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO infoKursor;
    GetConsoleCursorInfo(hConsole, &infoKursor);
    infoKursor.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &infoKursor);
}

void bersihkanLayar() {
    system("cls");
}

string ambilNamaFileSlot(int nomorSlot) {
    return "tetris_slot" + to_string(nomorSlot) + ".dat";
}

bool simpanDataGame(int nomorSlot, const StatusGameTetris& status) {
    string namaFile = ambilNamaFileSlot(nomorSlot);
    ofstream fileKeluar(namaFile, ios::out | ios::trunc);
    if (!fileKeluar.is_open()) return false;

    fileKeluar << status.namaPemain << "\n";
    fileKeluar << status.skor << "\n";
    fileKeluar << status.level << "\n";
    fileKeluar << status.totalBarisHancur << "\n";
    fileKeluar << status.waktuBermainDetik << "\n";
    fileKeluar.close();
    return true;
}

bool muatDataGame(int nomorSlot, StatusGameTetris& status) {
    string namaFile = ambilNamaFileSlot(nomorSlot);
    ifstream fileMasuk(namaFile, ios::in);
    if (!fileMasuk.is_open()) return false; 

    getline(fileMasuk, status.namaPemain);
    fileMasuk >> status.skor;
    fileMasuk >> status.level;
    fileMasuk >> status.totalBarisHancur;
    fileMasuk >> status.waktuBermainDetik;
    fileMasuk.close();
    return true;
}

void tampilkanDaftarSlot() {
    cout << "\n=== DAFTAR SLOT TETRIS (1 - " << JUMLAH_SLOT << ") ===\n";
    for (int i = 1; i <= JUMLAH_SLOT; i++) {
        StatusGameTetris statusTemp;
        cout << "Slot [" << i << "] : ";
        if (muatDataGame(i, statusTemp)) {
            cout << "[TERISI] Pemain: " << statusTemp.namaPemain 
                 << " | Lvl: " << statusTemp.level 
                 << " | Skor: " << statusTemp.skor << endl;
        } else {
            cout << "[KOSONG]" << endl;
        }
    }
    cout << "-------------------------------------------\n";
}

// ======================= LOGIKA MATRIKS TETRIS =======================
vector<vector<int>> ambilTetriminoAcak(char& jenisKeluar) {
    int indeksAcak = rand() % 7; 
    
    switch (indeksAcak) {
        case 0:
            jenisKeluar = 'I';
            return {{0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}};
        case 1:
            jenisKeluar = 'J';
            return {{1, 0, 0}, {1, 1, 1}, {0, 0, 0}};
        case 2:
            jenisKeluar = 'L';
            return {{0, 0, 1}, {1, 1, 1}, {0, 0, 0}};
        case 3:
            jenisKeluar = 'O';
            return {{1, 1}, {1, 1}};
        case 4:
            jenisKeluar = 'S';
            return {{0, 1, 1}, {1, 1, 0}, {0, 0, 0}};
        case 5:
            jenisKeluar = 'T';
            return {{0, 1, 0}, {1, 1, 1}, {0, 0, 0}};
        case 6:
            jenisKeluar = 'Z';
            return {{1, 1, 0}, {0, 1, 1}, {0, 0, 0}};
        default:
            jenisKeluar = 'I';
            return {{0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}};
    }
}

// Fungsi Rotasi Matriks Baru (Transpose)
vector<vector<int>> putarMatriks(const vector<vector<int>>& matriks) {
    int n = matriks.size();
    vector<vector<int>> diputar(n, vector<int>(n, 0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            diputar[j][n - 1 - i] = matriks[i][j];
        }
    }
    return diputar;
}

bool cekTabrakan(const vector<vector<int>>& papan, const vector<vector<int>>& bentuk, int posX, int posY) {
    int ukuran = bentuk.size();
    for (int i = 0; i < ukuran; ++i) {
        for (int j = 0; j < ukuran; ++j) {
            if (bentuk[i][j] != 0) {
                int papanX = posX + j;
                int papanY = posY + i;
                if (papanX < 0 || papanX >= LEBAR_PAPAN || papanY >= TINGGI_PAPAN) return true;
                if (papanY >= 0 && papan[papanY][papanX] != 0) return true;
            }
        }
    }
    return false;
}

int hapusBaris(vector<vector<int>>& papan, int& skor, int& level, int& totalBarisHancur) {
    int barisHancurGiliranIni = 0;
    for (int i = TINGGI_PAPAN - 1; i >= 0; --i) {
        bool apakahBarisPenuh = true;
        for (int j = 0; j < LEBAR_PAPAN; ++j) {
            if (papan[i][j] == 0) { apakahBarisPenuh = false; break; }
        }
        if (apakahBarisPenuh) {
            barisHancurGiliranIni++;
            for (int k = i; k > 0; --k) papan[k] = papan[k - 1];
            papan[0] = vector<int>(LEBAR_PAPAN, 0);
            i++; 
        }
    }
    if (barisHancurGiliranIni > 0) {
        totalBarisHancur += barisHancurGiliranIni;
        int poinDasar = 0;
        switch (barisHancurGiliranIni) {
            case 1: poinDasar = 100; break;     
            case 2: poinDasar = 300; break;    
            case 3: poinDasar = 500; break;    
            case 4: poinDasar = 800; break;   
        }
        skor += poinDasar * level;
        level = 1 + (totalBarisHancur / 5);
    }
    return barisHancurGiliranIni;
}

void renderPapan(const vector<vector<int>>& papan, const vector<vector<int>>& bentukAktif, int posX, int posY, int skor, int level, char jenisAktif, const string& namaPemain) {
    aturUlangKursor();
    const string garis(2 * LEBAR_PAPAN + 2, '=');
    cout << garis << "\n";
    cout << " Pemain : " << namaPemain << "\n";
    cout << " Skor   : " << skor << " | Level: " << level << "\n";
    cout << " Tipe   : " << jenisAktif << " | Lines: " << statusAktif.totalBarisHancur << "\n";
    cout << garis << "\n";

    vector<vector<int>> papanSementara = papan;
    int ukuran = bentukAktif.size();
    for (int i = 0; i < ukuran; ++i) {
        for (int j = 0; j < ukuran; ++j) {
            if (bentukAktif[i][j] != 0) {
                int targetY = posY + i;
                int targetX = posX + j;
                if (targetY >= 0 && targetY < TINGGI_PAPAN && targetX >= 0 && targetX < LEBAR_PAPAN) {
                    papanSementara[targetY][targetX] = 9;
                }
            }
        }
    }

    for (int i = 0; i < TINGGI_PAPAN; ++i) {
        cout << '|';
        for (int j = 0; j < LEBAR_PAPAN; ++j) {
            if (papanSementara[i][j] == 0) cout << ". ";
            else if (papanSementara[i][j] == 9) cout << "O ";
            else cout << "# ";
        }
        cout << "|\n";
    }
    cout << garis << "\n";
    cout << " [a] Kiri | [d] Kanan | [w] Putar\n";
    cout << " [s] Turun | [q] Keluar & Simpan\n";
}

// ======================= GAMEPLAY UTAMA =======================
void jalankanGameTetris(int nomorSlot, StatusGameTetris& status) {
    system("cls");
    sembunyikanKursor();

    vector<vector<int>> papan(TINGGI_PAPAN, vector<int>(LEBAR_PAPAN, 0));
    
    char jenisAktif;
    vector<vector<int>> bentukAktif = ambilTetriminoAcak(jenisAktif);
    int posX = 3, posY = 0;
    bool permainanSelesai = false;

    auto waktuMulai = chrono::steady_clock::now();

    while (!permainanSelesai) {
        renderPapan(papan, bentukAktif, posX, posY, status.skor, status.level, jenisAktif, status.namaPemain);
        
        if (_kbhit()) {
            char tombol = _getch();
            int posBaruX = posX, posBaruY = posY;
            vector<vector<int>> bentukBaru = bentukAktif;

            if (tombol == 'a' || tombol == 'A') posBaruX--;
            else if (tombol == 'd' || tombol == 'D') posBaruX++;
            else if (tombol == 'w' || tombol == 'W') bentukBaru = putarMatriks(bentukAktif);
            else if (tombol == 's' || tombol == 'S') posBaruY++;
            else if (tombol == 'q' || tombol == 'Q') {
                // Hitung waktu bermain dan simpan otomatis, lalu picu layar Game Over
                auto waktuSekarang = chrono::steady_clock::now();
                auto detikBerlalu = chrono::duration_cast<chrono::seconds>(waktuSekarang - waktuMulai).count();
                status.waktuBermainDetik += static_cast<int>(detikBerlalu);
                simpanDataGame(nomorSlot, status);
                
                permainanSelesai = true; 
                break; 
            }

            if (!cekTabrakan(papan, bentukBaru, posBaruX, posBaruY)) {
                posX = posBaruX;
                posY = posBaruY;
                bentukAktif = bentukBaru;
            }
        }

        Sleep(250); // Kecepatan jatuh dasar

        if (!cekTabrakan(papan, bentukAktif, posX, posY + 1)) {
            posY++;
        } else {
            int ukuran = bentukAktif.size();
            for (int i = 0; i < ukuran; ++i) {
                for (int j = 0; j < ukuran; ++j) {
                    if (bentukAktif[i][j] != 0) {
                        int bY = posY + i, bX = posX + j;
                        if (bY >= 0 && bY < TINGGI_PAPAN && bX >= 0 && bX < LEBAR_PAPAN) {
                            papan[bY][bX] = 1;
                        }
                    }
                }
            }

            hapusBaris(papan, status.skor, status.level, status.totalBarisHancur);
            
            bentukAktif = ambilTetriminoAcak(jenisAktif); 
            posX = 3; posY = 0;

            if (cekTabrakan(papan, bentukAktif, posX, posY)) {
                permainanSelesai = true;
            }
        }
    }

    // Tampilan Layar Game Over / Keluar
    simpanDataGame(nomorSlot, status);
    system("cls");
    cout << "========================================\n";
    cout << "           PERMAINAN SELESAI            \n";
    cout << "========================================\n";
    cout << " Pemain             : " << status.namaPemain << endl;
    cout << " Skor Akhir         : " << status.skor << endl;
    cout << " Total Baris Hancur : " << status.totalBarisHancur << endl;
    cout << "========================================\n";
    cout << "Tekan Enter untuk kembali ke menu...";
    cin.ignore(10000, '\n');
    cin.get();
}

// ======================= MENU & FITUR CRUD =======================
void handleGameBaru() {
    bersihkanLayar();
    cout << "========================================\n";
    cout << "            FITUR GAME BARU             \n";
    cout << "========================================\n";
    tampilkanDaftarSlot();
    
    int slotPilihan;
    cout << "\nPilih nomor slot kosong (1-" << JUMLAH_SLOT << "): ";
    if (!(cin >> slotPilihan)) {
        cin.clear();
        cin.ignore(10000, '\n');
        return;
    }

    if (slotPilihan < 1 || slotPilihan > JUMLAH_SLOT) {
        cout << "Nomor slot tidak valid! Tekan Enter...";
        cin.ignore(10000, '\n');
        cin.get();
        return;
    }

    StatusGameTetris statusTemp;
    if (muatDataGame(slotPilihan, statusTemp)) {
        cout << "\n[PERINGATAN] Slot " << slotPilihan << " sudah terisi oleh '" << statusTemp.namaPemain << "'!\n";
        cout << "Tekan Enter untuk kembali...";
        cin.ignore(10000, '\n');
        cin.get();
        return;
    }

    slotAktifDigunakan = slotPilihan;
    cout << "\nMasukkan Nama Pemain: ";
    cin >> statusAktif.namaPemain;

    statusAktif.skor = 0;
    statusAktif.level = 1;
    statusAktif.totalBarisHancur = 0;
    statusAktif.waktuBermainDetik = 0;

    simpanDataGame(slotAktifDigunakan, statusAktif);
    jalankanGameTetris(slotAktifDigunakan, statusAktif);
}

void handleMuatGame() {
    bersihkanLayar();
    cout << "========================================\n";
    cout << "            FITUR MUAT GAME             \n";
    cout << "========================================\n";
    tampilkanDaftarSlot();
    
    int slotPilihan;
    cout << "\nPilih nomor slot yang ingin dimuat (1-" << JUMLAH_SLOT << "): ";
    if (!(cin >> slotPilihan)) {
        cin.clear();
        cin.ignore(10000, '\n');
        return;
    }

    if (slotPilihan < 1 || slotPilihan > JUMLAH_SLOT) {
        cout << "Nomor slot tidak valid! Tekan Enter...";
        cin.ignore(10000, '\n');
        cin.get();
        return;
    }

    if (muatDataGame(slotPilihan, statusAktif)) {
        slotAktifDigunakan = slotPilihan;

        cout << "\n[Sukses] Memuat game atas nama: " << statusAktif.namaPemain 
             << " (Skor Terakhir: " << statusAktif.skor << ")\n";
        cout << "Tekan Enter untuk mulai bermain ronde baru...";
        cin.ignore(10000, '\n');
        cin.get();

        statusAktif.skor = 0;
        statusAktif.level = 1;
        statusAktif.totalBarisHancur = 0;
        
        jalankanGameTetris(slotAktifDigunakan, statusAktif);
    } else {
        cout << "\n[Gagal] Slot " << slotPilihan << " kosong atau tidak ditemukan!\n";
        cout << "Tekan Enter untuk kembali ke menu...";
        cin.ignore(10000, '\n');
        cin.get();
    }
}

void handleLeaderboard() {
    bersihkanLayar();
    cout << "========================================\n";
    cout << "          LEADERBOARD SKOR TERTINGGI     \n";
    cout << "========================================\n";

    vector<StatusGameTetris> daftarPemain;

    for (int i = 1; i <= JUMLAH_SLOT; i++) {
        StatusGameTetris statusTemp;
        if (muatDataGame(i, statusTemp)) {
            daftarPemain.push_back(statusTemp);
        }
    }

    if (daftarPemain.empty()) {
        cout << "\nBelum ada data pemain yang tersimpan.\n";
    } else {
        sort(daftarPemain.begin(), daftarPemain.end(), [](const StatusGameTetris& a, const StatusGameTetris& b) {
            return a.skor > b.skor;
        });

        cout << "No.  Pemain            Level   Skor Terakhir\n";
        cout << "----------------------------------------\n";
        for (size_t i = 0; i < daftarPemain.size(); i++) {
            cout << " [" << (i + 1) << "]  " 
                 << daftarPemain[i].namaPemain;
            
            int panjangNama = daftarPemain[i].namaPemain.length();
            for (int s = panjangNama; s < 16; s++) cout << " ";

            cout << " Lvl " << daftarPemain[i].level 
                 << "   " << daftarPemain[i].skor << endl;
        }
    }

    cout << "----------------------------------------\n";
    cout << "Tekan Enter untuk kembali ke menu...";
    cin.ignore();
    cin.get();
}

void handleCariPemain() {
    bersihkanLayar();
    cout << "========================================\n";
    cout << "           CARI DATA PEMAIN             \n";
    cout << "========================================\n";
    
    string kataKunci;
    cout << "Masukkan nama pemain yang dicari: ";
    cin >> kataKunci;

    string kataKunciKecil = kataKunci;
    transform(kataKunciKecil.begin(), kataKunciKecil.end(), kataKunciKecil.begin(), ::tolower);

    bool ditemukan = false;
    cout << "\nHasil Pencarian:\n";
    cout << "---------------------------------------------------------\n";

    for (int i = 1; i <= JUMLAH_SLOT; i++) {
        StatusGameTetris statusTemp;
        if (muatDataGame(i, statusTemp)) {
            string namaPemainKecil = statusTemp.namaPemain;
            transform(namaPemainKecil.begin(), namaPemainKecil.end(), namaPemainKecil.begin(), ::tolower);

            if (namaPemainKecil.find(kataKunciKecil) != string::npos) {
                cout << "-> Slot [" << i << "] | Nama: " << statusTemp.namaPemain 
                     << " | Skor: " << statusTemp.skor 
                     << " | Level: " << statusTemp.level 
                     << " | Lines: " << statusTemp.totalBarisHancur << endl;
                ditemukan = true;
            }
        }
    }

    if (!ditemukan) {
        cout << "[Info] Tidak ditemukan data pemain dengan nama '" << kataKunci << "'.\n";
    }

    cout << "---------------------------------------------------------\n";
    cout << "Tekan Enter untuk kembali ke menu...";
    cin.ignore(10000, '\n');
    cin.get();
}

void handleHapusSimpanan() {
    bersihkanLayar();
    cout << "========================================\n";
    cout << "            MENU: HAPUS SAVE            \n";
    cout << "========================================\n";
    tampilkanDaftarSlot();
    
    int slotPilihan;
    cout << "\nPilih nomor slot yang ingin dihapus (1-" << JUMLAH_SLOT << "): ";
    if (!(cin >> slotPilihan)) {
        cin.clear();
        cin.ignore(10000, '\n');
        return;
    }

    if (slotPilihan >= 1 && slotPilihan <= JUMLAH_SLOT) {
        string namaFile = ambilNamaFileSlot(slotPilihan);
        if (remove(namaFile.c_str()) == 0) {
            cout << "\n[Sukses] Slot " << slotPilihan << " berhasil dihapus!\n";
        } else {
            cout << "\n[Info] Slot " << slotPilihan << " sudah kosong.\n";
        }
    }
    cout << "Tekan Enter untuk kembali...";
    cin.ignore(10000, '\n');
    cin.get();
}

void handleCaraBermain() {
    bersihkanLayar();
    cout << "========================================\n";
    cout << "       CARA BERMAIN (HOW TO PLAY)       \n";
    cout << "========================================\n";
    cout << " - A : Geser Kiri\n";
    cout << " - D : Geser Kanan\n";
    cout << " - S : Soft Drop (Turun Cepat)\n";
    cout << " - W : Rotasi Balok (Matriks Transpose)\n";
    cout << " - Q : Keluar & Simpan ke Slot\n\n";
    cout << "Tekan Enter untuk kembali ke menu...";
    cin.ignore();
    cin.get();
}

int main() {
    int pilihanMenu = 0;

    do {
        bersihkanLayar();
        cout << "========================================\n";
        cout << "         C++ TETRIS GAME (WINDOWS)      \n";
        cout << "========================================\n";
        cout << "  [1] Game Baru (New Game)\n";
        cout << "  [2] Muat Game (Load Game)\n";
        cout << "  [3] Leaderboard (Skor Tertinggi)\n";
        cout << "  [4] Cari Data Pemain (Search)\n";
        cout << "  [5] Hapus Simpanan (Delete Save)\n";
        cout << "  [6] Cara Bermain (How to Play)\n";
        cout << "  [7] Keluar (Exit)\n";
        cout << "========================================\n";
        cout << "Pilih menu (1-7): ";
        
        if (!(cin >> pilihanMenu)) {
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }

        switch (pilihanMenu) {
            case 1: handleGameBaru(); break;
            case 2: handleMuatGame(); break;
            case 3: handleLeaderboard(); break;
            case 4: handleCariPemain(); break;
            case 5: handleHapusSimpanan(); break;
            case 6: handleCaraBermain(); break;
            case 7: 
                bersihkanLayar();
                cout << "Terima kasih telah bermain Tetris!\n";
                break;
            default:
                cout << "Pilihan tidak valid! Tekan Enter...";
                cin.ignore();
                cin.get();
        }

    } while (pilihanMenu != 7);

    return 0;
}
